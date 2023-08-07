async function load() {
    await fetch("/pengaturan?pengaturanArgs=1", {
        method: "POST"
    }).then(async response => {
        if (response.status == 200) {
            await response.text().then(data => {
                data = data.split("\n")
                $('#menggunakanTelegramBot').bootstrapToggle(data[0] == '1' ? 'on' : 'off')
                document.getElementById("telegramBotToken").value = data[1]
                document.getElementById("telegramUserID").value = data[2]
            })
        }
    })
}



$('#menggunakanTelegramBot').on('change.bootstrapSwitch', function(e) {
    
    if (e.target.checked) {
        document.getElementById("telegramBotToken").disabled = false
        document.getElementById("telegramUserID").disabled = false
    }
    else {
        document.getElementById("telegramBotToken").disabled = true;
        document.getElementById("telegramUserID").disabled = true;
    }
});

window.onload = async function() {
    await load()
    document.getElementById("telegramBotToken").focus()
}

function terapkanPengaturan() {
    fetch("/pengaturan?pengaturanArgs=2", {
        method: "POST",
        body: `${document.getElementById("menggunakanTelegramBot").checked ? '1' : '0'}\n${document.getElementById("telegramBotToken").value}\n${document.getElementById("telegramUserID").value}`
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