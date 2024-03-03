var current_id = [0, 0]

$("input[data-type='currency']").on({
    input: function() {
        formatCurrency($(this));
    }
});

$("#modal-editRolePermission").on("show.bs.modal", function() {
    setTimeout(function() {$('#who_using_this_role_table').DataTable().columns.adjust(); document.getElementById("nama_role_permission").focus()}, 200);  
})

$('a[do_resize="1"]').on('shown.bs.tab', function (e) {
    $('#who_using_this_role_table').DataTable().columns.adjust()
});

document.getElementById("import_database_upload").addEventListener("click", function() {
    var input = document.createElement('input');
    input.type = 'file';
    input.style.display = 'none';

    input.onchange = function (e) {
        var file = e.target.files[0];
        import_database(file);
    };

    document.body.appendChild(input);
    input.click();
})

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

function load_role_permission_table() {
    $("#roles_lists").DataTable().clear().draw(false)
    fetch("/?api_args=17&role_opt=1", {
        method: "POST"
    }).then(async response => {
        if (response.status == 200) {
            await response.text().then(data => {
                console.log(data)
                data = data.split("\x01")
                for (let a = 0; a < data.length; a++) {
                    data[a] = data[a].split("\x02")
                    data[a][data[a].length] = `<center><button type="button" class="btn btn-info" onclick="edit_role('${data[a][0]}', ${data[a][4]})">Edit/Info</button>${!Number(data[a][4]) ? `<button type="button" class="btn btn-danger" onclick="delete_role('${data[a][0]}')">Delete</button></center>`: ""}`
                    data[a].splice(4, 1)
                }
                current_id[1] = Number(data[data.length - 1][0])
                $("#roles_lists").DataTable().rows.add(data).draw(false)
            })
        }
    })
}

function load_user_table() {
    $("#users_list").DataTable().clear().draw(false)
    fetch("/?api_args=16&user_opt=1", {
        method: "POST"
    }).then(async response => {
        if (response.status == 200) {
            await response.text().then(data => {
                data = data.split("\x01")
                for (let a = 0; a < data.length; a++) {
                    data[a] = data[a].split("\x02")
                    data[a][data[a].length] = `<center><button type="button" class="btn btn-info" onclick="show_edit_user('${data[a][0]}', ${data[a][4]})">Edit/Info</button>${data[a][4] == 0 ? `<button type="button" class="btn btn-danger" onclick="delete_user('${data[a][0]}')">Delete</button></center>`: ""}`
                    data[a].splice(4, 1)
                }
                current_id[0] = Number(data[data.length - 1][0])
                $("#users_list").DataTable().rows.add(data).draw(false)
            })
        }
    })
}

function load() {
    fetch("/?api_args=13&pengaturanArgs=1", {
        method: "POST"
    }).then(async response => {
        if (response.status == 200) {
            await response.text().then(data => {
                data = data.split("\x01")
                document.getElementById("menggunakanTelegramBot").checked = Number(data[0])
                menggunakanTelegramBot_onChange(document.getElementById("menggunakanTelegramBot").checked)
                
                document.getElementById("telegramBotToken").value = data[1]
                document.getElementById("telegramUserID").value = data[2]
                document.getElementById("blockBarangKosong").checked = Number(data[3])
                document.getElementById("notifyBarangKosong").checked = Number(data[4])
                document.getElementById("notifyKasir").checked = Number(data[5])
                document.getElementById("isNotifyStockBarangDibawahJumlah").checked = Number(data[6])
                isNotifyStockBarangDibawahJumlah_onChange(document.getElementById("isNotifyStockBarangDibawahJumlah").checked)

                document.getElementById("jumlahNotifyStockBarangDibawahJumlah").value = data[7];
                document.getElementById("isNotifyAlarmPembukuan").checked = Number(data[8])
                isNotifyAlarmPembukuan_onChange(document.getElementById("isNotifyAlarmPembukuan").checked)
                
                document.getElementById('waktuNotifyAlarmPembukuan').value = data[9];
                document.getElementById("isAutoRefreshBarangTotalTerjual").checked = Number(data[10])
                document.getElementById("AutoSetFilterDate_date_select").selectedIndex = Number(data[11])
                document.getElementById("perbedaanInfoHariIniDenganKemarin").checked = Number(data[12])
            })
        }
    })
    load_user_table()
    load_role_permission_table()
}

