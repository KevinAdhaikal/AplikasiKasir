# AplikasiKasir (BETA)
AplikasiKasir agar memudahkan pemilik usaha untuk mengelola Kasir

# Fitur
- Dasar untuk Kasir (Daftar Barang, Tambah Barang, Barang Masuk, Barang Terjual, Pembukuan)
- Telegram (Untuk mengirim ke Informasi dari AplikasiKasir ke Telegram anda)
- Perhitungan Keuntungan (Harga Jual - Harga Modal)

# Build
## Windows
Untuk membuild source ini, anda harus mengikuti step di bawah ini
1. lakukan download <a href="https://sourceforge.net/projects/codeblocks/files/Binaries/20.03/Windows/codeblocks-20.03mingw-setup.exe">CodeBlocks (MinGW Version)</a> dan <a href="https://cmake.org/download/">CMake GUI</a>
2. Jika selesai instalasi CodeBlocks dan CMake GUI, download source AplikasiKasir dan Extract
3. Setelah di Extract, buka cmake-gui
4. Setalah buka cmake-gui, tekan tombol "Browse Source..." dan cari folder AplikasiKasir, dan tekan tombol "Select Folder"
5. setelah itu, tekan "Browse Build...", dan buatlah folder bernama "build", dan tekan folder build, dan tekan tombol "Select Folder"
6. Setelah itu, tekan "Configure", dan pilih "CodeBlocks - MinGW Makefiles" dan tekan Finish, lalu tekan "Generate"
7. Sekarang, kamu ke folder AplikasiKasir, dan ke folder build, disitu akan ada file bernama "AplikasiKasir.cbp". tekan file tersebut. (Jika ada tulisan "Select an App to open this .cbp file", pilih CodeBlocks)
8. Terakhir, tinggal menekan tombol "Build", setelah build, anda akan melihat file, bernama "AplikasiKasir.exe"

Jika anda malas untuk membuild source, anda bisa download lewat <a href="https://github.com/KevinAdhaikal/AplikasiKasir/releases">Release</a>

## Linux
Untuk membuild source ini di Linux, anda harus mengikuti step di bawah ini
1. lakukan update repository pada APT `apt update`
2. setelah itu, lakukan install Package `apt install git build-essential cmake -y`
3. lakukan git clone dari repository saya `git clone https://github.com/KevinAdhaikal/AplikasiKasir`
4. jika anda sudah melakukan git clone, masuk ke folder "AplikasiKasir" `cd AplikasiKasir`
5. Setelah masuk ke folder AplikasiKasir, buatlah folder "build" dan langsung masuk ke folder Build `mkdir build; cd build`
6. Setelah masuk ke folder build, lakukan `cmake ..`
7. Terakhir, jika CMake megenerate "Makefile", lakukan `make` (jika ingin cepat, anda bisa melakukan `make -j 4`) dan anda akan melihat file, bernama "AplikasiKasir". dan untuk menjalankan aplikasinya, ketik `sudo ./AplikasiKasir`

## Mac OS X
Untuk membuild source ini di Mac OS X, anda harus mengikuti step di bawah ini
1. buka Terminal dan ketik `xcode-select --install` (jika sudah di install, tidak perlu menginstall lagi)
2. Setelah itu, lakukan clone dengan cara `git clone https://github.com/kevinadhaikal/aplikasikasir`
3. Setelah clone, ke folder AplikasiKasir, dan ketik `make`
4. Setelah melakukan `make`, dia akan mengeluarkan file bernama `AplikasiKasir`. untuk menjalankannya, ketik `sudo ./AplikasiKasir`

## Android (versi 7.0 keatas)
Anda bisa mengetest AplikasiKasir di Android kalian, dan ini harus menggunakan Android Versi 7.0 keatas.
1. Download Termux di <a href="https://f-droid.org/id/packages/com.termux/">F-Droid</a> dan Install termux 
2. setelah itu, lakukan update repostiory pada Termux `pkg update`
3. setelah itu, lakukan install Package `pkg install git build-esential cmake nano -y`
4. lakukan git clone dari repository saya `git clone https://github.com/KevinAdhaikal/AplikasiKasir`
5. jika anda sudah melakukan git clone, masuk ke folder "AplikasiKasir" `cd AplikasiKasir`
6. jika hape anda tidak ter root, anda harus mengganti port di bagian main.c `nano src/main.c`
7. tekan CTRL + W di bagian termux dan keyboard hape anda, dan ketik `opt[0].port = "80";`
8. lakukan ganti port, dan port nya harus di atas 1024, setelah itu lakukan save & exit dengan tekan CTRL + X dan Y dan Enter (INGAT: Tidak boleh memakai port 8081, karena port 8081 adalah port dari Telegram API)
9. buatlah folder "build" dan langsung masuk ke folder Build `mkdir build; cd build`
10. Setelah masuk ke folder build, lakukan `cmake ..`
11. Terakhir, jika CMake megenerate "Makefile", lakukan `make` (jika ingin cepat, anda bisa melakukan `make -j 4`) dan anda akan melihat file, bernama "AplikasiKasir". dan untuk menjalankan aplikasinya, ketik ./AplikasiKasir

Jadi untuk non rooted user, harus memakai port yang sesuai anda set, contoh: `opt[0].port = "8082";`<br>
Berarti, untuk input URL ke Chrome, harus seperti ini: localhost:8082

# Username & Password
untuk username dan password<br>
Username: admin<br>
Password: admin
