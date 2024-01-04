window.onload = function() {
    document.getElementById("resetNamaBarang").checked = true
    document.getElementById("resetJumlahBarang").checked = true
    document.getElementById("resetHargaModal").checked = true
    document.getElementById("resetHargaJual").checked = true
    document.getElementById("resetBarcodeBarang").checked = true
}

var currentKey = 0;

$("input[data-type='currency']").on({
    input: function() {
        formatCurrency($(this));
    }
});

function formatNumber(n) {
    return n.replace(/\D/g, "").replace(/\B(?=(\d{3})+(?!\d))/g, ".");
}

function formatCurrency(input) {
    var input_val = input.val();
    if (input_val === "") return;
    var original_len = input_val.length;
    var caret_pos = input.prop("selectionStart");
    input_val = formatNumber(input_val);
    input.val(input_val);
    var updated_len = input_val.length;
    caret_pos = updated_len - original_len + caret_pos;
    input[0].setSelectionRange(caret_pos, caret_pos);
}

$('#namaBarang').keypress(function(e){
    if (e.keyCode == 13) {
        e.preventDefault()
        document.getElementById("jumlahBarang").focus()
    }
})

$('#jumlahBarang').keypress(function(e){
    if (e.keyCode == 13) {
        e.preventDefault()
        document.getElementById("hargaModal").focus()
    }
})

$('#hargaModal').keypress(function(e){
    if (e.keyCode == 13) {
        e.preventDefault()
        document.getElementById("hargaJual").focus()
    }
})

document.getElementById("hargaModal").addEventListener("input", function(e) {
    document.getElementById("hargaJual").value = document.getElementById("hargaModal").value
    document.getElementById("persenJual").value = "0.00"
})

$("#hargaJual").keypress(function(e) {
    if (e.keyCode == 13) {
        e.preventDefault()
        document.getElementById("barcodeBarang").focus()
    }
})

document.getElementById("hargaJual").addEventListener("input", function(e) {
    document.getElementById("persenJual").value = (((Number((e.target.value).replaceAll(".", "")) - Number($("#hargaModal").val().replaceAll(".", ""))) / Number($("#hargaModal").val().replaceAll(".", ""))) * 100).toFixed(2)
})

document.getElementById("persenJual").addEventListener("keydown", function(e) {
    currentKey = e.keyCode;
    if (currentKey == 13) {
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
    else if ((currentKey >= 47 && currentKey <= 58) || currentKey == 190 || currentKey == 8) {        
        document.getElementById("hargaJual").value = Intl.NumberFormat('id', {}).format(Number($("#hargaModal").val().replaceAll(".", "")) + (Number($("#hargaModal").val().replaceAll(".", "")) * (Number(e.target.value) / 100)))
    } else e.target.value = e.target.value.slice(0, -1)
});

$('#barcodeBarang').keypress(function(e){
    if (e.keyCode == 13) {
        e.preventDefault()
        document.getElementById("TambahBarangButton").click()
    }
})

async function tambahBarang() {
    if (!document.getElementById("namaBarang").value) {document.getElementById("namaBarang").focus(); return Swal.mixin({toast: true,position: 'top-end',showConfirmButton: false,timer: 3000}).fire({icon: 'error',title: 'mohon isi Nama Barang dengan benar!'})}
    else if (!document.getElementById("jumlahBarang").value) {document.getElementById("jumlahBarang").focus(); return Swal.mixin({toast: true,position: 'top-end',showConfirmButton: false,timer: 3000}).fire({icon: 'error',title: 'mohon isi Jumlah Barang dengan benar!'})}
    else if (!document.getElementById("hargaModal").value) {document.getElementById("hargaModal").focus(); return Swal.mixin({toast: true,position: 'top-end',showConfirmButton: false,timer: 3000}).fire({icon: 'error',title: 'mohon isi Harga Modal dengan benar!'})}
    else if (!document.getElementById("hargaJual").value) {document.getElementById("hargaJual").focus(); return Swal.mixin({toast: true,position: 'top-end',showConfirmButton: false,timer: 3000}).fire({icon: 'error',title: 'mohon isi Harga Jual dengan benar!'})}
    else if (parseInt(document.getElementById("jumlahBarang").value) < 1) {document.getElementById("jumlahBarang").focus(); return Swal.mixin({toast: true,position: 'top-end',showConfirmButton: false,timer: 3000}).fire({icon: 'error',title: 'Jumlah Barang tidak bisa di isi dengan 0 mohon isi harga dengan benar!'})}
    else if (parseInt(document.getElementById("hargaModal").value) < 1) {document.getElementById("hargaModal").focus(); return Swal.mixin({toast: true,position: 'top-end',showConfirmButton: false,timer: 3000}).fire({icon: 'error',title: 'Harga Modal tidak bisa di isi dengan 0 mohon isi harga dengan benar!'})}
    else if (parseInt(document.getElementById("hargaJual").value) < 1) {document.getElementById("hargaJual").focus(); return Swal.mixin({toast: true,position: 'top-end',showConfirmButton: false,timer: 3000}).fire({icon: 'error',title: 'Harga Jual tidak bisa di isi dengan 0 mohon isi harga dengan benar!'})}
    await fetch("/?api_args=2", {
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
