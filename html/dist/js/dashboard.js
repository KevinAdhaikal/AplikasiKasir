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

async function dashboardLoad() {
    await fetch("/dashboardLogic?dashboardType=1", {
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
    await fetch("/dashboardLogic?dashboardType=2", {
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
    await fetch("/pengaturan?pengaturanArgs=4", {
        method: "POST"
    }).then(async response => {
        if (response.status == 200) {
            await response.text().then(async data => {
                data = data.split("\n")
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
    await fetch("/dashboardLogic?dashboardType=3", {
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
    await fetch("/dashboardLogic?dashboardType=4", {
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
    $("#pendapatanID").text("Rp" + Intl.NumberFormat('id', {}).format(Number(Number($("#pendapatanHargaJualID").text().slice(2).replaceAll(".", "")) - Number($("#pengeluaranID").text().slice(2).replaceAll(".", "")))))
    await refreshBarangKosong()
}