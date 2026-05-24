#include <DHT.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>

const char* WIFI_SSID     = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";
const char* MQTT_SERVER   = "broker.emqx.io";
const int   MQTT_PORT     = 1883;
const char* MQTT_TOPIC    = "smartclassroom/kelas1";

#define PIN_PIR    4
#define PIN_LDR    39
#define PIN_DHT    22
#define PIN_MQ135  34
#define PIN_LED    26
#define PIN_LED2   25
#define PIN_SERVO  13
#define PIN_RELAY  27
#define DHT_TYPE   DHT22

#define SUHU_SEDANG   26.0
#define SUHU_PANAS    28.0
#define LDR_REDUP     1000
#define LDR_GELAP     2000
#define CO2_WARNING   2500
#define CO2_DANGER    3500
#define DELAY_MATIKAN 10000
#define INTERVAL_MQTT 5000

DHT dht(PIN_DHT, DHT_TYPE);
WiFiClient espClient;
PubSubClient mqtt(espClient);
Servo servoAC;

bool lampuNyala = false, relayAktif = false, sedangHitungMundur = false;
int sudutServo = 0;
unsigned long waktuKosong = 0, waktuMQTT = 0, waktuEnergiTerakhir = 0;
float kwhHariIni = 0.0, kwhBulanIni = 45.5, targetHarian = 1.5;

void connectWiFi() {
  Serial.print("[WiFi] Menghubungkan");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\n[WiFi] Terhubung!");
}

void connectMQTT() {
  while (!mqtt.connected()) {
    Serial.print("[MQTT] Menghubungkan...");
    String clientId = "ESP32-Kelas1-" + String(random(0xffff), HEX);
    if (mqtt.connect(clientId.c_str())) {
      Serial.println(" Terhubung!");
    } else {
      Serial.print(" Gagal rc="); Serial.println(mqtt.state());
      delay(3000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_PIR, INPUT);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_LED2, OUTPUT);
  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_LED, LOW);
  digitalWrite(PIN_LED2, LOW);
  digitalWrite(PIN_RELAY, LOW);
  servoAC.attach(PIN_SERVO);
  servoAC.write(0);
  dht.begin();
  connectWiFi();
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  connectMQTT();
  waktuEnergiTerakhir = millis();
  Serial.println("=== Kelas 1 Siap ===");
}

void hitungEnergi() {
  unsigned long sekarang = millis();
  float deltaJam = (sekarang - waktuEnergiTerakhir) / 3600000.0;
  waktuEnergiTerakhir = sekarang;
  float daya = 0;
  if (lampuNyala) daya += 20.0;
  if (digitalRead(PIN_LED2) == HIGH) daya += 20.0;
  if (relayAktif) daya += 100.0;
  if (sudutServo == 180) daya += 800.0;
  else if (sudutServo == 90) daya += 400.0;
  kwhHariIni  += daya * deltaJam;
  kwhBulanIni += daya * deltaJam;
}

void kontrolLampu(bool adaOrang, int ldr) {
  if (adaOrang) {
    if (ldr > LDR_GELAP) {
      // Gelap banget → 2 LED nyala
      digitalWrite(PIN_LED,  HIGH);
      digitalWrite(PIN_LED2, HIGH);
      lampuNyala = true;
      Serial.println("[LAMPU] Gelap — 2 LED nyala");
    } else if (ldr > LDR_REDUP) {
      // Mendung/redup → 1 LED saja
      digitalWrite(PIN_LED,  HIGH);
      digitalWrite(PIN_LED2, LOW);
      lampuNyala = true;
      Serial.println("[LAMPU] Mendung — 1 LED nyala");
    } else {
      // Terang → semua mati
      digitalWrite(PIN_LED,  LOW);
      digitalWrite(PIN_LED2, LOW);
      lampuNyala = false;
      Serial.println("[LAMPU] Terang — semua mati");
    }
  } else {
    digitalWrite(PIN_LED,  LOW);
    digitalWrite(PIN_LED2, LOW);
    lampuNyala = false;
  }
}

void kontrolServoAC(bool adaOrang, float suhu) {
  int target = 0;
  if (adaOrang) {
    if (suhu >= SUHU_PANAS)       target = 180;
    else if (suhu >= SUHU_SEDANG) target = 90;
  }
  if (target != sudutServo) { servoAC.write(target); sudutServo = target; }
}

void kontrolRelay(bool adaOrang) {
  if (adaOrang) {
    sedangHitungMundur = false;
    if (!relayAktif) { digitalWrite(PIN_RELAY, HIGH); relayAktif = true; }
  } else {
    if (!sedangHitungMundur) { sedangHitungMundur = true; waktuKosong = millis(); }
    if (millis() - waktuKosong >= DELAY_MATIKAN && relayAktif) {
      digitalWrite(PIN_RELAY, LOW); relayAktif = false; sedangHitungMundur = false;
    }
  }
}

void kirimMQTT(bool pir, float suhu, float kelembapan, int ldr, int mq) {
  if (!mqtt.connected()) return;

  String statusCO2 = "NORMAL";
  if (mq > CO2_DANGER)        statusCO2 = "BAHAYA";
  else if (mq > CO2_WARNING)  statusCO2 = "PERINGATAN";

  String statusListrik = "NORMAL";
  if (kwhHariIni > targetHarian)             statusListrik = "BOROS";
  else if (kwhHariIni < targetHarian * 0.4)  statusListrik = "HEMAT";

  // Hitung jumlah LED aktif
  int jumlahLED = 0;
  if (digitalRead(PIN_LED)  == HIGH) jumlahLED++;
  if (digitalRead(PIN_LED2) == HIGH) jumlahLED++;

  StaticJsonDocument<512> doc;
  doc["ruangan_aktif"]  = pir ? 1 : 0;
  doc["suhu"]           = suhu;
  doc["kelembapan"]     = kelembapan;
  doc["ldr"]            = ldr;
  doc["lampu"]          = jumlahLED;
  doc["servo_sudut"]    = sudutServo;
  doc["relay"]          = relayAktif ? 1 : 0;
  doc["co2_raw"]        = mq;
  doc["status_co2"]     = statusCO2;
  doc["kwh_hari_ini"]   = kwhHariIni;
  doc["kwh_bulan_ini"]  = kwhBulanIni;
  doc["status_listrik"] = statusListrik;

  char payload[512];
  serializeJson(doc, payload);
  if (mqtt.publish(MQTT_TOPIC, payload)) Serial.println("[MQTT] Terkirim - Kelas 1");
}

void loop() {
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();

  bool pir       = digitalRead(PIN_PIR) == HIGH;
  float suhu     = dht.readTemperature();
  float kelembapan = dht.readHumidity();
  int ldr        = analogRead(PIN_LDR);
  int mq         = analogRead(PIN_MQ135);

  if (isnan(suhu))       suhu = 25.0;
  if (isnan(kelembapan)) kelembapan = 60.0;

  kontrolLampu(pir, ldr);
  kontrolServoAC(pir, suhu);
  kontrolRelay(pir);
  hitungEnergi();

  if (millis() - waktuMQTT >= INTERVAL_MQTT) {
    waktuMQTT = millis();
    kirimMQTT(pir, suhu, kelembapan, ldr, mq);
  }

  delay(500);
}