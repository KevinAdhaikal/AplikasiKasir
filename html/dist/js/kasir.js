var isShowDialog = 0;
var cashierSettings;
var limitStock = 0;

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

function getCookie(cname) {
    let name = cname + "=";
    let decodedCookie = decodeURIComponent(document.cookie);
    let ca = decodedCookie.split(';');
    for(let i = 0; i <ca.length; i++) {
      let c = ca[i];
      while (c.charAt(0) == ' ') {
        c = c.substring(1);
      }
      if (c.indexOf(name) == 0) {
        return c.substring(name.length, c.length);
      }
    }
    return "";
}

window.onload = async function() {
    await fetch("/?api_args=8", {
        method: "POST"
    }).then(async response => {
        await response.text().then(data => cashierSettings = data.split("\n").slice(0, -1).map(Number))

    })
    document.getElementById("inputBarang").focus()
}

var tempBarang = {}
var tempFindBarang = {}

$('#modal-editBarang').on('shown.bs.modal', function () {
    $('#jumlahBarang').focus();
})

$("#modal-editBarang").on('hidden.bs.modal', function() {
    isShowDialog = 0;
    $("#inputBarang").focus();
})

$('#modal-findBarang').on('hidden.bs.modal', function () {
    isShowDialog = 0;
    $('#inputBarang').focus();
})

$("#modal-findBarang").on("shown.bs.modal", function() {
    $('#cariBarang').DataTable().columns.adjust()
})

$(document).keydown(function(event) {
    if (event.keyCode == 27) {
      $('#modal-findBarang').modal("hide");
    }
    if (!isShowDialog) {
        if (event.keyCode == 46) hapusSemuaBarang()
        else if (event.keyCode == 119) pembayaranBarang()
    }
    
});

$("#modal-pembayaranBarang").on("shown.bs.modal", function () {
    $("#tunaiUserInput").focus()
    
}) 

$("#modal-pembayaranBarang").on("hidden.bs.modal", function () {
    $('#inputBarang').focus();
    isShowDialog = 0;
})

$('#tunaiUserInput').keypress(function(e){
    if (e.keyCode == 13) {
        e.preventDefault()
        document.getElementById("bayarButton").click()
    }
    else if ((e.keyCode >= 48 && e.keyCode <= 58)) {
        e.target.value = Intl.NumberFormat('id', {}).format((e.target.value + e.key).replaceAll(".", ""));
        if (Number(e.target.value.replaceAll(".", "")) - Number(document.getElementById("totalHargaText").innerText.replaceAll(".", "").slice(2)) < 1) document.getElementById("kembalianUangText").innerText = "Rp0"
        else document.getElementById("kembalianUangText").innerText = "Rp" + Intl.NumberFormat('id', {}).format((Number(e.target.value.replaceAll(".", "")) - Number(document.getElementById("totalHargaText").innerText.replaceAll(".", "").slice(2))))
        e.preventDefault();
    } else return false
})

$('#tunaiUserInput').keydown(function(e){
    if (e.keyCode == 8) {
        e.target.value = Intl.NumberFormat('id', {}).format(e.target.value.slice(0, -1).replaceAll(".", ""));
        if (Number(e.target.value.replaceAll(".", "")) - Number(document.getElementById("totalHargaText").innerText.replaceAll(".", "").slice(2)) < 1) document.getElementById("kembalianUangText").innerText = "Rp0"
        else document.getElementById("kembalianUangText").innerText = "Rp" + Intl.NumberFormat('id', {}).format((Number(e.target.value.replaceAll(".", "")) - Number(document.getElementById("totalHargaText").innerText.replaceAll(".", "").slice(2))))
        e.preventDefault();
    }
})

$("#inputBarang").keydown(function(e) {
    if (e.keyCode == 13) document.getElementById("tambahKasirButton").click()
})

