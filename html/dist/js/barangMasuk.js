var tempData = [0, 0]

async function load(dateValue) {
    await $("#barangMasukTable").DataTable().clear().draw()
    await fetch("barangMasukLogic?barangMasukArgs=1", dateValue ? {
        method: "POST",
        headers: {
            tanggalBarangMasuk: dateValue.replaceAll("-", "_")
        }
    } : {
        method: "POST"
    }).then(async response => {
        if (response.status == 200) {
            await response.text().then(data => {
                data = data.split("\n")
                for (let a = 0; a < data.length - 1; a++) {
                    data[a] = data[a].split("|")
                    tempData[0] += Number(data[a][2])
                    tempData[1] += Number(data[a][3])
                    data[a][2] = Intl.NumberFormat("id", {}).format(data[a][2])
                    data[a][3] = Intl.NumberFormat("id", {}).format(data[a][3])
                    data[a][4] = `<center>
                    <button type="button" class="btn btn-danger" onclick="deleteBarangMasuk(${data[a][0]}, '${data[a][1]}', '${dateValue ? dateValue.replaceAll("-", "_") : ""}')">Hapus</button>
                    </center>`
                }
                console.log(data)
                $("#barangMasukTable").DataTable().rows.add(data.slice(0, -1)).draw()
                if (!dateValue) document.getElementById("tanggalBarangMasuk").value = data[data.length - 1]
                $("#barangMasukTable").DataTable().columns().footer()[1].innerHTML = `Total Barang: ${Intl.NumberFormat('id', {}).format(data.length - 1)}`
                $("#barangMasukTable").DataTable().columns().footer()[2].innerHTML = `Total Jumlah Barang: ${Intl.NumberFormat('id', {}).format(tempData[0])}`
                $("#barangMasukTable").DataTable().columns().footer()[3].innerHTML = `Total Harga Barang: Rp${Intl.NumberFormat('id', {}).format(tempData[1])}`
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
            fetch("barangMasukLogic?barangMasukArgs=4", tanggal ? {
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
                    $("#barangMasukTable").DataTable().rows(function ( idx, data, node ) {return data[0] == id ? true : false;}).remove().draw()
                    var rowData = $("#barangMasukTable").DataTable().rows().data()
                    for (let a = 0; a < rowData.count() / 5; a++) {
                        tempData[0] += Number(rowData[0][2].replaceAll(".", ""))
                        tempData[1] += Number(rowData[0][3].replaceAll(".", ""))
                    }
                    $("#barangMasukTable").DataTable().columns().footer()[1].innerHTML = `Total Barang: ${Intl.NumberFormat('id', {}).format(rowData.count() / 5)}`
                    $("#barangMasukTable").DataTable().columns().footer()[2].innerHTML = `Total Jumlah Barang: ${Intl.NumberFormat('id', {}).format(tempData[0])}`
                    $("#barangMasukTable").DataTable().columns().footer()[3].innerHTML = `Total Harga Barang: Rp${Intl.NumberFormat('id', {}).format(tempData[1])}`
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

window.onload = function() {
    load()
}