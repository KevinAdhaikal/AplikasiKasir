function logOut() {
    Swal.fire({
        icon: 'warning',
        title: 'Log Out',
        text: "Apakah kamu ingin Log Out dari AplikasiKasir?",
        showDenyButton: true,
        denyButtonColor: '#3085d6',
        confirmButtonColor: '#d33',
        confirmButtonText: 'Iya (Enter)',
        denyButtonText: `Tidak (Esc)`,
      }).then((result) => {
        if (result.isConfirmed) {
            document.cookie = `username=; Max-Age=0`
            document.cookie = `password=; Max-Age=0`
            location.replace("/login.html")
        }
    })
}