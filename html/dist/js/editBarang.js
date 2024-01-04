window.onload = async function() {
    await fetch("/?api_args=5", {
        method: "POST",
        headers: {
            "barangID": window.location.search.substr(1).slice(9)
        }
    }).then(async response => {
        await response.text().then(data => {
            data = data.split("|")
            document.getElementById("namaBarang").value = data[1]
            document.getElementById("jumlahBarang").value = Intl.NumberFormat('id', {}).format((data[2]));
            document.getElementById("hargaModal").value = Intl.NumberFormat('id', {}).format((data[3]));
            document.getElementById("hargaJual").value = Intl.NumberFormat('id', {}).format((data[4]));
            document.getElementById("barcodeBarang").value = data[5]
            
            document.getElementById("persenJual").value = (((Number(data[4].replaceAll(".", "")) - Number(data[3].replaceAll(".", ""))) / Number(data[3].replaceAll(".", ""))) * 100).toFixed(2)
        })
    })
}

$('#namaBarang').keypress(function(e){
    if (e.keyCode == 13) {
        e.preventDefault()
        document.getElementById("jumlahBarang").focus()
    }
})

$('#barcodeBarang').keypress(function(e){
    if (e.keyCode == 13) {
        e.preventDefault()
        document.getElementById("editBarangButton").click()
    }
})

$('#hargaModal').keypress(function(e){
    if (e.keyCode == 13) {
        e.preventDefault()
        document.getElementById("hargaJual").focus()
    }
    else if ((e.keyCode >= 47 && e.keyCode <= 58)) {
        e.target.value = Intl.NumberFormat('id', {}).format((e.target.value + e.key).replaceAll(".", ""));
        document.getElementById("hargaJual").value = e.target.value
        document.getElementById("persenJual").value = "0.00"
        e.preventDefault();
    } else return false
})

$('#hargaModal').keydown(function(e){
    if (e.keyCode == 8) {
        e.target.value = Intl.NumberFormat('id', {}).format(e.target.value.slice(0, -1).replaceAll(".", ""));
        document.getElementById("hargaJual").value = e.target.value
        document.getElementById("persenJual").value = "0.00"
        e.preventDefault();
    }
})

$('#jumlahBarang').keypress(function(e){
    if (e.keyCode == 13) {
        e.preventDefault()
        document.getElementById("hargaModal").focus()
    }
    else if ((e.keyCode >= 47 && e.keyCode <= 58)) {
        e.target.value = Intl.NumberFormat('id', {}).format((e.target.value + e.key).replaceAll(".", ""));
        e.preventDefault();
    } else return false
})

$('#jumlahBarang').keydown(function(e){
    if (e.keyCode == 8) {
        e.target.value = e.target.value = Intl.NumberFormat('id', {}).format(e.target.value.slice(0, -1).replaceAll(".", ""));
        e.preventDefault();
    }
})

var currentKey = [];

document.getElementById("hargaJual").addEventListener("keydown", function(e) {
    currentKey[1] = e.keyCode
    if (currentKey[1] == 13) {
        e.preventDefault()
        document.getElementById("barcodeBarang").focus()
    }
})

document.getElementById("hargaJual").addEventListener("input", function(e) {
    if ((currentKey[1] >= 47 && currentKey[1] <= 58) || currentKey[1] == 8) {
        e.target.value = Intl.NumberFormat('id', {}).format((e.target.value).replaceAll(".", ""));
        document.getElementById("persenJual").value = (((Number((e.target.value).replaceAll(".", "")) - Number($("#hargaModal").val().replaceAll(".", ""))) / Number($("#hargaModal").val().replaceAll(".", ""))) * 100).toFixed(2)
    } else e.target.value = e.target.value.slice(0, -1)
})

document.getElementById("persenJual").addEventListener("keydown", function(e) {
    currentKey[0] = e.keyCode;
    if (currentKey[0] == 13) {
        e.preventDefault()
        document.getElementById("barcodeBarang").focus()
    }
})