$("#kasirTable tbody").on('click', '.hapusBarangButton', function() {
    var index = $(this).closest('tr').index();
    var tableData = $("#kasirTable").DataTable().rows(index).data()[0]
    var temp = $("#kasirTable").DataTable().columns().footer()[0].innerHTML.split(":")
    temp[1] = Number(temp[1]) - 1
    $("#kasirTable").DataTable().columns().footer()[0].innerHTML = temp.toString().replaceAll(",", ": ")
    temp = $("#kasirTable").DataTable().columns().footer()[1].innerHTML.split(":")
    temp[1] = Number(temp[1]) - Number(tableData[1])
    $("#kasirTable").DataTable().columns().footer()[1].innerHTML = temp.toString().replaceAll(",", ": ")
    temp = $("#kasirTable").DataTable().columns().footer()[3].innerHTML.split(": Rp")
    temp[1] = Intl.NumberFormat('id', {}).format(Number(temp[1].replaceAll(".", "")) - Number(tableData[3].slice(2).replaceAll(".", "")));
    $("#kasirTable").DataTable().columns().footer()[3].innerHTML = temp.toString().replaceAll(",", ": Rp")
    $("#kasirTable").DataTable().row(index).remove().draw()
    delete tempBarang[tableData[0]]
});

$("#kasirTable tbody").on("click", ".editBarang", async function() {
    isShowDialog = 1;
    
    var index = $(this).closest('tr').index();
    var tableData = $("#kasirTable").DataTable().rows(index).data()[0]
    document.getElementById("namaBarang").value = tableData[0]
    document.getElementById("jumlahBarang").value = tableData[1]
    document.getElementById("hargaModal").value = tableData[2]
    document.getElementById("hargaBarang").value = tableData[3].slice(2)

    await fetch("/?api_args=9", {
        method: "POST",
        headers: {
            inputBarang: tableData[0]
        }
    }).then(async response => {await response.text().then(data => {limitStock = Number(data)})})

    tempHarga = Number(tableData[3].replaceAll(".", ""))
    document.getElementById("jumlahBarang").setAttribute("aria-invalid", "false");
    document.getElementById("jumlahBarang").classList.remove("is-invalid")
    document.getElementById("jumlahBarang-error").style.display = "none"
        
    $("#modal-editBarang").modal("show")
    document.getElementById("jumlahBarang").focus()
    if (cashierSettings[0]) document.getElementById("editBarangButton").setAttribute("onclick", `editBarang(${index}, ${limitStock})`);
    else document.getElementById("editBarangButton").setAttribute("onclick", `editBarang(${index}, "")`);
    isShowDialog = 1;
})

async function editBarang(index, limitStock) {
    if (limitStock && (Number(limitStock) - Number(document.getElementById("jumlahBarang").value.replaceAll(".", ""))) < 0) {
        return Swal.fire({
            icon: 'error',
            title: 'Stock Barang tidak cukup',
            text: `Barang Bernama "${document.getElementById("namaBarang").value}" tidak bisa diganti Jumlah Barang menjadi ${document.getElementById("jumlahBarang").value}, karena Barang tersebut stock nya tidak lebih dari Jumlah Barang yang Anda Input`,
            confirmButtonText: "Okay"
        })
    } else {
        var total = [0, 0];
        $("#kasirTable").DataTable().row(index).data([
            document.getElementById("namaBarang").value,
            document.getElementById("jumlahBarang").value,
            Number(document.getElementById("hargaModal").value.replaceAll(".", "")),
            "Rp" + document.getElementById("hargaBarang").value,
            `<center>
            <button type="button" class="btn btn-danger hapusBarangButton">Hapus</button>
            <button type="button" class="btn btn-primary editBarang">Edit</button>
            </center>`
        ]).draw()
        for (let a = 0; a < $("#kasirTable").DataTable().rows().data().count() / 5; a++) {
            total[0] += Number($("#kasirTable").DataTable().rows().data()[a][1].replaceAll(".", ""))
            total[1] += Number($("#kasirTable").DataTable().rows().data()[a][3].slice(2).replaceAll(".", ""))
        }
    
        $("#kasirTable").DataTable().columns().footer()[1].innerHTML = `Total Jumlah Barang: ${Intl.NumberFormat('id', {}).format((total[0]))}`
        $("#kasirTable").DataTable().columns().footer()[3].innerHTML = `Total Harga: Rp${Intl.NumberFormat('id', {}).format((total[1]))}`
        $("#modal-editBarang").modal("hide")
    }
}

