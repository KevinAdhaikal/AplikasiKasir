$("input[data-type='currency']").on({
    input: function() {
        formatCurrency($(this));
    }
});

function formatNumber(n) {
    return n.replace(/\D/g, "").replace(/\B(?=(\d{3})+(?!\d))/g, ".");
}

function convertTime(inputTime) {
    var timeArray = inputTime.split(':');
    var hours = parseInt(timeArray[0], 10);
    var minutes = parseInt(timeArray[1], 10);
    var convertedTime = '';

    if (hours < 12 && inputTime.indexOf('PM') > -1) {
        hours = hours + 12;
    } else if (hours === 12 && inputTime.indexOf('AM') > -1) {
        hours = 0;
    }

    convertedTime = ('0' + hours).slice(-2) + ':' + ('0' + minutes).slice(-2);
    return convertedTime
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

document.getElementById("jumlahNotifyStockBarangDibawahJumlah").addEventListener("keypress", function(e) {
    if (!(e.keyCode >= 47 && e.keyCode <= 58)) e.preventDefault();
})

document.getElementById("jumlahNotifyStockBarangDibawahJumlah").addEventListener("input", function(e) {
    if (Number(e.target.value) > 65535) e.target.value = "65535"
    else if (Number(e.target.value) < 0) e.target.value = "0"
})

function load() {
    fetch("/pengaturan?pengaturanArgs=1", {
        method: "POST"
    }).then(async response => {
        if (response.status == 200) {
            await response.text().then(data => {
                data = data.split("\n")

                $('#menggunakanTelegramBot').bootstrapToggle(data[0] == '1' ? 'on' : 'off')
                document.getElementById("telegramBotToken").value = data[1]
                document.getElementById("telegramUserID").value = data[2]
                $("#blockBarangKosong").bootstrapToggle(data[3] == '1' ? 'on' : 'off')
                $("#notifyBarangKosong").bootstrapToggle(data[4] == '1' ? 'on' : 'off')
                $("#notifyKasir").bootstrapToggle(data[5] == '1' ? 'on' : 'off')
                $("#isNotifyStockBarangDibawahJumlah").bootstrapToggle(data[6] == '1' ? 'on' : 'off')
                document.getElementById("jumlahNotifyStockBarangDibawahJumlah").value = data[7];
                $("#isNotifyAlarmPembukuan").bootstrapToggle(data[8] == '1' ? 'on' : 'off');
                document.getElementById('waktuNotifyAlarmPembukuan').value = data[9];
                $("#isAutoRefreshBarangTotalTerjual").bootstrapToggle(data[10] == '1' ? 'on' : 'off')
                document.getElementById("AutoSetFilterDate_date_select").selectedIndex = Number(data[11])
            })
        }
    })
}

document.addEventListener('DOMContentLoaded', function() {
    // reinitialize yang ada bootstrap toggle
    document.getElementById("telegram-content").classList.add("active");
    $('[data-toggle="toggle"]').bootstrapToggle();
    document.getElementById("telegram-content").classList.remove("active");
    
    load()
    document.getElementById("telegramBotToken").focus()
});

$('#menggunakanTelegramBot').on('change.bootstrapSwitch', function(e) {
    if (e.target.checked) document.getElementById("boxTelegram").classList.remove("d-none")
    else document.getElementById("boxTelegram").classList.add("d-none")
});

$('#isNotifyAlarmPembukuan').on('change.bootstrapSwitch', function(e) {
    if (e.target.checked) {
        document.getElementById("isNotifyAlarmPembukuan_div").classList.remove("d-none")
        document.getElementById("waktuNotifyAlarmPembukuan").disabled = false
    }
    else {
        document.getElementById("isNotifyAlarmPembukuan_div").classList.add("d-none")
        document.getElementById("waktuNotifyAlarmPembukuan").disabled = true
    }
});

$('#isNotifyStockBarangDibawahJumlah').on('change.bootstrapSwitch', function(e) {
    if (e.target.checked) {
        document.getElementById("isNotifyStockBarangDibawahJumlah_div").classList.remove("d-none")
        document.getElementById("jumlahNotifyStockBarangDibawahJumlah").disabled = false
    }
    else {
        document.getElementById("isNotifyStockBarangDibawahJumlah_div").classList.add("d-none")
        document.getElementById("jumlahNotifyStockBarangDibawahJumlah").disabled = true
    }
});

function terapkanPengaturan() {
    fetch("/pengaturan?pengaturanArgs=2", {
        method: "POST",
        body: `${document.getElementById("menggunakanTelegramBot").checked ? '1' : '0'}
${document.getElementById("telegramBotToken").value}
${document.getElementById("telegramUserID").value}
${document.getElementById("blockBarangKosong").checked ? '1' : '0'}
${document.getElementById("notifyBarangKosong").checked ? '1' : '0'}
${document.getElementById("notifyKasir").checked ? '1' : '0'}
${document.getElementById("isNotifyStockBarangDibawahJumlah").checked ? '1' : '0'}
${document.getElementById("jumlahNotifyStockBarangDibawahJumlah").value}
${document.getElementById("isNotifyAlarmPembukuan").checked ? '1' : '0'}
${convertTime(document.getElementById('waktuNotifyAlarmPembukuan').value)}
${document.getElementById("isAutoRefreshBarangTotalTerjual").checked ? '1' : '0'}
${document.getElementById("AutoSetFilterDate_date_select").selectedIndex}`
    }).then(response => {
        if (response.status == 200) {
            Swal.mixin({
                toast: true,
                position: 'top-end',
                showConfirmButton: false,
                timer: 3000,
              }).fire({
                icon: 'success',
                title: "Pengaturan berhasil di terapkan!"
            })
        }
    })
}

function testMessage() {
    fetch("/pengaturan?pengaturanArgs=3", {
        method: "POST",
        headers: {
            "teleToken": document.getElementById("telegramBotToken").value,
            "teleID": document.getElementById("telegramUserID").value
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
                title: "Test Message berhasil di kirimkan ke telegram! Cek pesan di telegram mu"
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