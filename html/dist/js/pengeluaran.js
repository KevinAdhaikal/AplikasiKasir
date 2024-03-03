var tempPengeluaran = 0;

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

$('#modal-tambahPengeluaran').on('shown.bs.modal', function () {
    $('#descPengeluaran').focus();
})

$('#descPengeluaran').keypress(function(e){
    if (e.keyCode == 13) {
        e.preventDefault()
        document.getElementById("uangPengeluaran").focus()
    }
})

$('#uangPengeluaran').keypress(function(e){
    if (e.keyCode == 13) {
        e.preventDefault()
        document.getElementById("tambahPengeluaranButton").click()
    }
})
async function tambahPengeluaran() {
    if (!document.getElementById("descPengeluaran").value) {
        Swal.mixin({
            toast: true,
            position: 'top-end',
            showConfirmButton: false,
            timer: 3000,
          }).fire({
            icon: 'error',
            title: "Deskripsi Pengeluaran tidak boleh kosong!"
        })
    }
    else if (!document.getElementById("uangPengeluaran").value) {
        Swal.mixin({
            toast: true,
            position: 'top-end',
            showConfirmButton: false,
            timer: 3000,
          }).fire({
            icon: 'error',
            title: "Uang Pengeluaran tidak boleh kosong!"
        })
    }
    fetch("/?api_args=10&pembukuanArgs=4", {
        method: "POST",
        headers: {
            "descPengeluaran": document.getElementById("descPengeluaran").value,
            "uangPengeluaran": document.getElementById("uangPengeluaran").value.replaceAll(".", "")
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
                title: "Pengeluaran tersebut berhasil ditambahkan ke dalam database!"
            })
            load()
            document.getElementById("descPengeluaran").value = ""
            document.getElementById("uangPengeluaran").value = ""
            $("#modal-tambahPengeluaran").modal("hide")
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

async function load(dateValue) {
    await $("#pengeluaranTable").DataTable().clear().draw(false)
    fetch("/?api_args=10&pembukuanArgs=5", dateValue ? {
        "method": "POST",
        headers: {
            "tanggalPengeluaran": dateValue.replaceAll("-", "_")
        }
    } : {
        method: "POST"
    }).then(response => {
        if (response.status == 200) {
            response.text().then(data => {
                data = data.split("\x01")

                $("#pengeluaranTable").DataTable().columns().footer()[2].innerHTML = `Total: ${data.length - 1}`

                for (let a = 0; a < data.length - 1; a++) {
                    data[a] = data[a].split("\x02");
                    data[a][3] = "Rp" + Intl.NumberFormat('id', {}).format(Number(data[a][3].replaceAll(".", "")))
                    data[a][4] = `<center>
                    <button type="button" class="btn btn-danger" onclick="hapusPengeluaran(${data[a][0]}, '${dateValue ? dateValue.replaceAll("-", "_") : ""}')">Hapus</button>
                    </center>`
                    tempPengeluaran += Number(Number(data[a][3].replaceAll(".", "").slice(2)))
                }

                $("#pengeluaranTable").DataTable().rows.add(data.slice(0, -1)).draw(false)
                $("#pengeluaranTable").DataTable().columns().footer()[3].innerHTML = `Total: Rp${Intl.NumberFormat('id', {}).format(tempPengeluaran)}`
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
    if (window.location.href.indexOf("?tanggal=") != -1) {
        document.getElementById("tanggalPengeluaran").value = window.location.href.slice(window.location.href.indexOf("?tanggal=") + 9).replaceAll("_", "-")
        await load(window.location.href.slice(window.location.href.indexOf("?tanggal=") + 9))
    } else await load()
    
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
            await fetch("/?api_args=10&pembukuanArgs=6", dateValue ? {
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
                    $("#pengeluaranTable").DataTable().columns().footer()[2].innerHTML = "Total: " + Intl.NumberFormat('id', {}).format(Number($("#pengeluaranTable").DataTable().columns().footer()[2].innerHTML.split(": ")[1].replaceAll(".", "")) - 1)
                    $("#pengeluaranTable").DataTable().columns().footer()[3].innerHTML = "Total: Rp" + Intl.NumberFormat('id', {}).format(Number($("#pengeluaranTable").DataTable().columns().footer()[3].innerHTML.split(": Rp")[1].replaceAll(".", "") - pengeluaranData.data()[3].slice(2).replaceAll(".", "")))
                    pengeluaranData.remove().draw(false)
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