$('#jumlahBarang').keypress(function(e){
    if (e.keyCode == 13) {
        e.preventDefault()
        document.getElementById("editBarangButton").click()
    }
})

document.getElementById("jumlahBarang").addEventListener("input", function(e) {
    document.getElementById("hargaBarang").value = Intl.NumberFormat('id', {}).format(Number(tempBarang[document.getElementById("namaBarang").value][0]) * Number(e.target.value.replaceAll(".", "")));
    document.getElementById("hargaModal").value = Intl.NumberFormat("id", {}).format(Number(tempBarang[document.getElementById("namaBarang").value][1]) * Number(e.target.value.replaceAll(".", "")));
    if ((Number(limitStock) - Number(e.target.value.replaceAll(".", ""))) < 0 && cashierSettings[0]) {
        document.getElementById("jumlahBarang-error").style.display = null
        document.getElementById("jumlahBarang").setAttribute("aria-invalid", "true");
        document.getElementById("jumlahBarang").classList.add("is-invalid")
    } else {
        document.getElementById("jumlahBarang").setAttribute("aria-invalid", "false");
        document.getElementById("jumlahBarang").classList.remove("is-invalid")
        document.getElementById("jumlahBarang-error").style.display = "none"
    }
})

function hapusSemuaBarang() {
    Swal.fire({
        icon: 'warning',
        title: 'Hapus Semua Barang',
        text: "Apakah kamu yakin menghapus semua barang yang ada di Table kasir?",
        showDenyButton: true,
        denyButtonColor: '#3085d6',
        confirmButtonColor: '#d33',
        confirmButtonText: 'Iya (Enter)',
        denyButtonText: `Tidak (Esc)`,
      }).then((result) => {
        if (result.isConfirmed) {
            Swal.mixin({
                toast: true,
                position: 'top-end',
                showConfirmButton: false,
                timer: 3000,
              }).fire({
                icon: 'success',
                title: "Semua barang di kasir berhasil di hapuskan!"
            })
            tempBarang = {}
            $("#kasirTable").DataTable().clear().draw()
            $("#kasirTable").DataTable().columns().footer()[0].innerHTML = "Total Barang: 0"
            $("#kasirTable").DataTable().columns().footer()[1].innerHTML = "Total Jumlah Barang: 0"
            $("#kasirTable").DataTable().columns().footer()[3].innerHTML = "Total Harga: Rp0"
        }
        $("#inputBarang").focus()
    })
}

