# User Guideline Mini Car

🕹️ User Guideline
Sistem Kendali Mobil Car Racer dengan PS4 Controller
🎯 Tujuan
Panduan ini ditujukan untuk membantu pengguna dalam mengoperasikan mobil Car Racer berbasis ESP32 menggunakan PS4 Controller secara aman, efisien, dan mudah dipahami.

🚗 Langkah-Langkah Penggunaan
1. 💻 Persiapan Awal
   
a. Instalasi SixaxisPair Tool
Unduh dan instal SixaxisPair Tool di laptop/komputer Anda.

b. Upload Kode ke ESP32
Buka Arduino IDE dan upload kode kendali ke board ESP32. Setelah kode berhasil dijalankan, salin MAC Address yang muncul di output serial monitor.

c. Hubungkan PS4 Controller ke Laptop via Kabel USB
Sambungkan controller PS4 ke laptop menggunakan kabel USB.
Buka aplikasi SixaxisPair Tool dan tempelkan MAC Address ESP32 yang sudah disalin sebelumnya.
Klik Update.

d. Lepaskan Kabel & Pairing Wireless
Cabut kabel dari controller.
Nyalakan controller dan tunggu beberapa saat hingga controller terhubung secara nirkabel ke ESP32.


2. 🔋 Nyalakan Mobil
Pastikan baterai sudah terpasang dengan daya yang cukup.
Geser saklar daya ke posisi ON untuk mengaktifkan mobil.

3. 🎮 Hubungkan PS4 Controller ke Mobil
Tekan tombol PS pada controller hingga lampu indikator berkedip.
Tunggu beberapa detik hingga controller terhubung ke mobil.
Jika berhasil, mobil siap menerima perintah.

4. 🕹️ Kendali Gerak Mobil
Gunakan analog kiri pada PS4 Controller untuk mengendalikan mobil
✅ Tips: Gerakan mobil akan lebih responsif jika analog ditekan penuh ke arah tertentu.

5. ❌ Mematikan Koneksi
Untuk mematikan sistem, geser saklar mobil ke posisi OFF.
PS4 Controller akan otomatis terputus dan mati.


