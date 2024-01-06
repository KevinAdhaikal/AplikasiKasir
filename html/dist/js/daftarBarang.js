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

async function mainPage() {
    console.log("test");
    await fetch("/?api_args=3", {
        method: "POST"
    }).then(async response => {
        if (response.status == 403) {
            Swal.mixin({
                toast: true,
                position: 'top-end',
                showConfirmButton: false,
                timer: 3000,
              }).fire({
                icon: 'error',
                title: response.statusText
            })
        } else {
            await response.text().then(data => {
                data = data.split('\n');
                for (let a = 0; a < data.length - 1; a++) {
                    data[a] = data[a].split("|")
                    data[a][2] = Intl.NumberFormat('id', {}).format((data[a][2]));
                    data[a][3] = "Rp" + Intl.NumberFormat('id', {}).format((data[a][3]));
                    data[a][4] = "Rp" + Intl.NumberFormat('id', {}).format((data[a][4]));
                    data[a][6] = `<center>
                    <button type="button" class="btn btn-danger hapusBarangButton" onclick=hapusDaftarBarang(${data[a][0]})>Hapus</button>
                    <button type="button" class="btn btn-primary" onclick='infoBarang(${data[a][0]}, ${data[a][5] ? 0 : 1})'>Edit/Info</button>
                    </center>`
                    data[a][5] = data[a][5] ? data[a][5] : "Tidak Ada"
                }
                $("#daftarBarangTable").DataTable().rows.add(data.slice(0, -1)).draw(false)
                data = ""
            })
        }
    })
}

$('#modal-infoBarang').on('shown.bs.modal', function () {
    $('#namaBarang').focus();
})

$('#namaBarang').keypress(function(e) {
    if (e.keyCode == 13) {
        e.preventDefault()
        document.getElementById("jumlahBarang").focus()
    }
})

$("#jumlahBarang").keypress(function(e) {
    if (e.keyCode == 13) {
        e.preventDefault()
        document.getElementById("hargaModal").focus()
    }
})

$("#hargaModal").keypress(function(e) {
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

document.getElementById("persenJual").addEventListener("input", function(e) {
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

$('#barcodeBarang').keypress(function(e) {
    if (e.keyCode == 13) {
        e.preventDefault()
        document.getElementById("editButton").click()
    }
})

window.onload = function() {
    mainPage()
}

function infoBarang(id, isBarcodeNull) {
    var rowData = $("#daftarBarangTable").DataTable().rows(function ( idx, data, node ) {return data[0] == id ? true : false;}).data()[0]
    document.getElementById("namaBarang").value = rowData[1]
    document.getElementById("jumlahBarang").value = rowData[2]
    document.getElementById("hargaModal").value = rowData[3].slice(2)
    document.getElementById("hargaJual").value = rowData[4].slice(2)
    document.getElementById("persenJual").value = (((Number((rowData[4].slice(2)).replaceAll(".", "")) - Number(rowData[3].slice(2).replaceAll(".", ""))) / Number(rowData[3].slice(2).replaceAll(".", ""))) * 100).toFixed(2)
    document.getElementById("barcodeBarang").value = isBarcodeNull ? '' : rowData[5]
    document.getElementById("editButton").setAttribute("onclick", `editBarang(${id})`)
    $("#modal-infoBarang").modal("show")
}

async function hapusDaftarBarang(id) {
    Swal.fire({
        icon: 'warning',
        title: 'Hapus Barang',
        text: "Apakah anda yakin untuk menghapus Barang tersebut?",
        showDenyButton: true,
        denyButtonColor: '#3085d6',
        confirmButtonColor: '#d33',
        confirmButtonText: 'Iya (Enter)',
        denyButtonText: `Tidak (Esc)`,
      }).then(async result => {
        if (result.isConfirmed) {
            await fetch("/?api_args=4", {
                method: "POST",
                headers: {
                    "IDNumber": id
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
                        title: 'Barang tersebut berhasil dihapuskan!'
                    })
                    $("#daftarBarangTable").DataTable().rows(function ( idx, data, node ) {return data[0] == id ? true : false;}).remove().draw()
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

async function editBarang(id) {
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
            "barangID": id,
            "namaBarang": document.getElementById("namaBarang").value,
            "jumlahBarang": document.getElementById("jumlahBarang").value.replaceAll(".", ""),
            "hargaModal": document.getElementById("hargaModal").value.replaceAll(".", ""),
            "hargaJual": document.getElementById("hargaJual").value.replaceAll(".", ""),
            "barcodeBarang": document.getElementById("barcodeBarang").value != "Tidak Ada" ? document.getElementById("barcodeBarang").value : "",
        }
    }).then(response => {
        if (response.status == 200) {
            $("#modal-infoBarang").modal("hide")
            Swal.mixin({
                toast: true,
                position: 'top-end',
                showConfirmButton: false,
                timer: 3000,
              }).fire({
                icon: 'success',
                title: "Barang tersebut berhasil di Edit!"
            })
            $("#daftarBarangTable").DataTable().row(function ( idx, data, node ) {return Number(data[0]) == id ? true : false;}).data([
                Number(id),
                document.getElementById("namaBarang").value,
                document.getElementById("jumlahBarang").value,
                "Rp" + document.getElementById("hargaModal").value,
                "Rp" + document.getElementById("hargaJual").value,
                document.getElementById("barcodeBarang").value ? document.getElementById("barcodeBarang").value : "Tidak Ada",
                `<center>
                <button type="button" class="btn btn-danger hapusBarangButton" onclick=hapusDaftarBarang(${id})>Hapus</button>
                <button type="button" class="btn btn-primary" onclick=infoBarang(${id})>Edit/Info</button>
                </center>`
            ]).draw(false)
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