async function show_create_user_dialog() {
    document.getElementById("title_modal_user").innerText = "Tambah User"
    document.getElementById("edit_user_button").innerText = "Tambahkan User"
    document.getElementById("edit_user_button").onclick = function() {create_user()}
    document.getElementById("role_user").innerHTML = "" // kita kosongin dulu role_user nya

    await fetch("/?api_args=17&role_opt=5", { // nah, ini baru kita nge fetch list role nya dari server
        method: "POST"
    }).then(async response => {
        if (response.status == 200) {
            await response.text().then(async result => {
                result = result.split("\x01");
                for (let a = 0; a < result.length; a++) {
                    var result1 = result[a].split("\x02")
                    document.getElementById("role_user").innerHTML += `<option value=${result1[0]}>${result1[1]}</option>`
                }
            })
        }
    })

    document.getElementById("username_user").value = ""
    document.getElementById("password_user").value = ""
    document.getElementById("role_user").disabled = false
    await $("#modal-editUser").modal("show")
}

function create_user() {
    fetch(`/?api_args=16&user_opt=2&username=${document.getElementById("username_user").value}&password=${document.getElementById("password_user").value}&role_id=${document.getElementById("role_user").value}`, {
        method: "POST"
    }).then(response => {
        if (response.status == 200) {
            load_user_table()
            $("#modal-editUser").modal("hide")
            Swal.mixin({
                toast: true,
                position: 'top-end',
                showConfirmButton: false,
                timer: 3000,
              }).fire({
                icon: 'success',
                title: "User berhasil ditambahkan!"
            })
        }
    })
}

$('#modal-editRolePermission').on('hidden.bs.modal', function () {
    document.getElementById("user-information").classList = ["tab-pane text-left fade active show"]
    document.getElementById("permission-role").classList = ["tab-pane fade"]
    document.getElementById("user-information-tab").classList = ["nav-link active"]
    document.getElementById("permission-role-tab").classList = ["nav-link"]
    $("#who_using_this_role_table").DataTable().clear().draw(false)
})

function tambahRole() {
    document.querySelectorAll('[permission_checkbox="1"]').forEach(function(button) {
        button.disabled = 0;
    });
    document.getElementById("alert_permission").style = "display: none;"
    document.getElementById("who_using_this_role").style = "display: none;"
    document.getElementById("title_modal_permission").innerText = "Tambah Role Permission"
    document.getElementById("nama_role_permission").value = ""
    document.getElementById("edit_role_button").innerText = "Tambah Role Permission"
    document.getElementById("edit_role_button").onclick = function() {process_tambahRole()}
    document.querySelectorAll('input[permission_checkbox]').forEach(function(button) {
        button.disabled = false;
        button.checked = false;
    })
    $("#modal-editRolePermission").modal("show")
}

function process_tambahRole() {
    fetch("/?api_args=17&role_opt=2", {
        method: "POST",
        body: `${document.getElementById("nama_role_permission").value}\x01${Number(document.getElementById("full_access").checked)}`
    }).then(async response => {
        if (response.status == 200) {
            load_role_permission_table()
            Swal.mixin({
                toast: true,
                position: 'top-end',
                showConfirmButton: false,
                timer: 3000,
              }).fire({
                icon: 'success',
                title: "Role berhasil ditambahkan!"
            })
            $("#modal-editRolePermission").modal("hide")
        }
    })
}

async function show_edit_user(id_user) {
    document.getElementById("title_modal_user").innerText = "Edit User"
    document.getElementById("edit_user_button").innerText = "Edit User"
    document.getElementById("edit_user_button").onclick = function() {edit_user(id_user)}
    document.getElementById("role_user").innerHTML = "" // kita kosongin dulu role_user nya

    await fetch("/?api_args=17&role_opt=5", { // nah, ini baru kita nge fetch list role nya dari server
        method: "POST"
    }).then(async response => {
        if (response.status == 200) {
            await response.text().then(async result => {
                result = result.split("\x01");
                for (let a = 0; a < result.length; a++) {
                    var result1 = result[a].split("\x02")
                    document.getElementById("role_user").innerHTML += `<option value=${result1[0]}>${result1[1]}</option>`
                }
            })
        }
    })

    await fetch(`/?api_args=16&user_opt=1&is_view_user=1&user_id=${id_user}`, {
        method: "POST"
    }).then(async response => {
        if (response.status == 200) {
            await response.text().then(async result => {
                result = result.split("\x02")
                document.getElementById("username_user").value = result[0]
                document.getElementById("password_user").value = result[1]
                document.getElementById("role_user").value = result[2]
                document.getElementById("role_user").disabled = Number(result[3])
            })
        }
    })

    await $("#modal-editUser").modal("show")
}

