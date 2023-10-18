var tempPos = 0;

var enableDays = [];
var currentData = [];
var mergeArray = {};
var tempSplit;
var tempPembukuan = [0, 0, 0];

async function mergeFunction() {
    var dataTable = $("#barangTerjualTable").DataTable().rows().data();
    for (let a = 0; a < dataTable.count() / 7; a++) {
        if (mergeArray[dataTable[a][2]]) {
            mergeArray[dataTable[a][2]] = [Object.keys(mergeArray).indexOf(dataTable[a][2]),
            dataTable[a][2],
            Intl.NumberFormat('id', {}).format(Number(dataTable[a][3].replaceAll(".", "")) + Number(mergeArray[dataTable[a][2]][2].replaceAll(".", ""))),
            Intl.NumberFormat('id', {}).format(Number(dataTable[a][4].slice(2).replaceAll(".", "")) + Number(mergeArray[dataTable[a][2]][3].replaceAll(".", ""))),
            Intl.NumberFormat('id', {}).format(Number(dataTable[a][5].slice(2).replaceAll(".", "")) + Number(mergeArray[dataTable[a][2]][4].replaceAll(".", "")))]
        } else {
            mergeArray[dataTable[a][2]] = [0, dataTable[a][2], dataTable[a][3], dataTable[a][4].slice(2), dataTable[a][5].slice(2)]
            mergeArray[dataTable[a][2]][0] = Object.keys(mergeArray).indexOf(dataTable[a][2])
        }
    }
    await $("#barangTerjualTable").DataTable().clear().draw()

    for (let a = 0; a < Object.keys(mergeArray).length; a++) {
        var name = Object.keys(mergeArray)[a]
        console.log(mergeArray[name])
        $("#barangTerjualTable").DataTable().row.add([mergeArray[name][0], '-', mergeArray[name][1], mergeArray[name][2], "Rp" + mergeArray[name][3], "Rp" + mergeArray[name][4], `<center>
        <button type="button" class="btn btn-danger" onclick="hapusBarangTerjualMerge('${name}', '${document.getElementById("tanggalBarangTerjual").value}', ${a})">Hapus</button>
        </center>`]).draw()
    }
    
    $("#barangTerjualTable").DataTable().columns().footer()[2].innerHTML = `Total: ${Object.keys(mergeArray).length}`
    mergeArray = {}
}