function tambahBarangKasir(namaBarang) {
    var tableData = $("#kasirTable").DataTable().rows().data()
    var total = [0,0];

    for (let a = 0; a < tableData.count() / 5; a++) {
        if (Object.keys(tempBarang)[a] == namaBarang) {
            if ((Number(tempFindBarang[namaBarang][2]) - Number(tableData[a][1].replaceAll(".", ""))) <= 0 && cashierSettings[0]) {
                document.getElementById("inputBarang").value = ""
                return Swal.fire({
                    icon: 'error',
                    title: 'Barang Kosong',
                    text: `Barang Bernama "${namaBarang}" tidak bisa ditambahkan ke Kasir, karena Barang tersebut sudah kosong`,
                    confirmButtonText: "Okay"
                  })
            }

            $("#kasirTable").DataTable().row(a).data([
                namaBarang,
                Intl.NumberFormat('id', {}).format((Number(tableData[a][1].replaceAll(".", "")) + 1)),
                Number(tempFindBarang[namaBarang][1]) * (Number(tableData[a][1].replaceAll(".", "")) + 1),
                "Rp" + Intl.NumberFormat('id', {}).format((Number(tempFindBarang[namaBarang][0]) * (Number(tableData[a][1].replaceAll(".", "")) + 1))),
                `<center>
                <button type="button" class="btn btn-danger hapusBarangButton">Hapus</button>
                <button type="button" class="btn btn-primary editBarang">Edit</button>
                </center>`
            ]).draw(false)

            for (let b = 0; b < $("#kasirTable").DataTable().rows().data().count() / 5; b++) {
                total[0] += Number($("#kasirTable").DataTable().rows().data()[b][1].replaceAll(".", ""))
                total[1] += Number($("#kasirTable").DataTable().rows().data()[b][3].slice(2).replaceAll(".", ""))
            }

            $("#kasirTable").DataTable().columns().footer()[1].innerHTML = `Total Jumlah Barang: ${Intl.NumberFormat('id', {}).format((total[0]))}`
            $("#kasirTable").DataTable().columns().footer()[3].innerHTML = `Total Harga: Rp${Intl.NumberFormat('id', {}).format((total[1]))}`

            $("#modal-findBarang").modal("hide");
            return tempFindBarang = {}
        }
    }

    if (Number(tempFindBarang[namaBarang][2]) <= 0 && cashierSettings[0]) {
        Swal.fire({
            icon: 'error',
            title: 'Barang Kosong',
            text: `Barang Bernama "${namaBarang}" tidak bisa ditambahkan ke Kasir, karena Barang tersebut sudah kosong`,
            confirmButtonText: "Okay"
          })
          return tempFindBarang = {}
    }
    
    $("#kasirTable").DataTable().row.add([namaBarang, "1", Number(tempFindBarang[namaBarang][1]), "Rp" + Intl.NumberFormat('id', {}).format(tempFindBarang[namaBarang][0]), `
    <center>
    <button type="button" class="btn btn-danger hapusBarangButton">Hapus</button>
    <button type="button" class="btn btn-primary editBarang">Edit</button>
    </center>`]).draw()

    for (let b = 0; b < $("#kasirTable").DataTable().rows().data().count() / 5; b++) {
        total[0] += Number($("#kasirTable").DataTable().rows().data()[b][1].replaceAll(".", ""))
        total[1] += Number($("#kasirTable").DataTable().rows().data()[b][3].slice(2).replaceAll(".", ""))
    }

    $("#kasirTable").DataTable().columns().footer()[0].innerHTML = `Total Barang: ${Number($("#kasirTable").DataTable().columns().footer()[0].innerHTML.split(": ")[1]) + 1}`
    $("#kasirTable").DataTable().columns().footer()[1].innerHTML = `Total Jumlah Barang: ${Intl.NumberFormat('id', {}).format((total[0]))}`
    $("#kasirTable").DataTable().columns().footer()[3].innerHTML = `Total Harga: Rp${Intl.NumberFormat('id', {}).format((total[1]))}`

    $("#modal-findBarang").modal("hide");
    tempBarang[namaBarang] = tempFindBarang[namaBarang]
    tempFindBarang = {}
}

