# Sistem Smart Classroom berbasis IoT (Internet of Things) menggunakan MQTT, Real-Time Monitoring, dan Multi-Sensor dalam Mendukung Green Campus Universitas Brawijaya

## 1. Latar Belakang 
Universitas Brawijaya adalah satu di antara banyak perguruan tinggi yang populer di kalangan masyarakat. Hal ini terbukti dari jumlah mahasiswa baru pada tahun 2025 yang mencapai sekitar 17 ribu. Dengan jumlah mahasiswa yang membludak, Universitas Brawijaya tentu terus mengembangkan fasilitas yang dimiliki untuk menunjang hal tersebut. Salah satu fasilitasnya adalah gedung beserta ruang kelas. Setiap gedung memiliki banyak ruang kelas. Tentunya setiap ruang kelas menggunakan perangkat listrik, seperti lampu, AC, dan lain sebagainya setiap hari. Seiring dengan banyaknya perangkat listrik yang digunakan, konsumsi daya dan tagihan pembayaran listrik pun semakin meningkat. Hal ini membuat Universitas Brawijaya memerlukan sistem monitoring terpusat untuk melakukan pemantauan kondisi ruangan secara real-time.

Pada kondisi saat ini, pengawasan penggunaan listrik masih dilakukan oleh petugas kebersihan dan keamanan. Dengan jumlah kelas yang banyak, metode manual tersebut tentu membutuhkan waktu yang lama sehingga tidak efisien. Selain itu, metode ini juga dinilai tidak konsisten karena hal ini bergantung pada ketelitian petugas. Metode ini juga menyulitkan petugas untuk menjangkau seluruh ruangan secara menyeluruh. Di samping itu, tidak adanya sistem monitoring berkelanjutan yang menyebabkan kondisi penggunaan listrik tidak dapat dipantau setiap saat. Hal ini berpotensi menimbulkan pemborosan energi listrik serta meningkatkan biaya operasional yang harus ditanggung oleh pihak universitas.

## 2. Arsitektur Sistem 
Arsitektur sistem terdiri dari tiga lapisan utama, yaitu lapisan perangkat (device layer), lapisan jaringan (network layer), dan lapisan aplikasi (application layer).
1. Lapisan Perangkat (device layer)
Pada lapisan perangkat, seluruh sensor (PIR HC-SR501, LDR, DHT22, MQ-135) terhubung langsung ke pin input ESP32 yang terpasang di dalam ruang kelas. PIR dipasang di  pojok ruangan untuk cakupan deteksi kehadiran yang luas, LDR dipasang di dalam ruangan untuk memverifikasi status lampu, sedangkan DHT22 dan MQ-135 dipasang di posisi representatif untuk membaca kondisi udara ruangan. Output ESP32 terhubung ke modul relay untuk mengontrol lampu dan AC, serta ke pin PWM yang dihubungkan ke motor driver untuk mengatur kecepatan motor DC sebagai replika blower AC.
2. Lapisan Jaringan (network layer)
Pada lapisan jaringan, ESP32 terhubung ke jaringan Wi-Fi kampus yang sudah tersedia. Data sensor dikirimkan secara berkala menggunakan protokol MQTT menuju broker MQTT yang berjalan di server ThingsBoard. MQTT dipilih karena protokol ini sangat ringan (low-bandwidth) dan dirancang khusus untuk perangkat IoT, menjadikannya pilihan ideal untuk sistem yang akan beroperasi di ratusan ruangan secara bersamaan.
3. Lapisan Aplikasi (application layer)
Pada lapisan aplikasi, dashboard monitoring menerima dan menampilkan seluruh data sensor secara real-time melalui koneksi MQTT sehingga kondisi ruangan dapat dipantau langsung oleh admin DPTSI melalui browser. Dashboard menampilkan informasi status tiap ruang kelas, kondisi suhu dan kualitas udara, status pencahayaan otomatis, konsumsi energi simulasi, serta grafik historis sebagai bahan evaluasi penggunaan energi dan kenyamanan ruang belajar.

## 3. Alur Logika & Integrasi Perangkat (Data Flow)
1. Lapisan Sensor (Data Acquisition)
     Input Fisik: Sensor membaca kondisi ruang kelas secara berkala:
     - PIR HC-SR501: Keberadaan/aktivitas manusia.
     - LDR: Intensitas cahaya ruangan.
     - DHT22: Suhu dan kelembaban udara.
     - MQ-135: Kualitas/polusi udara.
   Tujuan Data: Seluruh data mentah dikirim ke ESP32 sebagai pusat pemrosesan.
