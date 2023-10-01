async function dashboardLoad() {
    await fetch("/dashboardLogic?dashboardType=1", {
        method: "POST"
    }).then(async response => {
        if (response.status == 200) {
            await response.text().then(data => {
                if (data) $("#barangTerjualID").text(Intl.NumberFormat('id', {}).format(data))
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
    await fetch("/dashboardLogic?dashboardType=2", {
        method: "POST"
    }).then(async response => {
        if (response.status == 200) {
            await response.text().then(data => {
                if (data) $("#pendapatanHargaJualID").text("Rp" + Intl.NumberFormat('id', {}).format(data))
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
    await fetch("/dashboardLogic?dashboardType=3", {
        method: "POST"
    }).then(async response => {
        if (response.status == 200) {
            await response.text().then(data => {
                if (data) $("#pengeluaranID").text("Rp" + Intl.NumberFormat('id', {}).format(data))
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
    await fetch("/dashboardLogic?dashboardType=4", {
        method: "POST"
    }).then(async response => {
        if (response.status == 200) {
            await response.text().then(data => {
                if (data) $("#keuntunganID").text("Rp" + Intl.NumberFormat('id', {}).format(data))
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

async function refreshBarangKosong() {
    await fetch("/dashboardLogic?dashboardType=5", {
        method: "POST"
    }).then(async response => {
        if (response.status == 200) {
            await response.text().then(data => {
                data = data.split("\n");
                $("#barangKosongTable").DataTable().clear().draw() // clear table
                for (let a = 0; a < data.length - 1; a++) $("#barangKosongTable").DataTable().row.add([data[a]]).draw(false)
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
    await dashboardLoad()
    $("#pendapatanID").text("Rp" + Intl.NumberFormat('id', {}).format(Number(Number($("#pendapatanHargaJualID").text().slice(2).replaceAll(".", "")) - Number($("#pengeluaranID").text().slice(2).replaceAll(".", "")))))
    await refreshBarangKosong()
}
