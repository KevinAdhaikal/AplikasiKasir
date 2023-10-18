# AplikasiKasir
AplikasiKasir agar memudahkan pemilik usaha untuk mengelola Kasir<br>
Untuk mencoba versi HTTP Thread (EXPERIMENTAL): https://github.com/KevinAdhaikal/AplikasiKasir/tree/experimental
# Fitur
- Mudah digunakan
- Dasar untuk Kasir (Daftar Barang, Tambah Barang, Barang Masuk, Barang Terjual, Pembukuan)
- Telegram (Untuk mengirim ke Informasi dari AplikasiKasir ke Telegram anda)
- Perhitungan Keuntungan (Harga Jual - Harga Modal)
# Build
## Windows

Untuk membuild source ini, anda harus mengikuti step di bawah ini
1. lakukan download <a href="https://sourceforge.net/projects/codeblocks/files/Binaries/20.03/Windows/codeblocks-20.03mingw-setup.exe">Codeblocks (MinGW Version)</a> dan Install 2. Jika selesai instalasi Codeblocks, download source dan extract
3. Setelah di extract, tekan file bernama `AplikasiKasir.cbp`
4. Setelah itu tekan build
5. Setelah di build, pindahkan html folder ke bin/Release

Jika anda malas untuk membuild source, anda bisa download lewat <a href="https://github.com/KevinAdhaikal/AplikasiKasir/releases">Release</a>
## Linux
Untuk membuild source ini di Linux, anda harus mengikuti step di bawah ini
1. lakukan update repository pada APT `apt update`
2. setelah itu, lakukan install Package `apt install git build-essential -y`
3. lakukan git clone dari repository saya `git clone https://github.com/KevinAdhaikal/AplikasiKasir`
4. jika anda sudah melakukan git clone, masuk ke folder "AplikasiKasir" `cd AplikasiKasir`
5. dan terakhir, lakukan build `bash build.sh`
## Android
Anda bisa mengetest AplikasiKasir di Android kalian, tetapi anda harus mengikuti step di bawah ini
1. Download Termux di <a href="https://f-droid.org/id/packages/com.termux/">F-Droid</a> dan Install termux 
2. setelah itu, lakukan update repostiory pada Termux `pkg update`
3. setelah itu, lakukan install Package `pkg install git build-esential nano -y`
4. lakukan git clone dari repository saya `git clone https://github.com/KevinAdhaikal/AplikasiKasir`
5. jika anda sudah melakukan git clone, masuk ke folder "AplikasiKasir" `cd AplikasiKasir`
6. Jika hape anda ter root, anda bisa langsung saja lakukan build `bash build.sh`
7. jika hape anda tidak ter root, anda harus mengganti port di bagian main.c `nano src/main.c`
8. tekan CTRL + W di bagian termux dan keyboard hape anda, dan ketik `opt[0].port = "80";`
9. lakukan ganti port, dan port nya harus di atas 1024, setelah itu lakukan save & exit dengan tekan CTRL + X dan Y dan Enter
10. dan terakhir, lakukan build `bash build.sh`

Jadi untuk non rooted user, harus memakai port yang sesuai anda set, contoh: `opt[0].port = "8082";`<br>
Berarti, untuk input URL ke Chrome, harus seperti ini: localhost:8081

# Username & Password
untuk username dan password<br>
Username: admin<br>
Password: admin
