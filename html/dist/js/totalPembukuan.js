function getDaysArray(start, end) {
    for(var arr=[],dt=new Date(start); dt<=new Date(end); dt.setDate(dt.getDate()+1)){
        var obj = new Date(dt)
        arr.push(`${obj.getDate()}_${obj.getMonth() + 1}_${obj.getFullYear()}`)
    }
    return arr;
};
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

async function load() {
    var tempArray = [0, 0]
    $("#barangTerjualTable").DataTable().clear().draw()
    $("#pengeluaranTable").DataTable().clear().draw()
    var splitArray = document.getElementById("tanggalPembukuan").value.split(" - ")
    var dateArray = [splitArray[0].split("/"), splitArray[1].split("/")]

    await fetch("/?api_args=10&pembukuanArgs=7", {
        method: "POST",
        body: getDaysArray(new Date(`${dateArray[0][2]}-${dateArray[0][1]}-${dateArray[0][0]}`),new Date(`${dateArray[1][2]}-${dateArray[1][1]}-${dateArray[1][0]}`)).join("\n") 
    }).then(async response => {
        var tempData = [0, 0, 0]

        if (response.status == 200) {
            await response.json().then(data => {
                var barangTerjual = data["barangTerjual"]
                var pengeluaran = data["pengeluaran"]

                for (let a = 0; a < Object.keys(barangTerjual).length; a++) {
                    for (let b = 0; b < barangTerjual[Object.keys(barangTerjual)[a]].length; b++) {
                        var valueSplit = barangTerjual[Object.keys(barangTerjual)[a]][b].split("|");
                        tempData[0] += Number(valueSplit[1])
                        tempData[1] += Number(valueSplit[2])
                        tempData[2] += Number(valueSplit[3])
                    }

                    $("#barangTerjualTable").DataTable().row.add([`<a href="barangTerjual.html?tanggal=${Object.keys(barangTerjual)[a]}">${Object.keys(barangTerjual)[a].replaceAll("_", "/")}</a>`, Intl.NumberFormat('id', {}).format(tempData[0]), "Rp" + Intl.NumberFormat('id', {}).format(tempData[1]), "Rp" + Intl.NumberFormat('id', {}).format(tempData[2]), "Rp" + Intl.NumberFormat('id', {}).format(tempData[2] - tempData[1])]).draw()
                    tempArray[0] += tempData[2]
                    tempData[0] = 0
                    tempData[1] = 0
                    tempData[2] = 0
                }

                for (let a = 0; a < Object.keys(pengeluaran).length; a++) {
                    for (let b = 0; b < pengeluaran[Object.keys(pengeluaran)[a]].length; b++) {
                        var valueSplit = pengeluaran[Object.keys(pengeluaran)[a]][b].split("|");
                        tempData[2] += Number(valueSplit[1])
                    }

                    $("#pengeluaranTable").DataTable().row.add([`<a href="pengeluaran.html?tanggal=${Object.keys(pengeluaran)[a]}">${Object.keys(pengeluaran)[a].replaceAll("_", "/")}</a>`, "Rp" + Intl.NumberFormat('id', {}).format(tempData[2])]).draw()
                    tempArray[1] += tempData[2]
                    tempData[2] = 0
                }
            })

            $("#totalBarangTerjual").text("Rp" + Intl.NumberFormat('id', {}).format(tempArray[0]))
            $("#totalPengeluaran").text("Rp" + Intl.NumberFormat('id', {}).format(tempArray[1]))
            $("#totalUang").text("Rp" + Intl.NumberFormat('id', {}).format((tempArray[0] - tempArray[1])))
            tempArray = [0,0,0]
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

window.onload = async function() {
    $('#tanggalPembukuan').daterangepicker({
        locale: {
          format: 'DD/MM/YYYY',
        },
        startDate: new Date(),
        endDate: new Date()
    })
    await load();
}

async function kirimKeTelegram() {
    var splitArray = document.getElementById("tanggalPembukuan").value.split(" - ")
    await fetch(`${window.location.protocol}//${window.location.hostname}:8081/?username=${getCookie("username")}&password=${getCookie("password")}&teleArgs=1`, {
        method: "POST",
        body: getDaysArray(new Date(splitArray[0].replaceAll("/", "-")),new Date(splitArray[1].replaceAll("/", "-"))).join("\n") 
    }).then(async response => {
        if (response.status == 200) {
            Swal.mixin({
                toast: true,
                position: 'top-end',
                showConfirmButton: false,
                timer: 3000,
              }).fire({
                icon: 'success',
                title: "Total Pembukuan telah masuk di Telegram! mohon cek pesan di Telegram"
            })
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