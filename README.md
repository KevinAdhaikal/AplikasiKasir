# AplikasiKasir (BETA)
AplikasiKasir bertujuan untuk memudahkan pemilik usaha dalam mengelola kasir.

## Fitur
- Dasar untuk Kasir (Daftar Barang, Tambah Barang, Barang Masuk, Barang Terjual, Pembukuan)
- Integrasi dengan Telegram (Mengirim informasi dari AplikasiKasir ke Telegram Anda)
- Perhitungan Keuntungan (Harga Jual - Harga Modal)

## Build
### Windows
1. Unduh dan install [CodeBlocks (MinGW Version)](https://sourceforge.net/projects/codeblocks/files/Binaries/20.03/Windows/codeblocks-20.03mingw-setup.exe) dan [CMake GUI](https://cmake.org/download/).
2. Unduh dan ekstrak sumber AplikasiKasir.
3. Buka CMake GUI, lalu atur direktori sumber dan direktori build.
4. Konfigurasi dengan "CodeBlocks - MinGW Makefiles" dan tekan Generate.
5. Buka file `AplikasiKasir.cbp` dalam folder build menggunakan CodeBlocks, kemudian tekan Build.

Jika Anda tidak ingin membuild, Anda dapat mengunduhnya Executable dari [Releases](https://github.com/KevinAdhaikal/AplikasiKasir/releases).
<hr>

### Linux
<details><summary><b>CMake</b></summary>

1. Update repositori APT dengan `apt update`.
2. Instal paket yang diperlukan dengan `apt install git build-essential cmake -y`.
3. Clone repositori dengan cara `git clone https://github.com/KevinAdhaikal/AplikasiKasir`.
4. Masuk ke direktori AplikasiKasir dan buat folder build: `cd AplikasiKasir && mkdir build && cd build`.
5. Lakukan `cmake ..` untuk membuat Makefile.
6. Jalankan `make` untuk membuild aplikasi (atau lakukan `make -j 4` agar build nya lebih cepat).
7. Untuk menjalankan aplikasi, ketik `sudo ./AplikasiKasir`.

</details>

<details><summary><b>Makefile (tanpa CMake)</b></summary>

1. Update repositori APT dengan `apt update`.
2. Instal paket yang diperlukan dengan `apt install git build-essential -y`.
3. Clone repositori dengan cara `git clone https://github.com/KevinAdhaikal/AplikasiKasir`.
4. Masuk ke direktori AplikasiKasir, ketik: `cd AplikasiKasir`.
5. Untuk build aplikasi Jalankan, ketik: `make` (atau lakukan `make -j 4` agar build nya lebih cepat).
6. Untuk menjalankan aplikasi, ketik `sudo ./AplikasiKasir`

</details>
<hr>

### Mac OS X
1. Buka Terminal dan jalankan `xcode-select --install` jika belum diinstal.
2. Clone repositori dengan `git clone https://github.com/kevinadhaikal/aplikasikasir`.
3. Masuk ke direktori AplikasiKasir dan jalankan `make` (atau lakukan `make -j 4` agar build nya lebih cepat).
4. Aplikasi akan dibangun dengan nama `AplikasiKasir`. Jalankan dengan cara `sudo ./AplikasiKasir`.

<hr>

### Android (versi 7.0 ke atas)
Untuk Android versi 7.0 ke atas, bisa menggunakan [Termux](https://f-droid.org/id/packages/com.termux/). dan Download Termux harus lewat di F-Droid

<details><summary><b>CMake</b></summary>

1. Buka Termux dan jalankan `pkg update`.
2. Instal paket yang diperlukan dengan `pkg install git build-essential cmake -y`.
3. Clone repositori dengan cara `git clone https://github.com/KevinAdhaikal/AplikasiKasir`.
4. Masuk ke direktori "AplikasiKasir" dan edit file `src/main.c` untuk mengganti port (jika hape anda tidak di-root, gunakan port di atas 1024).
5. buat folder "build" lalu jalankan cmake untuk membuat Makefile: `mkdir build; cd build; cmake ..`.
6. Jalankan `make` untuk membuild aplikasi (atau lakukan `make -j 4` agar build nya lebih cepat).
7. Untuk menjalankan aplikasi, ketik `./AplikasiKasir` (Untuk pengguna root. bisa menjalankan AplikasiKasir, dengan melakukan `sudo ./AplikasiKasir`).

</details>

<details><summary><b>Makefile (tanpa CMake)</b></summary>

1. Buka Termux dan jalankan `pkg update`.
2. Instal paket yang diperlukan dengan `pkg install git build-essential -y`.
3. Clone repositori dengan cara `git clone https://github.com/KevinAdhaikal/AplikasiKasir`.
4. Masuk ke direktori "AplikasiKasir" dan edit file `src/main.c` untuk mengganti port (jika hape anda tidak di-root, gunakan port di atas 1024).
5. buat folder "build" lalu jalankan cmake untuk membuat Makefile: `mkdir build; cd build; cmake ..`.
6. Untuk build aplikasi, ketik: `make` (atau lakukan `make -j 4` agar build nya lebih cepat).
7. Untuk menjalankan aplikasi, ketik `./AplikasiKasir` (Untuk pengguna root. bisa menjalankan AplikasiKasir, dengan melakukan `sudo ./AplikasiKasir`).

</details>
<hr>

PERINGATAN: Jangan memakai port 8081, karena itu untuk HTTP Telegram API<br>
Untuk pengguna non-root, pastikan port yang Anda gunakan tidak bertabrakan dengan port lainnya. Misalnya, `opt[0].port = "8082"`.

## Username & Password
- Username: admin
- Password: admin
