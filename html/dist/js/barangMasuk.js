var tempData = [0, 0]

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

async function load(dateValue) {
    await $("#barangMasukTable").DataTable().clear().draw(false)
    await fetch("/?api_args=12&barangMasukArgs=1", dateValue ? {
        method: "POST",
        headers: {
            tanggalBarangMasuk: dateValue.replaceAll("-", "_")
        }
    } : {
        method: "POST"
    }).then(async response => {
        if (response.status == 200) {
            await response.text().then(data => {
                data = data.split("\x01")
                for (let a = 0; a < data.length - 1; a++) {
                    data[a] = data[a].split("\x02")
                    console.log(data[a])
                    tempData[0] += Number(data[a][3])
                    tempData[1] += Number(data[a][4])
                    data[a][3] = Intl.NumberFormat("id", {}).format(data[a][3])
                    data[a][4] = "Rp" + Intl.NumberFormat("id", {}).format(data[a][4])
                    data[a][5] = `<center>
                    <button type="button" class="btn btn-danger" onclick="deleteBarangMasuk(${data[a][0]}, '${data[a][2]}', '${dateValue ? dateValue.replaceAll("-", "_") : ""}')">Hapus</button>
                    </center>`
                }
                $("#barangMasukTable").DataTable().rows.add(data.slice(0, -1)).draw(false)
                if (!dateValue) document.getElementById("tanggalBarangMasuk").value = data[data.length - 1]
                $("#barangMasukTable").DataTable().columns().footer()[2].innerHTML = `Total: ${Intl.NumberFormat('id', {}).format(data.length - 1)}`
                $("#barangMasukTable").DataTable().columns().footer()[3].innerHTML = `Total: ${Intl.NumberFormat('id', {}).format(tempData[0])}`
                $("#barangMasukTable").DataTable().columns().footer()[4].innerHTML = `Total: Rp${Intl.NumberFormat('id', {}).format(tempData[1])}`
                tempData = [0, 0]
                data = ""
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
        }
    })
}

async function deleteBarangMasuk(id, nama, tanggal) {
    Swal.fire({
        icon: 'warning',
        title: 'Hapus Barang Masuk',
        text: "Apakah kamu yakin menghapus Barang Masuk tersebut?",
        showDenyButton: true,
        denyButtonColor: '#3085d6',
        confirmButtonColor: '#d33',
        confirmButtonText: 'Iya (Enter)',
        denyButtonText: `Tidak (Esc)`,
    }).then(async result => {
        if (result.isConfirmed) {
            fetch("/?api_args=12&barangMasukArgs=4", tanggal ? {
                method: "POST",
                headers: {
                    barangMasukID: id,
                    namaBarang: nama,
                    tanggalBarangMasuk: tanggal
                }
            } : {
                method: "POST",
                headers: {
                    barangMasukID: id,
                    namaBarang: nama
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
                        title: "Barang Masuk tersebut berhasil dihapus!"
                    })
                    $("#barangMasukTable").DataTable().rows(function ( idx, data, node ) {return data[0] == id ? true : false;}).remove().draw(false)
                    var rowData = $("#barangMasukTable").DataTable().rows().data()
                    for (let a = 0; a < rowData.count() / 6; a++) {
                        tempData[0] += Number(rowData[0][3].replaceAll(".", ""))
                        tempData[1] += Number(rowData[0][4].slice(2).replaceAll(".", ""))
                    }
                    $("#barangMasukTable").DataTable().columns().footer()[2].innerHTML = `Total: ${Intl.NumberFormat('id', {}).format(rowData.count() / 6)}`
                    $("#barangMasukTable").DataTable().columns().footer()[3].innerHTML = `Total: ${Intl.NumberFormat('id', {}).format(tempData[0])}`
                    $("#barangMasukTable").DataTable().columns().footer()[4].innerHTML = `Total: Rp${Intl.NumberFormat('id', {}).format(tempData[1])}`
                    tempData = [0, 0]
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
    })
}

$('#modal-tambahBarangMasuk').on('shown.bs.modal', function () {
    resetSemuaInput()
    $('#namaBarcodeBarang').focus();
})

function barangMasukModalSetup() {
    document.getElementById("namaBarcodeBarang").value = "";
    document.getElementById("jumlahBarang").value = "";
    document.getElementById("hargaBarang").value = "";
    $('#modal-tambahBarangMasuk').modal('show')
}

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
    fetch("/?api_args=12&barangMasukArgs=2", {
        method: "POST",
        headers: {
            findBarang: namaBarang
        }
    }).then(async response => {
        if (response.status == 200) {
            await response.text().then(data => {
                data = data.split("\x01");
                if (data.length > 2) {
                    $("#cariBarang").DataTable().clear().draw(false)
                    for (let a = 0; a < data.length - 1; a++) {
                        $("#cariBarang").DataTable().row.add([data[a], `<center>
                        <button type="button" class="btn btn-primary" onclick="tambahBarangMasuk(${a})">Tambah Barang</button>
                        </center>`]).draw(false)
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

$('#namaBarcodeBarang').keypress(function(e){
    if (e.keyCode == 13) {
        e.preventDefault()
        document.getElementById("checkBarangButton").click()
    }
})

$('#jumlahBarang').keypress(function(e){
    if (e.keyCode == 13) {
        e.preventDefault()
        document.getElementById("hargaBarang").focus()
    }
})

$('#hargaBarang').keypress(function(e){
    if (e.keyCode == 13) {
        e.preventDefault()
        document.getElementById("TambahBarangButton").click()
    }
})

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
    await fetch("/?api_args=12&barangMasukArgs=3", {
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
            $("#modal-tambahBarangMasuk").modal("hide");
            resetSemuaInput()
            load()
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

window.onload = function() {
    load()
}