2. Pemrosesan & Logika Alat (Edge Processing)
   Di dalam ESP32, data sensor dianalisis untuk menentukan dua tindakan otomatis:
   Kendali Otomatis: Jika PIR mendeteksi orang DAN LDR mendeteksi ruangan gelap maka Relay menyalakan lampu.
                     Jika DHT22 mendeteksi suhu panas DAN PIR mendeteksi orang maka Relay mengaktifkan pendingin + Servo Motor bergerak (simulasi blower AC) menggunakan sinyal PWM (sudut membesar seiring naiknya suhu).
   Kalkulasi Energi & Emisi: Berdasarkan durasi aktifnya Relay dan Servo, ESP32 menghitung:
     -  Estimasi konsumsi daya (kWh).
     -  Estimasi biaya listrik harian.
     -  Estimasi pengurangan emisi karbon.
 3. Transmisi Data (Network Gateway)
    - Langkah 1: ESP32 menyatukan seluruh data (Sensor, Status Aktuator, Hasil Kalkulasi Energi) ke dalam format JSON.
    - Langkah 2: Paket JSON dikirim secara real-time via Wi-Fi menggunakan protokol MQTT menuju broker EMQX.
    - Langkah 3: Broker EMQX meneruskan data tersebut ke Web Dashboard melalui protokol WebSocket.
4. Lapisan Visualisasi & Interaksi (Application Layer)
   Monitoring Otomatis: Dasbor berbasis HTML, CSS, dan JavaScript menerima data via WebSocket dan memperbarui tampilan (grafik suhu, status lampu, kualitas udara, biaya operasional, jejak karbon) secara otomatis tanpa refresh halaman.
   Kendali Manual (Override Downlink): Admin DPTSI menekan tombol "Matikan Paksa" di dashboard.
      - Perintah dikirim balik via MQTT $\rightarrow$ EMQX $\rightarrow$ ESP32.
      - ESP32 mengeksekusi instruksi dengan mematikan Relay dan menghentikan Servo Motor.
  
##  4. Eksplorasi & Simulasi Data 
Proyek ini dilengkapi dengan berkas simulasi untuk memvalidasi konsep sistem sebelum diimplementasikan pada perangkat keras menggunakan website simulasi Wokwi
Proyek pada Wokwi dapat diakses melalui link berikut:
- Kelas 1: https://wokwi.com/projects/464805154374722561
- Kelas 2: https://wokwi.com/projects/464805361023909889

## 5. Cara Menjalankan Proyek Simulasi dan Dashboard
1. Buka https://www.emqx.com/en/mqtt/public-mqtt5-broker di browser untuk memastikan public broker sedang aktif
2. Pastikan konfigurasi berikut sudah terisi di dalam kode dan tidak diubah — broker: broker.emqx.io, port ESP32: 1883, port dashboard: 8084 (WSS/WebSocket), topik: smartclassroom/kelas1 dan smartclassroom/kelas2, Client ID sudah di-generate otomatis secara random sehingga tidak perlu diisi manual
3. Buka wokwi.com di browser lalu klik New Project dan pilih ESP32
4. Buka tab Libraries di Wokwi lalu cari dan tambahkan library berikut satu per satu: DHT sensor library, PubSubClient, ArduinoJson, ESP32Servo
5. Paste kode .ino terbaru ke editor Wokwi
6. Pastikan di bagian atas kode WiFi sudah terisi Wokwi-GUEST dan password kosong karena Wokwi menyediakan jaringan WiFi simulasi otomatis dengan nama tersebut
7. Tambahkan komponen di Wokwi yaitu DHT22, PIR, LDR, Potensio sebagai pengganti MQ135, Servo, Relay, dan LED sebanyak 2 buah
8. Sesuaikan koneksi pin di diagram.json dengan pin yang ada di kode
9. Klik Run di Wokwi lalu buka Serial Monitor dan pastikan muncul tulisan [WiFi] Terhubung! dilanjutkan [MQTT] Terhubung! dan [MQTT] Terkirim
10. Buat file baru di komputer dan beri nama dashboard.html
11. Buka Notepad atau VS Code lalu paste seluruh kode dashboard ke dalamnya — kode HTML, CSS, dan JS sudah digabung dalam 1 file yang sama, CSS ada di bagian style dan JS ada di bagian script sehingga tidak perlu membuat file terpisah
12. Simpan file dan pastikan ekstensinya .html bukan .txt
13. Buka file dashboard.html langsung di browser Chrome atau Edge — tidak perlu server atau software tambahan
14. Pastikan status di pojok kanan atas dashboard berubah menjadi Online EMQX
15. Putar potensio di Wokwi dan pastikan nilai CO2 di dashboard ikut berubah
16. Aktifkan PIR di Wokwi dan pastikan status okupansi di dashboard ikut berubah
17. Naikkan nilai suhu di DHT22 Wokwi dan pastikan badge status AC di dashboard ikut berubah
