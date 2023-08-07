fetch("/checkLogin", {
    method: "POST"
}).then(response => {
    if (response.status == 403) location.replace("/login.html")
})

function logOut() {
    Swal.fire({
        icon: 'warning',
        title: 'Log Out',
        text: "Apakah kamu ingin Log Out dari AplikasiKasir?",
        showDenyButton: true,
        denyButtonColor: '#3085d6',
        confirmButtonColor: '#d33',
        confirmButtonText: 'Iya (Enter)',
        denyButtonText: `Tidak (Esc)`,
      }).then((result) => {
        if (result.isConfirmed) {
            document.cookie = `username=; Max-Age=0`
            document.cookie = `password=; Max-Age=0`
            location.replace("/login.html")
        }
    })
}

async function tambahBarang() {
    if (!document.getElementById("namaBarang").value) {document.getElementById("namaBarang").focus(); return Swal.mixin({toast: true,position: 'top-end',showConfirmButton: false,timer: 3000}).fire({icon: 'error',title: 'mohon isi Nama Barang dengan benar!'})}
    else if (!document.getElementById("jumlahBarang").value) {document.getElementById("jumlahBarang").focus(); return Swal.mixin({toast: true,position: 'top-end',showConfirmButton: false,timer: 3000}).fire({icon: 'error',title: 'mohon isi Jumlah Barang dengan benar!'})}
    else if (!document.getElementById("hargaModal").value) {document.getElementById("hargaModal").focus(); return Swal.mixin({toast: true,position: 'top-end',showConfirmButton: false,timer: 3000}).fire({icon: 'error',title: 'mohon isi Harga Modal dengan benar!'})}
    else if (!document.getElementById("hargaJual").value) {document.getElementById("hargaJual").focus(); return Swal.mixin({toast: true,position: 'top-end',showConfirmButton: false,timer: 3000}).fire({icon: 'error',title: 'mohon isi Harga Jual dengan benar!'})}
    else if (parseInt(document.getElementById("jumlahBarang").value) < 1) {document.getElementById("jumlahBarang").focus(); return Swal.mixin({toast: true,position: 'top-end',showConfirmButton: false,timer: 3000}).fire({icon: 'error',title: 'Jumlah Barang tidak bisa di isi dengan 0 mohon isi harga dengan benar!'})}
    else if (parseInt(document.getElementById("hargaModal").value) < 1) {document.getElementById("hargaModal").focus(); return Swal.mixin({toast: true,position: 'top-end',showConfirmButton: false,timer: 3000}).fire({icon: 'error',title: 'Harga Modal tidak bisa di isi dengan 0 mohon isi harga dengan benar!'})}
    else if (parseInt(document.getElementById("hargaJual").value) < 1) {document.getElementById("hargaJual").focus(); return Swal.mixin({toast: true,position: 'top-end',showConfirmButton: false,timer: 3000}).fire({icon: 'error',title: 'Harga Jual tidak bisa di isi dengan 0 mohon isi harga dengan benar!'})}
    await fetch("/tambahBarang", {
        method: "POST",
        headers: {
            "namaBarang": document.getElementById("namaBarang").value,
            "jumlahBarang": document.getElementById("jumlahBarang").value.replaceAll(".", ""),
            "hargaModal": document.getElementById("hargaModal").value.replaceAll(".", ""),
            "hargaJual": document.getElementById("hargaJual").value.replaceAll(".", ""),
            "barcodeBarang": document.getElementById("barcodeBarang").value ? document.getElementById("barcodeBarang").value : ""
        }
    }).then(response => {
        if (response.status == 200) {
            Swal.mixin({
                toast: true,
                position: 'top-end',
                showConfirmButton: false,
                timer: 3000,
              }).fire({
                icon: 'success',
                title: 'Barang berhasil ditambahkan ke database!'
            })
            if (document.getElementById("resetNamaBarang").checked) document.getElementById("namaBarang").value = ""
            if (document.getElementById("resetJumlahBarang").checked) document.getElementById("jumlahBarang").value = ""
            if (document.getElementById("resetHargaModal").checked) document.getElementById("hargaModal").value = ""
            if (document.getElementById("resetHargaJual").checked && document.getElementById("resetHargaModal").checked) {
                document.getElementById("hargaJual").value = ""
                document.getElementById("persenJual").value = ""
            }
            else if (document.getElementById("resetHargaJual").checked && !document.getElementById("resetHargaModal").checked) {
                document.getElementById("hargaJual").value = document.getElementById("hargaModal").value
                document.getElementById("persenJual").value = "0.00"
            }
            else if (!document.getElementById("resetHargaJual").checked && document.getElementById("resetHargaModal").checked) {
                document.getElementById("hargaJual").value = ""
                document.getElementById("persenJual").value = ""
            }
            if (document.getElementById("resetBarcodeBarang").checked) document.getElementById("barcodeBarang").value = ""

            document.getElementById("namaBarang").focus()
        }
        else {
            Swal.mixin({
                toast: true,
                position: 'top-end',
                showConfirmButton: false,
                timer: 3000,
              }).fire({
                icon: 'error',
                title: response.statusText
            })
        }
    })
}