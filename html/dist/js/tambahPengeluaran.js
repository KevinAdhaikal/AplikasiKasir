window.onload = function() {
    document.getElementById("descPengeluaran").focus()
}

async function tambahPengeluaran() {
    fetch("/pembukuan?pembukuanArgs=4", {
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
            document.getElementById("descPengeluaran").value = ""
            document.getElementById("uangPengeluaran").value = ""
            document.getElementById("descPengeluaran").focus()
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
    else if ((e.keyCode >= 47 && e.keyCode <= 58)) {
        e.target.value = Intl.NumberFormat('id', {}).format((e.target.value + e.key).replaceAll(".", ""));
        e.preventDefault();
    } else return false
})

$('#uangPengeluaran').keydown(function(e){
    if (e.keyCode == 8) {
        e.target.value = e.target.value = Intl.NumberFormat('id', {}).format(e.target.value.slice(0, -1).replaceAll(".", ""));
        e.preventDefault();
    }
})