async function edit_user(user_id) {
    await fetch(`/?api_args=16&user_opt=3&user_id=${user_id}&username=${document.getElementById("username_user").value}&password=${document.getElementById("password_user").value}&role_id=${document.getElementById("role_user").value}`, {
        method: "POST"
    }).then(async response => {
        if (response.status == 200) {
            load_user_table()
            Swal.mixin({
                toast: true,
                position: 'top-end',
                showConfirmButton: false,
                timer: 3000,
              }).fire({
                icon: 'success',
                title: "User tersebut berhasil diedit!"
            })
            $("#modal-editUser").modal("hide")
        } else {
            Swal.mixin({
                toast: true,
                position: 'top-end',
                showConfirmButton: false,
                timer: 3000,
              }).fire({
                icon: 'error',
                title: `Gagal mengedit User: ${response.statusText}`
            })
        }
    })
}

function delete_user(user_id) {
    Swal.fire({
        icon: 'warning',
        title: 'Hapus User',
        text: "Apakah kamu yakin untuk menghapus User tersebut?",
        showDenyButton: true,
        denyButtonColor: '#3085d6',
        confirmButtonColor: '#d33',
        confirmButtonText: 'Iya (Enter)',
        denyButtonText: `Tidak (Esc)`,
      }).then((result) => {
        if (result.isConfirmed) {
            fetch(`/?api_args=16&user_opt=4&user_id=${user_id}`, {
                method: "POST"
            }).then(response => {
                if (response.status == 200) {
                    Swal.mixin({
                        toast: true,
                        position: 'top-end',
                        showConfirmButton: false,
                        timer: 3000,
                      }).fire({
                        icon: 'success',
                        title: "User tersebut berhasil dihapus!"
                    })
                    $("#users_list").DataTable().rows(function ( idx, data, node ) { return data[0] == user_id ? true : false;}).remove().draw(false)
                    current_id[0] = $('#users_list').DataTable().row($('#users_list').DataTable().rows().count() - 1).data()[0]
                }
            })
        }
    })
}

async function edit_role(role_id, view_role_name_only) {
    document.getElementById("title_modal_permission").innerText = "Edit Role Permission"
    await fetch(`/?api_args=17&role_opt=1&role_id=${role_id}&role_view_opt=1`, {
        method: "POST"
    }).then(async response => {
        if (response.status == 200) {
            await response.text().then(async result => {
                result = result.split("\x02")
                if (view_role_name_only) {
                    document.getElementById("alert_permission").style = ""
                    document.querySelectorAll('input[permission_checkbox]').forEach(function(button) {
                        button.disabled = 1;
                    });
                } else {
                    document.getElementById("alert_permission").style = "display: none;"
                    document.querySelectorAll('input[permission_checkbox]').forEach(function(button) {
                        button.disabled = 0;
                    });
                }
                await fetch(`/?api_args=17&role_opt=1&role_view_opt=2&role_id=${role_id}`, {
                    method: "POST"
                }).then(async response2 => {
                    await response2.text().then(data2 => {
                        if (data2) {
                            data2 = data2.split("\x01").map(function(item) {
                                return [item];
                            });
                            $("#who_using_this_role_table").DataTable().rows.add(data2).draw(false)
                        }
                    })
                })
                document.getElementById("who_using_this_role").style = ""
                document.getElementById("nama_role_permission").value = result[0]
                document.getElementById("full_access").checked = Number(result[1][0])
                document.getElementById("edit_role_button").innerText = "Edit Role Permission"
                document.getElementById("edit_role_button").onclick = function() {process_edit_role(role_id)}
                $("#modal-editRolePermission").modal("show")
            })
        }
    })
}

