document.getElementById("namaBarcodeBarang").focus()

document.getElementById("namaBarcodeBarang").addEventListener("keypress", function(e) {
    if (e.keyCode == 13) document.getElementById("checkBarangButton").click()
})

$('#jumlahBarang').keypress(function(e){
    if (e.keyCode == 13) {
        e.preventDefault()
        document.getElementById("hargaBarang").focus()
    }
    else if ((e.keyCode >= 48 && e.keyCode <= 58)) {
        e.target.value = Intl.NumberFormat('id', {}).format((e.target.value + e.key).replaceAll(".", ""));
        e.preventDefault();
    } else return false
})

$('#jumlahBarang').keydown(function(e){
    if (e.keyCode == 8) {
        e.target.value = Intl.NumberFormat('id', {}).format(e.target.value.slice(0, -1).replaceAll(".", ""));
        e.preventDefault();
    }
})

$('#hargaBarang').keypress(function(e){
    if (e.keyCode == 13) {
        e.preventDefault()
        document.getElementById("TambahBarangButton").click()
    }
    else if ((e.keyCode >= 48 && e.keyCode <= 58)) {
        e.target.value = Intl.NumberFormat('id', {}).format((e.target.value + e.key).replaceAll(".", ""));
        e.preventDefault();
    } else return false
})

$('#hargaBarang').keydown(function(e){
    if (e.keyCode == 8) {
        e.target.value = Intl.NumberFormat('id', {}).format(e.target.value.slice(0, -1).replaceAll(".", ""));
        e.preventDefault();
    }
})

$(document).keydown(function(event) { 
    if (event.keyCode == 27) { 
      $('#modal-findBarang').modal("hide");
    }
});

async function tambahBarangMasuk(index) {
    document.getElementById("namaBarcodeBarang").value = $("#cariBarang").DataTable().rows(index).data()[0][0]
    document.getElementById("namaBarcodeBarang").disabled = true
    document.getElementById("checkBarangButton").disabled = true

    document.getElementById("jumlahBarang").disabled = false
    document.getElementById("hargaBarang").disabled = false
    document.getElementById("TambahBarangButton").disabled = false

    $('#modal-findBarang').modal("hide");
    document.getElementById("jumlahBarang").focus()
}

function checkBarang(namaBarang) {
    fetch("barangMasukLogic?barangMasukArgs=2", {
        method: "POST",
        headers: {
            findBarang: namaBarang
        }
    }).then(async response => {
        if (response.status == 200) {
            await response.text().then(data => {
                data = data.split("\n");
                if (data.length > 2) {
                    $("#cariBarang").DataTable().clear().draw()
                    for (let a = 0; a < data.length - 1; a++) {
                        $("#cariBarang").DataTable().row.add([data[a], `<center>
                        <button type="button" class="btn btn-primary" onclick="tambahBarangMasuk(${a})">Tambah Barang</button>
                        </center>`]).draw()
                    }
                    $("#modal-findBarang").modal("show")
                    document.getElementById("namaBarcodeBarang").blur()
                }
                else {
                    document.getElementById("namaBarcodeBarang").value = data[0]
                    document.getElementById("namaBarcodeBarang").disabled = true
                    document.getElementById("checkBarangButton").disabled = true

                    document.getElementById("jumlahBarang").disabled = false
                    document.getElementById("hargaBarang").disabled = false
                    document.getElementById("TambahBarangButton").disabled = false
                    
                    document.getElementById("jumlahBarang").focus()
                }
            })
        } else {
            Swal.mixin({
                toast: true,
                position: 'top-end',
                showConfirmButton: false,
                timer: 3000,
              }).fire({
                icon: 'error',
                title: response.statusText
            })

            document.getElementById("namaBarcodeBarang").value = ""
            
        }
    })
}

function resetSemuaInput() {
    document.getElementById("namaBarcodeBarang").value = ""
    document.getElementById("namaBarcodeBarang").disabled = false;

    document.getElementById("checkBarangButton").disabled = false;

    document.getElementById("jumlahBarang").value = ""
    document.getElementById("jumlahBarang").disabled = true
    document.getElementById("hargaBarang").value = ""
    document.getElementById("hargaBarang").disabled = true

    document.getElementById("TambahBarangButton").disabled = true
    document.getElementById("namaBarcodeBarang").focus()
}

async function barangMasuk() {
    await fetch("barangMasukLogic?barangMasukArgs=3", {
        method: "POST",
        headers: {
            namaBarang: document.getElementById("namaBarcodeBarang").value,
            jumlahBarang: document.getElementById("jumlahBarang").value.replaceAll(".", ""),
            hargaBarang: document.getElementById("hargaBarang").value.replaceAll(".", "")
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
                title: "Barang Tersebut berhasil di tambahkan ke dalam Barang Masuk!"
            })
            resetSemuaInput()
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