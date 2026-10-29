## Daftar Isi

- [Gambaran Umum](#gambaran-umum)
- [Perangkat yang Digunakan](#perangkat-yang-digunakan)
- [Eksperimen A: ON/OFF dengan kode asli](#eksperimen-a-onoff-dengan-kode-asli)
- [Eksperimen B: kode modifikasi](#eksperimen-b-kode-modifikasi)
- [Kesimpulan](#kesimpulan)

## Gambaran Umum
Pada Modul 1, perintah ke motor diketik satu per satu di Serial Monitor. Modul 2 memindahkan tugas itu ke program Python, sehingga perintah dapat dikirim berurutan dan diulang tanpa diketik. Notebook Jupyter dengan pustaka PySerial mencari port ESP32, membuka koneksi, mengirim karakter, lalu membaca balasan. Sisi ESP32 tidak berubah dan memakai sketch dari Modul 1.

Tujuan praktikum:
1. Mengendalikan motor pada kit iMCLab dari Python menggunakan PySerial.
2. Memahami cara notebook memilih port ESP32 secara otomatis dari VID:PID dan deskripsi port.
3. Memahami apa yang terjadi saat port dibuka dan cara membaca balasan ESP32 dengan benar.
4. Memodifikasi notebook agar mendukung tiga level kecepatan.

## Perangkat yang Digunakan
<p align="center">
  <img src="image/kit_label.jpg" width="420" alt="Kit iMCLab">
  <br>< <b>Gambar 1.</b>
</p>

| No | Bagian | Peran |
|---|---|---|
| 1 | ESP32 DevKit (modul ESP32-WROOM-32) | Pengendali, menerima karakter dari Python dan menghasilkan sinyal arah serta PWM |
| 2 | IC driver motor 16 pin | Menyalurkan daya dari adaptor ke motor sesuai sinyal ESP32 |
| 3 | Motor DC dengan rumah putih cetak 3D | Aktuator yang diamati |
| 4 | Piringan putih pada poros | Penanda putaran yang melewati celah sensor |
| 5 | Modul sensor celah (*slot*) | Mendeteksi putaran piringan |
| 6 | LED hijau dan resistor | Indikator pada papan |
| 7 | Tombol EN | Mereset ESP32 |
| 8 | Tombol BOOT | Ditahan saat unggah program bila proses macet di `Connecting...` |
| 9 | Konektor micro-USB | Jalur daya dan data serial antara laptop dan ESP32 |
| 10 | Jack DC | Catu daya adaptor |

Pada ESP32 sendiri ada dua LED kecil: merah sebagai penanda daya, dan biru yang terhubung ke pin 2. LED biru menyala bersamaan dengan motor, sehingga dipakai untuk memastikan perintah sudah diterima.

| Perangkat lunak | Keterangan |
|---|---|
| Python | versi 3.13.0 (kernel notebook) |
| Pustaka | PySerial (`serial`, `serial.tools.list_ports`) |
| Notebook | Jupyter, `Tes_IMCLab_pilih_port_otomatis.ipynb` (dosen) dan `Tes_IMCLab_v2_modifikasi.ipynb` (modifikasi) |
| Arduino IDE | versi 2.3.1, hanya untuk mengunggah sketch |
| Sketch di ESP32 | `on_off_Motor.ino` (Eksperimen A) dan `on_off_Motor_modifikasi.ino` (Eksperimen B), keduanya dari Modul 1 |
| Port | COM6 (USB-Enhanced-SERIAL CH9102, VID:PID 1A86:55D4) |
| Baud rate | 115200 |

### Kode asli dan kode modifikasi

| Aspek | Kode dosen | Kode modifikasi |
|---|---|---|
| Perintah | 1, 0 | 1, 0, 2, 3, 4 |
| Pemilihan port | deteksi otomatis | `PORT_MANUAL` dipakai langsung bila diisi, deteksi otomatis bila kosong |
| Daftar perintah sah | `('0', '1')` ditulis di dalam fungsi | konstanta `PERINTAH_VALID` |
| Jeda sebelum membaca balasan | 0,1 detik | 0,2 detik |
| Membaca balasan | `if` dan satu kali `readline()` | `while` sampai buffer kosong |
| Fungsi perintah | `motor_on`, `motor_off` | ditambah `motor_pelan`, `motor_sedang`, `motor_cepat` |
| Urutan demo | ON 3 detik, OFF 1 detik, ON 2 detik, OFF | pelan 3 detik, sedang 3 detik, cepat 3 detik, OFF |
| Pesan bila gagal | periksa PORT dan kabel USB | ditambah pengingat menutup Serial Monitor Arduino IDE |
| Pesan bila port tidak terdeteksi | tentukan PORT secara manual | isi `PORT_MANUAL` di bagian konfigurasi |

Kedua notebook memakai alur yang sama: daftar port, pilih port, buka koneksi, urutan demo, mode interaktif, lalu `finally` yang mematikan motor dan menutup port.

## Eksperimen A: ON/OFF dengan kode asli

| Perintah | Balasan ESP32 di Python | LED biru | Duty cycle |
|---|---|---|---|
| 1 | `LED is on !!!` | menyala | 205 (80,4%) |
| 0 | `LED is off !!!` | padam | 0 (0%) |

### Analisis

- **Python hanya menggantikan Serial Monitor.** `ser.write()` mengirim karakter yang sama dengan yang diketik di Serial Monitor, dan ESP32 tidak bisa membedakan pengirimnya. Karena itu sketch Modul 1 dipakai tanpa perubahan.
- **Port dibuka lalu ditunggu 2 detik.** `buka_koneksi` memanggil `time.sleep(2)` setelah `serial.Serial(...)`. Alasannya terlihat pada Eksperimen B: membuka port membuat ESP32 restart.
- **Balasan dibaca satu baris.** `kirim_perintah` memakai `if ser.in_waiting > 0` lalu `readline()` sekali. Bila ESP32 mengirim lebih dari satu baris, baris sisanya tertinggal di buffer.
- **Motor selalu dimatikan saat keluar.** Blok `finally` mengirim `motor_off` lalu menutup port, termasuk bila notebook dihentikan dengan `Ctrl+C`.
- **Satu port hanya untuk satu program.** Serial Monitor Arduino IDE harus ditutup sebelum notebook dijalankan, kalau tidak koneksi gagal dibuka.

## Eksperimen B: kode modifikasi

### Apa yang diubah dan alasannya

1. **Tiga fungsi kecepatan dan `PERINTAH_VALID`.** `motor_pelan`, `motor_sedang`, `motor_cepat` mengirim `2`, `3`, `4`, sesuai sketch modifikasi Modul 1. Daftar perintah sah dipindah ke satu konstanta supaya mudah ditambah.
2. **Balasan dibaca dengan `while`.** Perulangan ini membaca semua baris yang menunggu di buffer, bukan hanya baris pertama.
3. **Jeda 0,1 detik menjadi 0,2 detik.** Memberi ESP32 waktu lebih longgar untuk menjalankan perintah dan membalas sebelum dibaca.
4. **`PORT_MANUAL`.** Bila diisi, port langsung dipakai tanpa deteksi otomatis. Berguna saat port sudah diketahui atau ada beberapa port serial.
5. **Urutan demo.** ON/OFF diganti pelan, sedang, cepat masing-masing 3 detik, lalu OFF.

Kenaikan perintah dari `2` ke `3` ke `4` setara duty 100, 150, 255 atau 39,2%, 58,8%, dan 100%.

### Hasil

| Perintah | Fungsi Python | Duty | Balasan ESP32 | Respons motor |
|---|---|---|---|---|
| `2` | `motor_pelan` | 100 | `Motor pelan (duty 100)` | Berputar pelan |
| `3` | `motor_sedang` | 150 | `Motor sedang (duty 150)` | Berputar cukup pelan |
| `4` | `motor_cepat` | 255 | `Motor cepat (duty 255)` | Berputar kencang|
| `1` | `motor_on` | 205 | `LED is on !!!` | Berputar cukup kencang |
| `0` | `motor_off` | 0 | `LED is off !!!` | Motor berhenti |

### Analisis

- **Membuka port me-reset ESP32.** 
- **Notebook dan sketch harus satu pasangan.** Balasan `Motor pelan (duty 100)` hanya muncul bila ESP32 berisi sketch modifikasi. Pada sketch asli, karakter `2`, `3`, `4` diabaikan tanpa balasan.
- **Perintah `0` dan `1` tidak berubah.** Balasannya sama dengan Eksperimen A, yaitu `LED is off !!!` dan `LED is on !!!`.
- Sketch modifikasi memberi kick-start 60 ms, tetapi balasan tetap terbaca dalam satu putaran jeda 0,2 detik.

## Kesimpulan

1. Motor pada iMCLab dapat dikendalikan dari Python dengan PySerial: notebook mengirim karakter `1` atau `0` dan membaca balasan ESP32, menggantikan peran Serial Monitor.
2. Membuka port me-reset ESP32, sehingga notebook perlu menunggu 2 detik dan membaca semua baris balasan, bukan hanya satu.
3. Modifikasi menambah tiga level kecepatan (`2`, `3`, `4`), `PORT_MANUAL`, dan pembacaan balasan banyak baris. Notebook modifikasi hanya bekerja penuh bila ESP32 berisi sketch modifikasi Modul 1.