async function load(dateValue) {
    await $("#barangTerjualTable").DataTable().clear().draw()
    await fetch("/pembukuan?pembukuanArgs=2", dateValue ? {
        method: "POST",
        headers: {
            tanggalPembukuan: dateValue.replaceAll("-", "_")
        }
    } : {
        method: "POST"
    }).then(async response => {
        if (response.status == 200) {
            await response.text().then(data => {
                data = data.split("\n");
                $("#barangTerjualTable").DataTable().columns().footer()[2].innerHTML = `Total: ${data.length - 1}`
                for (let a = 0; a < data.length - 1; a++) {
                    data[a] = data[a].split("|")
                    data[a][3] = Intl.NumberFormat('id', {}).format(Number(data[a][3].replaceAll(".", "")))
                    data[a][4] = "Rp" + Intl.NumberFormat('id', {}).format(Number(data[a][4].replaceAll(".", "")))
                    data[a][5] = "Rp" + Intl.NumberFormat('id', {}).format(Number(data[a][5].replaceAll(".", "")))
                    data[a][6] = `<center>
                    <button type="button" class="btn btn-danger" onclick="hapusBarangTerjual(${data[a][0]}, '${dateValue ? dateValue.replaceAll("-", "_") : ""}')">Hapus</button>
                    </center>`

                    tempPembukuan[0] += Number(data[a][3].replaceAll(".", ""))
                    tempPembukuan[1] += Number(data[a][4].replaceAll(".", "").slice(2))
                    tempPembukuan[2] += Number(data[a][5].replaceAll(".", "").slice(2))
                }
                $("#barangTerjualTable").DataTable().rows.add(data.slice(0, -1)).draw(false)
                if (!dateValue) document.getElementById("tanggalBarangTerjual").value = data[data.length - 1]
                $("#barangTerjualTable").DataTable().columns().footer()[3].innerHTML = `Total: ${Intl.NumberFormat('id', {}).format(tempPembukuan[0])}`
                $("#barangTerjualTable").DataTable().columns().footer()[4].innerHTML = `Total: Rp${Intl.NumberFormat('id', {}).format(tempPembukuan[1])}`
                $("#barangTerjualTable").DataTable().columns().footer()[5].innerHTML = `Total: Rp${Intl.NumberFormat('id', {}).format(tempPembukuan[2])}` 
            })
            tempPembukuan = [0, 0, 0]
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

window.onload = async function() {
    $('#tanggalBarangTerjual').datepicker({
        dateFormat: 'd-m-yy',
        changeMonth: true,
        changeYear: true,
        beforeShow: function (input, inst) {
            var rect = input.getBoundingClientRect();
            setTimeout(function () {
                inst.dpDiv.css({ top: rect.top + 40, left: rect.left + 0 });
            }, 0);
        }
    })
    await load()
}

function hapusBarangTerjual(id, dateValue) {
    Swal.fire({
        icon: 'warning',
        title: 'Hapus Barang Terjual',
        text: "Apakah kamu yakin menghapus Barang Terjual tersebut?",
        showDenyButton: true,
        denyButtonColor: '#3085d6',
        confirmButtonColor: '#d33',
        confirmButtonText: 'Iya (Enter)',
        denyButtonText: `Tidak (Esc)`,
      }).then(async result => {
        if (result.isConfirmed) {
            fetch(`/pembukuan?pembukuanArgs=3&idBarang=${id}`, dateValue ? {
                method: "POST",
                headers: {
                    "tanggalPembukuan": dateValue.replaceAll("-", "_")
                }
            } : {
                method: "POST"
            }).then(response => {
                if (response.status == 200) {
                    Swal.mixin({
                        toast: true,
                        position: 'top-end',
                        showConfirmButton: false,
                        timer: 3000,
                      }).fire({
                        icon: 'success',
                        title: "barang Terjual tersebut berhasil dihapus!"
                    })

                    var barangTerjualData = $("#barangTerjualTable").DataTable().row(function ( idx, data, node ) {return data[0] == id ? true : false;});
                    $("#barangTerjualTable").DataTable().columns().footer()[2].innerHTML = "Total: " + Intl.NumberFormat('id', {}).format(Number($("#barangTerjualTable").DataTable().columns().footer()[2].innerHTML.split(": ")[1].replaceAll(".", "")) - 1)
                    $("#barangTerjualTable").DataTable().columns().footer()[3].innerHTML = "Total: " + Intl.NumberFormat('id', {}).format(Number($("#barangTerjualTable").DataTable().columns().footer()[3].innerHTML.split(": ")[1].replaceAll(".", "") - barangTerjualData.data()[3].replaceAll(".", "")))
                    $("#barangTerjualTable").DataTable().columns().footer()[4].innerHTML = "Total: Rp" + Intl.NumberFormat('id', {}).format(Number($("#barangTerjualTable").DataTable().columns().footer()[4].innerHTML.split(": Rp")[1].replaceAll(".", "") - barangTerjualData.data()[4].slice(2).replaceAll(".", "")))
                    $("#barangTerjualTable").DataTable().columns().footer()[5].innerHTML = "Total: Rp" + Intl.NumberFormat('id', {}).format(Number($("#barangTerjualTable").DataTable().columns().footer()[5].innerHTML.split(": Rp")[1].replaceAll(".", "") - barangTerjualData.data()[5].slice(2).replaceAll(".", "")))
                    barangTerjualData.remove().draw()
                    
                    //load(dateValue ? dateValue.replaceAll("-", "_") : "");
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
    })
}

function hapusBarangTerjualMerge(namaBarang, valueDate, id) {
    Swal.fire({
        icon: 'warning',
        title: 'Hapus Barang Terjual',
        text: "Apakah kamu yakin menghapus Barang Terjual tersebut?",
        showDenyButton: true,
        denyButtonColor: '#3085d6',
        confirmButtonColor: '#d33',
        confirmButtonText: 'Iya (Enter)',
        denyButtonText: `Tidak (Esc)`,
      }).then(async result => {
        if (result.isConfirmed) {
            fetch(`/pembukuan?pembukuanArgs=3`, valueDate ? {
                method: "POST",
                headers: {
                    "tanggalPembukuan": valueDate.replaceAll("-", "_"),
                    "namaBarang": namaBarang
                }
            } : {
                method: "POST",
                headers: {
                    "namaBarang": namaBarang
                }
            }).then(async response => {
                if (response.status == 200) {
                    Swal.mixin({
                        toast: true,
                        position: 'top-end',
                        showConfirmButton: false,
                        timer: 3000,
                    }).fire({
                        icon: 'success',
                        title: "Barang Terjual tersebut berhasil dihapus!"
                    })
                    var barangTerjualData = $("#barangTerjualTable").DataTable().row(function ( idx, data, node ) {return data[0] == id ? true : false;});
                    $("#barangTerjualTable").DataTable().columns().footer()[2].innerHTML = "Total: " + Intl.NumberFormat('id', {}).format(Number($("#barangTerjualTable").DataTable().columns().footer()[2].innerHTML.split(": ")[1].replaceAll(".", "")) - 1)
                    $("#barangTerjualTable").DataTable().columns().footer()[3].innerHTML = "Total: " + Intl.NumberFormat('id', {}).format(Number($("#barangTerjualTable").DataTable().columns().footer()[3].innerHTML.split(": ")[1].replaceAll(".", "") - barangTerjualData.data()[3].replaceAll(".", "")))
                    $("#barangTerjualTable").DataTable().columns().footer()[4].innerHTML = "Total: Rp" + Intl.NumberFormat('id', {}).format(Number($("#barangTerjualTable").DataTable().columns().footer()[4].innerHTML.split(": Rp")[1].replaceAll(".", "") - barangTerjualData.data()[4].slice(2).replaceAll(".", "")))
                    $("#barangTerjualTable").DataTable().columns().footer()[5].innerHTML = "Total: Rp" + Intl.NumberFormat('id', {}).format(Number($("#barangTerjualTable").DataTable().columns().footer()[5].innerHTML.split(": Rp")[1].replaceAll(".", "") - barangTerjualData.data()[5].slice(2).replaceAll(".", "")))
                    barangTerjualData.remove().draw()
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
    })
}

async function applyBarangTerjual() {
    await load(document.getElementById("tanggalBarangTerjual").value ? document.getElementById("tanggalBarangTerjual").value : "")
    if ($("#mergeBarangTerjual").is(':checked')) await mergeFunction();
    /*var isDoing = false;
    if (currentData[0] != document.getElementById("tanggalBarangTerjual").value) {
        await load(document.getElementById("tanggalBarangTerjual").value)
        isDoing = true;
        currentData[0] = document.getElementById("tanggalBarangTerjual").value
    }
    
    else {
        if (!isDoing) await load(document.getElementById("tanggalBarangTerjual").value)
    }*/
}