async function process_edit_role(role_id) {
    fetch(`/?api_args=17&role_opt=4`, {
        method: "POST",
        body: `${role_id}
${document.getElementById("nama_role_permission").value}\x01${document.getElementById("full_access").checked ? '1' : '0'}`
    }).then(async response => {
        if (response.status == 200) {
            load_role_permission_table()
            Swal.mixin({
                toast: true,
                position: 'top-end',
                showConfirmButton: false,
                timer: 3000,
              }).fire({
                icon: 'success',
                title: "Role tersebut berhasil diedit!"
            })
            $("#modal-editRolePermission").modal("hide")
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

function delete_role(role_id) {
    Swal.fire({
        icon: 'warning',
        title: 'Hapus Role',
        text: "Apakah kamu yakin untuk menghapus Role tersebut?\nPERINGATAN: Ini akan menghapus User yang dipakai Role ini",
        showDenyButton: true,
        denyButtonColor: '#3085d6',
        confirmButtonColor: '#d33',
        confirmButtonText: 'Iya (Enter)',
        denyButtonText: `Tidak (Esc)`,
      }).then((result) => {
        if (result.isConfirmed) {
            fetch(`/?api_args=17&role_opt=3&role_id=${role_id}`, {
                method: "POST"
            }).then(response => {
                if (response.status == 200) {
                    Swal.mixin({
                        toast: true,
                        position: 'top-end',
                        showConfirmButton: false,
                        timer: 3000,
                      }).fire({
                        icon: 'success',
                        title: "Role tersebut berhasil dihapus!"
                    })
                    $("#roles_lists").DataTable().rows(function ( idx, data, node ) { return data[0] == role_id ? true : false;}).remove().draw(false)
                    current_id[1] = $('#roles_lists').DataTable().row($('#roles_lists').DataTable().rows().count() - 1).data()[0]
                }
            })
        }
    })
}


function backup_database() {
    const today = new Date();

    const date = today.getDate();
    const month = today.getMonth() + 1;
    const year = today.getFullYear();

    fetch("/?api_args=14", {
        method: "POST"
    }).then(response => response.arrayBuffer()).then(data => {
        const blob = new Blob([data], { type: 'application/octet-stream' });
        const url = window.URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = `backup_${date < 10 ? '0' + date : date}_${month < 10 ? '0' + month : month}_${year}.ksr`;
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        window.URL.revokeObjectURL(url);
    })
}

function import_database(file) {
    var reader = new FileReader();
    reader.readAsArrayBuffer(file);

    reader.onload = function() {
        fetch("/?api_args=15", {
            method: "POST",
            body: reader.result
        }).then(response => {
            if (response.status == 200) return Swal.fire({
                icon: 'success',
                title: 'Import Database',
                text: `Mengimport Database Berhasil!`,
                confirmButtonText: "Okay"
            }).then(result => location.replace("/login"))
            else return Swal.fire({
                icon: 'error',
                title: 'Import Database',
                text: `Gagal mengimport Database: ${response.statusText}`,
                confirmButtonText: "Okay"
            })
        })
    }
}

$('a[using_table="true"]').on('shown.bs.tab', function (e) {
    DataTable.tables({ visible: true, api: true }).columns.adjust();
})

document.addEventListener('DOMContentLoaded', function() {
    new DataTable('table.table', {
        scrollCollapse: true,
        scrollX: true,
        scrollY: 500
    })
    load()
});

function menggunakanTelegramBot_onChange(checked) {
    checked ? document.getElementById("boxTelegram").classList.remove("d-none") : document.getElementById("boxTelegram").classList.add("d-none")
}

function isNotifyAlarmPembukuan_onChange(checked) {
    if (checked) {
        document.getElementById("isNotifyAlarmPembukuan_div").classList.remove("d-none")
        document.getElementById("waktuNotifyAlarmPembukuan").disabled = false
    }
    else {
        document.getElementById("isNotifyAlarmPembukuan_div").classList.add("d-none")
        document.getElementById("waktuNotifyAlarmPembukuan").disabled = true
    }
}

function isNotifyStockBarangDibawahJumlah_onChange(checked) {
    if (checked) {
        document.getElementById("isNotifyStockBarangDibawahJumlah_div").classList.remove("d-none")
        document.getElementById("jumlahNotifyStockBarangDibawahJumlah").disabled = false
    }
    else {
        document.getElementById("isNotifyStockBarangDibawahJumlah_div").classList.add("d-none")
        document.getElementById("jumlahNotifyStockBarangDibawahJumlah").disabled = true
    }
}

function terapkanPengaturan() {
    fetch("/?api_args=13&pengaturanArgs=2", {
        method: "POST",
        body: `${document.getElementById("menggunakanTelegramBot").checked ? '1' : '0'}\x01${document.getElementById("telegramBotToken").value}\x01${document.getElementById("telegramUserID").value}\x01${document.getElementById("blockBarangKosong").checked ? '1' : '0'}\x01${document.getElementById("notifyBarangKosong").checked ? '1' : '0'}\x01${document.getElementById("notifyKasir").checked ? '1' : '0'}\x01${document.getElementById("isNotifyStockBarangDibawahJumlah").checked ? '1' : '0'}\x01${document.getElementById("jumlahNotifyStockBarangDibawahJumlah").value}\x01${document.getElementById("isNotifyAlarmPembukuan").checked ? '1' : '0'}\x01${convertTime(document.getElementById('waktuNotifyAlarmPembukuan').value)}\x01${document.getElementById("isAutoRefreshBarangTotalTerjual").checked ? '1' : '0'}\x01${document.getElementById("AutoSetFilterDate_date_select").selectedIndex}\x01${document.getElementById("perbedaanInfoHariIniDenganKemarin").checked ? '1' : '0'}`
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

function testMessage() {
    fetch("/?api_args=13&pengaturanArgs=3", {
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