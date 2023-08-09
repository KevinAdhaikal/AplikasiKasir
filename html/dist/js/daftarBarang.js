async function mainPage() {
    var splitCookie = document.cookie.split("; ");

    for (let a = 0; a < splitCookie.length; a++) if (splitCookie[a].includes("successMessage")) {
        Swal.mixin({
            toast: true,
            position: 'top-end',
            showConfirmButton: false,
            timer: 3000,
          }).fire({
            icon: 'success',
            title: splitCookie[a].slice(15)
        })

        document.cookie = "successMessage=; max-age=0;"
    }
    await fetch("/daftarBarang", {
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
                    data[a][3] = Intl.NumberFormat('id', {}).format((data[a][3]));
                    data[a][4] = Intl.NumberFormat('id', {}).format((data[a][4]));
                    data[a][5] = data[a][5] ? data[a][5] : "Tidak Ada"
                    data[a][6] = `<center>
                    <button type="button" class="btn btn-danger hapusBarangButton" onclick=hapusDaftarBarang(${data[a][0]})>Hapus</button>
                    <button type="button" class="btn btn-info" onclick=editDaftarBarang(${data[a][0]})>Edit</button>
                    <button type="button" class="btn btn-secondary" onclick=infoBarang(${data[a][0]})>Info</button>
                    </center>`
                }
                $("#daftarBarangTable").DataTable().rows.add(data.slice(0, -1)).draw(false)
                data = ""
            })
        }
    })
}
window.onload = function(){
    mainPage()
}
function infoBarang(id) {
    var rowData = $("#daftarBarangTable").DataTable().rows(function ( idx, data, node ) {return data[0] == id ? true : false;}).data()[0]
    document.getElementById("namaBarang").value = rowData[1]
    document.getElementById("jumlahBarang").value = rowData[2]
    document.getElementById("hargaModal").value = rowData[3]
    document.getElementById("hargaJual").value = rowData[4]
    document.getElementById("persenJual").value = (((Number((rowData[4]).replaceAll(".", "")) - Number(rowData[3].replaceAll(".", ""))) / Number(rowData[3].replaceAll(".", ""))) * 100).toFixed(2) 
    document.getElementById("barcodeBarang").value = rowData[5] ? rowData[5] : "Tidak Ada"
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
            await fetch("/hapusDaftarBarang", {
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

function editDaftarBarang(id) {
    location.replace(`/editBarang.html?barangID=${id}`)
}