async function findBarang(val) {
    await fetch("/?api_args=7", {
        method: "POST",
        headers: {
            inputBarang: val
        }
    }).then(async response => {
        if (response.status == 200) {
            await response.text().then(data => {
                data = data.split("\n")
                if (data.length == 2) {
                    data = data[0].split("|")
                    var tableData = $("#kasirTable").DataTable().rows().data()
                    for (let a = 0; a < tableData.count() / 5; a++) {
                        if (Object.keys(tempBarang)[a] == data[0]) {
                            if ((Number(data[1]) - Number(tableData[a][1].replaceAll(".", ""))) <= 0 && cashierSettings[0]) {
                                document.getElementById("inputBarang").value = ""
                                return Swal.fire({
                                    icon: 'error',
                                    title: 'Barang Kosong',
                                    text: `Barang Bernama "${data[0]}" tidak bisa ditambahkan ke Kasir, karena Barang tersebut sudah kosong`,
                                    confirmButtonText: "Okay"
                                  })
                            }
                            $("#kasirTable").DataTable().row(a).data([
                                data[0],
                                Intl.NumberFormat('id', {}).format((Number(tableData[a][1].replaceAll(".", "")) + 1)),
                                Number(data[2]) * (Number(tableData[a][1].replaceAll(".", "")) + 1),
                                "Rp" + Intl.NumberFormat('id', {}).format((Number(data[3].replaceAll(".", "")) * (Number(tableData[a][1].replaceAll(".", "")) + 1))),
                                `<center>
                                <button type="button" class="btn btn-danger hapusBarangButton">Hapus</button>
                                <button type="button" class="btn btn-primary editBarang">Edit</button>
                                </center>`
                            ]).draw(false)

                            var total = [0,0];
                            
                            for (let b = 0; b < tableData.count() / 5; b++) {
                                total[0] += Number($("#kasirTable").DataTable().rows().data()[b][1].replaceAll(".", ""))
                                total[1] += Number($("#kasirTable").DataTable().rows().data()[b][3].slice(2).replaceAll(".", ""))
                            }

                            $("#kasirTable").DataTable().columns().footer()[1].innerHTML = `Total Jumlah Barang: ${Intl.NumberFormat('id', {}).format((total[0]))}`
                            $("#kasirTable").DataTable().columns().footer()[3].innerHTML = `Total Harga: Rp${Intl.NumberFormat('id', {}).format((total[1]))}`
                            return
                        }
                    }

                    if (Number(data[1]) <= 0 && cashierSettings[0]) {
                        return Swal.fire({
                            icon: 'error',
                            title: 'Barang Kosong',
                            text: `Barang Bernama "${data[0]}" tidak bisa ditambahkan ke Kasir, karena Barang tersebut sudah kosong`,
                            confirmButtonText: "Okay"
                          })
                    }

                    tempBarang[data[0]] = []
                    tempBarang[data[0]][0] = Number(data[3].replaceAll(".", ""))
                    tempBarang[data[0]][1] = Number(data[2].replaceAll(".", ""))
                    $("#kasirTable").DataTable().row.add([data[0], '1', Number(data[2]), "Rp" + Intl.NumberFormat('id', {}).format((data[3])), `<center>
                    <button type="button" class="btn btn-danger hapusBarangButton">Hapus</button>
                    <button type="button" class="btn btn-primary editBarang">Edit</button>
                    </center>`]).draw(false)
                    var temp = $("#kasirTable").DataTable().columns().footer()[0].innerHTML.split(": ")
                    temp[1] = Intl.NumberFormat('id', {}).format(Number(temp[1].replaceAll(".", "")) + 1);
                    $("#kasirTable").DataTable().columns().footer()[0].innerHTML = temp.toString().replaceAll(",", ": ")
                    temp = $("#kasirTable").DataTable().columns().footer()[1].innerHTML.split(": ")
                    temp[1] = Intl.NumberFormat('id', {}).format(Number(temp[1].replaceAll(".", "")) + 1);
                    $("#kasirTable").DataTable().columns().footer()[1].innerHTML = temp.toString().replaceAll(",", ": ")
                    temp = $("#kasirTable").DataTable().columns().footer()[3].innerHTML.split(": Rp")
                    temp[1] = Intl.NumberFormat('id', {}).format(Number(temp[1].replaceAll(".", "")) + Number(data[3]));
                    $("#kasirTable").DataTable().columns().footer()[3].innerHTML = temp.toString().replaceAll(",", ": Rp")
                } else if (data.length > 2) {
                    document.getElementById("inputBarang").blur()
                    $("#cariBarang").DataTable().clear().draw()
                    isShowDialog = 1;
                    for (let a = 0; a < data.length - 1; a++) {
                        var result = data[a].split("|")
                        result[1] = Intl.NumberFormat("id", {}).format(Number(result[1]))
                        result[3] = Intl.NumberFormat('id', {}).format(Number(result[3]))
                        tempFindBarang[result[0]] = []
                        tempFindBarang[result[0]][0] = Number(result[3].replaceAll(".", ""))
                        tempFindBarang[result[0]][1] = Number(result[2].replaceAll(".", ""))
                        tempFindBarang[result[0]][2] = Number(result[1].replaceAll(".", ""))
                        $("#cariBarang").DataTable().row.add(result.concat(`<center><button type="button" class="btn btn-info" onclick="tambahBarangKasir('${result[0]}')">Tambah Barang</button></center>`)).draw(false)
                    }
                    $("#modal-findBarang").modal("show")
                    
                } else {
                    Swal.mixin({
                        toast: true,
                        position: 'top-end',
                        showConfirmButton: false,
                        timer: 3000,
                      }).fire({
                        icon: 'error',
                        title: "Barang tersebut tidak tersedia di dalam database!"
                    })
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
        }
    })
    document.getElementById('inputBarang').value = ""
}

function pembayaranBarang() {
    if ($("#kasirTable").DataTable().columns().footer()[3].innerHTML.split(": Rp")[1] == "0") {
        return Swal.mixin({
            toast: true,
            position: 'top-end',
            showConfirmButton: false,
            timer: 3000,
          }).fire({
            icon: 'error',
            title: "Mohon input barang terlebih dahhulu!"
        })
    }
    isShowDialog = 1;
    document.getElementById("totalHargaText").innerText = $("#kasirTable").DataTable().columns().footer()[3].innerHTML.split(": ")[1]
    document.getElementById("kembalianUangText").innerText = "Rp0"
    document.getElementById("tunaiUserInput").value = ""
    $("#modal-pembayaranBarang").modal("show")
}

async function bayarFunction() {
    if ((Number(document.getElementById("totalHargaText").innerText.slice(2).replaceAll(".", "")) - Number(document.getElementById("tunaiUserInput").value.replaceAll(".", ""))) > 0) {
        return Swal.mixin({
            toast: true,
            position: 'top-end',
            showConfirmButton: false,
            timer: 3000,
          }).fire({
            icon: 'error',
            title: "Uang tersebut masih kurang!"
        })
    }
    var resultData = "";
    var tableRow = $("#kasirTable").DataTable().rows().data()
    for (let a = 0; a < tableRow.count() / 5; a++) {
        resultData += `${tableRow[a][0]}|${tableRow[a][1].replaceAll(".", "")}|${tableRow[a][2]}|${tableRow[a][3].slice(2).replaceAll(".", "")}\n`
    }
    
    await fetch("/?api_args=10&pembukuanArgs=1", {
        method: "POST",
        body: resultData.slice(0, -1)
    }).then(response => {
        if (response.status == 200) {
            $("#modal-pembayaranBarang").modal("hide")
            Swal.mixin({
                toast: true,
                position: 'top-end',
                showConfirmButton: false,
                timer: 3000,
              }).fire({
                icon: 'success',
                title: "Barang tersebut berhasil masuk ke Pembukuan!"
            })
            tempBarang = {}
            document.getElementById("inputBarang").value = ""
            $("#kasirTable").DataTable().columns().footer()[0].innerHTML = "Total Barang: 0"
            $("#kasirTable").DataTable().columns().footer()[1].innerHTML = "Total Jumlah Barang: 0"
            $("#kasirTable").DataTable().columns().footer()[3].innerHTML = "Total Harga: Rp0"
            $("#kasirTable").DataTable().clear().draw()
        }
    })

    if (cashierSettings[1]) {
        fetch(`${window.location.protocol}//${window.location.hostname}:8081/?username=${getCookie("username")}&password=${getCookie("password")}&teleArgs=2`, {
            method: "POST",
            body: resultData.slice(0, -1)
        })
    }
    
}