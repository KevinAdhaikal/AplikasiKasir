var tempPengeluaran = 0;

async function load(dateValue) {
    await $("#pengeluaranTable").DataTable().clear().draw()
    fetch("/pembukuan?pembukuanArgs=5", dateValue ? {
        "method": "POST",
        headers: {
            "tanggalPengeluaran": dateValue.replaceAll("-", "_")
        }
    } : {
        method: "POST"
    }).then(response => {
        if (response.status == 200) {
            response.text().then(data => {
                data = data.split("\n")

                $("#pengeluaranTable").DataTable().columns().footer()[1].innerHTML = `Total: ${data.length - 1}`

                for (let a = 0; a < data.length - 1; a++) {
                    data[a] = data[a].split("|");
                    data[a][2] = "Rp" + Intl.NumberFormat('id', {}).format(Number(data[a][2].replaceAll(".", "")))
                    data[a][3] = `<center>
                    <button type="button" class="btn btn-danger" onclick="hapusPengeluaran(${data[a][0]}, '${dateValue ? dateValue.replaceAll("-", "_") : ""}')">Hapus</button>
                    </center>`
                    tempPengeluaran += Number(Number(data[a][2].replaceAll(".", "").slice(2)))
                }

                $("#pengeluaranTable").DataTable().rows.add(data.slice(0, -1)).draw(false)
                $("#pengeluaranTable").DataTable().columns().footer()[2].innerHTML = `Total: Rp${Intl.NumberFormat('id', {}).format(tempPengeluaran)}`
                if (!dateValue) document.getElementById("tanggalPengeluaran").value = data[data.length - 1]
                tempPengeluaran = 0;
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

window.onload = async function() {
    $('#tanggalPengeluaran').datepicker({
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

async function hapusPengeluaran(id, dateValue) {
    Swal.fire({
        icon: 'warning',
        title: 'Hapus Pengeluaran',
        text: "Apakah kamu yakin menghapus pengeluaran tersebut?",
        showDenyButton: true,
        denyButtonColor: '#3085d6',
        confirmButtonColor: '#d33',
        confirmButtonText: 'Iya (Enter)',
        denyButtonText: `Tidak (Esc)`,
      }).then(async result => {
        if (result.isConfirmed) {
            await fetch("/pembukuan?pembukuanArgs=6", dateValue ? {
                method: "POST",
                headers: {
                    "rowPengeluaran": id,
                    "tanggalPengeluaran": dateValue
                }
            } : {
                method: "POST",
                headers: {
                    "rowPengeluaran": id
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
                        title: "Pengeluaran tersebut berhasil dihapuskan dalam database!"
                    })
                    var pengeluaranData = $("#pengeluaranTable").DataTable().row(function ( idx, data, node ) {return data[0] == id ? true : false;});
                    $("#pengeluaranTable").DataTable().columns().footer()[1].innerHTML = "Total: " + Intl.NumberFormat('id', {}).format(Number($("#pengeluaranTable").DataTable().columns().footer()[1].innerHTML.split(": ")[1].replaceAll(".", "")) - 1)
                    $("#pengeluaranTable").DataTable().columns().footer()[2].innerHTML = "Total: Rp" + Intl.NumberFormat('id', {}).format(Number($("#pengeluaranTable").DataTable().columns().footer()[2].innerHTML.split(": Rp")[1].replaceAll(".", "") - pengeluaranData.data()[2].slice(2).replaceAll(".", "")))
                    pengeluaranData.remove().draw()
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