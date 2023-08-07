window.onload = function() {
    document.getElementById("resetNamaBarang").checked = true
    document.getElementById("resetJumlahBarang").checked = true
    document.getElementById("resetHargaModal").checked = true
    document.getElementById("resetHargaJual").checked = true
    document.getElementById("resetBarcodeBarang").checked = true
    console.log(document.getElementById("resetBarcodeBarang").checked)
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
        document.getElementById("TambahBarangButton").click()
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