document.getElementById("persenJual").addEventListener("input", function(e){
    if (!$("#hargaModal").val().length) {
        Swal.mixin({
            toast: true,
            position: 'top-end',
            showConfirmButton: false,
            timer: 3000,
          }).fire({
            icon: 'error',
            title: "Mohon isi Harga Modal terlebih dahulu"
        })
        return e.target.value = e.target.value.slice(0, -1)
    }
    else if ((currentKey[0] >= 47 && currentKey[0] <= 58) || currentKey[0] == 190 || currentKey[0] == 8) {        
        document.getElementById("hargaJual").value = Intl.NumberFormat('id', {}).format(Number($("#hargaModal").val().replaceAll(".", "")) + (Number($("#hargaModal").val().replaceAll(".", "")) * (Number(e.target.value) / 100)))
    } else e.target.value = e.target.value.slice(0, -1)
});

async function editBarang() {
    if (!document.getElementById("namaBarang").value) {document.getElementById("namaBarang").focus(); return Swal.mixin({toast: true,position: 'top-end',showConfirmButton: false,timer: 3000}).fire({icon: 'error',title: 'mohon isi Nama Barang dengan benar!'})}
    else if (!document.getElementById("jumlahBarang").value) {document.getElementById("jumlahBarang").focus(); return Swal.mixin({toast: true,position: 'top-end',showConfirmButton: false,timer: 3000}).fire({icon: 'error',title: 'mohon isi Jumlah Barang dengan benar!'})}
    else if (!document.getElementById("hargaModal").value) {document.getElementById("hargaModal").focus(); return Swal.mixin({toast: true,position: 'top-end',showConfirmButton: false,timer: 3000}).fire({icon: 'error',title: 'mohon isi Harga Modal dengan benar!'})}
    else if (!document.getElementById("hargaJual").value) {document.getElementById("hargaJual").focus(); return Swal.mixin({toast: true,position: 'top-end',showConfirmButton: false,timer: 3000}).fire({icon: 'error',title: 'mohon isi Harga Jual dengan benar!'})}
    else if (parseInt(document.getElementById("jumlahBarang").value) < 1) {document.getElementById("jumlahBarang").focus(); return Swal.mixin({toast: true,position: 'top-end',showConfirmButton: false,timer: 3000}).fire({icon: 'error',title: 'Jumlah Barang tidak bisa di isi dengan 0 mohon isi harga dengan benar!'})}
    else if (parseInt(document.getElementById("hargaModal").value) < 1) {document.getElementById("hargaModal").focus(); return Swal.mixin({toast: true,position: 'top-end',showConfirmButton: false,timer: 3000}).fire({icon: 'error',title: 'Harga Modal tidak bisa di isi dengan 0 mohon isi harga dengan benar!'})}
    else if (parseInt(document.getElementById("hargaJual").value) < 1) {document.getElementById("hargaJual").focus(); return Swal.mixin({toast: true,position: 'top-end',showConfirmButton: false,timer: 3000}).fire({icon: 'error',title: 'Harga Jual tidak bisa di isi dengan 0 mohon isi harga dengan benar!'})}

    await fetch("/?api_args=6", {
        method: "POST",
        headers: {
            "barangID": window.location.search.substr(1).slice(9),
            "namaBarang": document.getElementById("namaBarang").value,
            "jumlahBarang": document.getElementById("jumlahBarang").value.replaceAll(".", ""),
            "hargaModal": document.getElementById("hargaModal").value.replaceAll(".", ""),
            "hargaJual": document.getElementById("hargaJual").value.replaceAll(".", ""),
            "barcodeBarang": document.getElementById("barcodeBarang").value,
        }
    }).then(response => {
        if (response.status == 200) {
            document.cookie = "successMessage=Barang tersebut berhasil di Edit!"
            location.replace("/daftarBarang.html")
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
        }
    })
}