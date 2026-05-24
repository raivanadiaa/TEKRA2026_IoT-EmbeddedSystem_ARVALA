// ============================================================
// DATA
// ============================================================

const db = {
  kelas1:{
    ruangan_aktif:1,
    suhu:26.2,
    kelembapan:62,
    ldr:1800,
    lampu:1,
    servo_sudut:90,
    co2_raw:920,
    status_co2:'NORMAL',
    kwh_hari_ini:0.183,
    kwh_bulan_ini:45.5,
    status_listrik:'HEMAT'
  },

  kelas2:{
    ruangan_aktif:0,
    suhu:25.2,
    kelembapan:59,
    ldr:2200,
    lampu:0,
    servo_sudut:0,
    co2_raw:830,
    status_co2:'NORMAL',
    kwh_hari_ini:0.146,
    kwh_bulan_ini:20.1,
    status_listrik:'HEMAT'
  }
};

let aktif='kelas1';

const MAX_PTS=20;

const history={
  kelas1:{labels:[],suhu:[],kelembapan:[],co2:[],kwh:[]},
  kelas2:{labels:[],suhu:[],kelembapan:[],co2:[],kwh:[]}
};