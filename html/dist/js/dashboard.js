Date.prototype.addDays = function(days) {
    var date = new Date(this.valueOf());
    date.setDate(date.getDate() + days);
    return date;
}

Date.prototype.addMonth = function(month) {
    var date = new Date(this.valueOf());
    date.setMonth(date.getMonth() + month);
    return date;
}

Date.prototype.addYear = function(year) {
    var date = new Date(this.valueOf());
    date.setUTCFullYear(date.getUTCFullYear() + year);
    return date;
}

function dmyyyy(date) {
    return [date.getDate(), date.getMonth() + 1, date.getFullYear()].join('/');
}

var pengaturan

async function dashboardLoad() {
    await fetch("/?api_args=13&pengaturanArgs=4", {
        method: "POST"
    }).then(async response => {
        if (response.status == 200) {
            await response.text().then(async data => {
                data = data.split("\n")
                pengaturan = data
                if (data[1] == '1') {
                    await loadDate(Number(data[0]), true)
                    document.getElementById("date_select").selectedIndex = Number(data[0])
                    await refreshBarangTotalTerjual()    
                } else await loadDate(document.getElementById("date_select").selectedIndex, true)
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
    await fetch("/?api_args=11&dashboardType=1", {
        method: "POST"
    }).then(async response => {
        if (response.status == 200) {
            await response.text().then(data => {
                if (data) {
                    data = data.split("|")
                    $("#barangTerjualID").text(Intl.NumberFormat('id', {}).format(data[0]))
                    $("#pendapatanHargaJualID").text("Rp" + Intl.NumberFormat('id', {}).format(data[1]))
                    $("#keuntunganID").text("Rp" + Intl.NumberFormat('id', {}).format(data[2]))
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
    await fetch("/?api_args=11&dashboardType=2", {
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
    if (pengaturan[2] == '1') {
        await fetch("/?api_args=11&dashboardType=5", {
            method: "POST"
        }).then(async response => {
            if (response.status == 200) {
                await response.text().then(data => {
                    document.getElementById("barangTerjualText").innerHTML = "Barang Terjual (Hari Ini / Kemarin)"
                    document.getElementById("totalHargaJualText").innerHTML = "Total Harga Jual (Hari Ini / Kemarin)"
                    document.getElementById("totalPengeluaranText").innerHTML = "Total Pengeluaran (Hari Ini / Kemarin)"
                    document.getElementById("totalKeuntunganText").innerHTML = "Total Keuntungan (Hari Ini / Kemarin)"
                    document.getElementById("totalPendapatanText").innerHTML = "Total Pendapatan (Hari Ini / Kemarin)"
                    data = data.split("\n");
                    data[0] = data[0].split("|")
                    $("#barangTerjualID").text($("#barangTerjualID").text() + " / " + Intl.NumberFormat('id', {}).format(data[0][0]))
                    $("#pendapatanHargaJualID").text($("#pendapatanHargaJualID").text() + " / Rp" + Intl.NumberFormat('id', {}).format(data[0][1]))
                    $("#keuntunganID").text($("#keuntunganID").text() + " / Rp" + Intl.NumberFormat('id', {}).format(data[0][2]))
                    $("#pengeluaranID").text($("#pengeluaranID").text() + " / Rp" + Intl.NumberFormat('id', {}).format(data[1]))
                    $("#pendapatanID").text("Rp" + Intl.NumberFormat('id', {}).format(Number(Number($("#pendapatanHargaJualID").text().split(" / ")[0].slice(2).replaceAll(".", "")) - Number($("#pengeluaranID").text().split(" / ")[0].slice(2).replaceAll(".", "")))) + " / Rp" + Intl.NumberFormat('id', {}).format(Number(Number($("#pendapatanHargaJualID").text().split(" / ")[1].slice(2).replaceAll(".", "")) - Number($("#pengeluaranID").text().split(" / ")[1].slice(2).replaceAll(".", "")))))

                    console.log($("#pendapatanHargaJualID").text().split(" / ")[1].slice(2))
                    document.getElementById("barangTerjualText").innerHTML += ` <i class="${Number($("#barangTerjualID").text().split(" / ")[0].replaceAll(".", "")) > Number($("#barangTerjualID").text().split(" / ")[1].replaceAll(".", "")) ? "ion ion-android-arrow-up text-success" : "ion ion-android-arrow-down text-danger"}"></i>`
                    document.getElementById("totalHargaJualText").innerHTML += ` <i class="${Number($("#pendapatanHargaJualID").text().split(" / ")[0].slice(2).replaceAll(".", "")) > Number($("#pendapatanHargaJualID").text().split(" / ")[1].slice(2).replaceAll(".", "")) ? "ion ion-android-arrow-up text-success" : "ion ion-android-arrow-down text-danger"}"></i>`
                    document.getElementById("totalKeuntunganText").innerHTML += ` <i class="${Number($("#keuntunganID").text().split(" / ")[0].slice(2).replaceAll(".", "")) > Number($("#keuntunganID").text().split(" / ")[1].slice(2).replaceAll(".", "")) ? "ion ion-android-arrow-up text-success" : "ion ion-android-arrow-down text-danger"}"></i>`
                    //document.getElementById("totalPengeluaranText").innerHTML += ` <i class="${Number($("#pengeluaranID").text().split(" / ")[0].slice(2).replaceAll(".", "")) > Number($("#pengeluaranID").text().split(" / ")[1].slice(2).replaceAll(".", "")) ? "ion ion-android-arrow-up text-success" : "ion ion-android-arrow-down text-danger"}"></i>`
                    document.getElementById("totalPendapatanText").innerHTML += ` <i class="${Number($("#pendapatanID").text().split(" / ")[0].slice(2).replaceAll(".", "")) > Number($("#pendapatanID").text().split(" / ")[1].slice(2).replaceAll(".", "")) ? "ion ion-android-arrow-up text-success" : "ion ion-android-arrow-down text-danger"}"></i>`
                })
            }
        })
    } else {
        $("#pendapatanID").text("Rp" + Intl.NumberFormat('id', {}).format(Number(Number($("#pendapatanHargaJualID").text().slice(2).replaceAll(".", "")) - Number($("#pengeluaranID").text().slice(2).replaceAll(".", "")))))
    }
}

async function loadDate(target, is_first_load) {
    if (!is_first_load) $('#dateBarangSeringTerjual').data('daterangepicker').remove()
    if (target == 3) {
        return $('#dateBarangSeringTerjual').daterangepicker({
            showDropdowns: true,
            locale: {
              format: 'D/M/YYYY',
            }
        })
    }
    
    await $('#dateBarangSeringTerjual').daterangepicker({
        singleDatePicker: true,
        showDropdowns: true,
        locale: {
          format: 'D/M/YYYY',
        }
    })
    var newDate = new Date()
    
    switch (target) {
        case 0: {
            await newDate.setDate(newDate.getDate() - 6)
            $('#dateBarangSeringTerjual').data('daterangepicker').setStartDate(dmyyyy(newDate));
            $('#dateBarangSeringTerjual').data('daterangepicker').setEndDate(dmyyyy(newDate.addDays(6)));
            $('#dateBarangSeringTerjual').val(`${dmyyyy(newDate)} - ${dmyyyy(newDate.addDays(6))}`)
            
            $('#dateBarangSeringTerjual').on('apply.daterangepicker', function(ev, picker) {
                var currentDate = new Date(picker.startDate)
                $('#dateBarangSeringTerjual').data('daterangepicker').setStartDate(dmyyyy(currentDate));
                $('#dateBarangSeringTerjual').data('daterangepicker').setEndDate(dmyyyy(currentDate.addDays(6)));
                $('#dateBarangSeringTerjual').val(`${dmyyyy(currentDate)} - ${dmyyyy(currentDate.addDays(6))}`)
            });
            
            $('#dateBarangSeringTerjual').on('hide.daterangepicker', function(ev, picker) {
                var currentDate = new Date(picker.startDate)
                $('#dateBarangSeringTerjual').data('daterangepicker').setStartDate(dmyyyy(currentDate));
                $('#dateBarangSeringTerjual').data('daterangepicker').setEndDate(dmyyyy(currentDate.addDays(6)));
                $('#dateBarangSeringTerjual').val(`${dmyyyy(currentDate)} - ${dmyyyy(currentDate.addDays(6))}`)
            });
            break;
        }
        case 1: {
            await newDate.setMonth(newDate.getMonth() - 1)
            $('#dateBarangSeringTerjual').data('daterangepicker').setStartDate(dmyyyy(newDate));
            $('#dateBarangSeringTerjual').data('daterangepicker').setEndDate(dmyyyy(newDate.addMonth(1)));
            $('#dateBarangSeringTerjual').val(`${dmyyyy(newDate)} - ${dmyyyy(newDate.addMonth(1))}`)

            $('#dateBarangSeringTerjual').on('apply.daterangepicker', function(ev, picker) {
                var currentDate = new Date(picker.startDate)
                $('#dateBarangSeringTerjual').data('daterangepicker').setStartDate(dmyyyy(currentDate));
                $('#dateBarangSeringTerjual').data('daterangepicker').setEndDate(dmyyyy(currentDate.addMonth(1)));
                $('#dateBarangSeringTerjual').val(`${dmyyyy(currentDate)} - ${dmyyyy(currentDate.addMonth(1))}`)
            });
            
            $('#dateBarangSeringTerjual').on('hide.daterangepicker', function(ev, picker) {
                var currentDate = new Date(picker.startDate)
                $('#dateBarangSeringTerjual').data('daterangepicker').setStartDate(dmyyyy(currentDate));
                $('#dateBarangSeringTerjual').data('daterangepicker').setEndDate(dmyyyy(currentDate.addMonth(1)));
                $('#dateBarangSeringTerjual').val(`${dmyyyy(currentDate)} - ${dmyyyy(currentDate.addMonth(1))}`)
            });
            break;
        }
        case 2: {
            await newDate.setUTCFullYear(newDate.getUTCFullYear() - 1)
            $('#dateBarangSeringTerjual').data('daterangepicker').setStartDate(dmyyyy(newDate));
            $('#dateBarangSeringTerjual').data('daterangepicker').setEndDate(dmyyyy(newDate.addYear(1)));
            $('#dateBarangSeringTerjual').val(`${dmyyyy(newDate)} - ${dmyyyy(newDate.addYear(1))}`)

            $('#dateBarangSeringTerjual').on('apply.daterangepicker', function(ev, picker) {
                var currentDate = new Date(picker.startDate)
                $('#dateBarangSeringTerjual').data('daterangepicker').setStartDate(dmyyyy(currentDate));
                $('#dateBarangSeringTerjual').data('daterangepicker').setEndDate(dmyyyy(currentDate.addYear(1)));
                $('#dateBarangSeringTerjual').val(`${dmyyyy(currentDate)} - ${dmyyyy(currentDate.addYear(1))}`)
            });
            
            $('#dateBarangSeringTerjual').on('hide.daterangepicker', function(ev, picker) {
                var currentDate = new Date(picker.startDate)
                $('#dateBarangSeringTerjual').data('daterangepicker').setStartDate(dmyyyy(currentDate));
                $('#dateBarangSeringTerjual').data('daterangepicker').setEndDate(dmyyyy(currentDate.addYear(1)));
                $('#dateBarangSeringTerjual').val(`${dmyyyy(currentDate)} - ${dmyyyy(currentDate.addYear(1))}`)
            });
            break;
        }
        default: break;
    }
}

document.getElementById("date_select").addEventListener("change", async function(e) {
    await loadDate(e.target.selectedIndex)
})

async function refreshBarangKosong() {
    $("#barangKosongTable").DataTable().clear().draw()
    await fetch("/?api_args=11&dashboardType=3", {
        method: "POST"
    }).then(async response => {
        if (response.status == 200) {
            await response.text().then(data => {
                data = data.split("\n");
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

async function refreshBarangTotalTerjual() {
    $("#barangTotalTerjualTable").DataTable().clear().draw()
    var dateSplit = document.getElementById("dateBarangSeringTerjual").value.split(" - ")
    await fetch("/?api_args=11&dashboardType=4", {
        method: "POST",
        headers: {
            startDate: dateSplit[0].replaceAll("/", "_"),
            endDate: dateSplit[1].replaceAll("/", "_")
        }
    }).then(async response => {
        if (response.status == 200) {
            await response.text().then(data => {
                data = data.split("\n");

                for (let a = 0; a < data.length - 1; a++) {
                    var valueSplit = data[a].split("|")
                    $("#barangTotalTerjualTable").DataTable().row.add([valueSplit[0], valueSplit[1]]).draw(false)
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
}
window.onload = async function() {
    await dashboardLoad()
    await refreshBarangKosong()
}