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

document.getElementById("dark_mode_checkbox").addEventListener("change", function() {
    if (this.checked) {
        document.cookie = "dark_mode=1;"
        document.getElementById("swal_css").href = "plugins/sweetalert2-dark/dark.min.css"
        if (document.getElementById("navbarMain")) {
          document.getElementById("navbarMain").classList.add("navbar-dark")
          document.getElementById("navbarMain").classList.remove("navbar-light")
        }
        document.getElementById("bodyMain").classList.add("dark-mode")
    }
    else {
        document.cookie = "dark_mode=; Max-Age=0"
        document.getElementById("swal_css").href = "plugins/sweetalert2/sweetalert2.min.css"
        if (document.getElementById("navbarMain")) {
          document.getElementById("navbarMain").classList.remove("navbar-dark")
          document.getElementById("navbarMain").classList.add("navbar-light")
        }
        document.getElementById("bodyMain").classList.remove("dark-mode")
    }
})

if (getCookie("dark_mode")) {
    document.getElementById("swal_css").href = "plugins/sweetalert2-dark/dark.min.css"
    document.getElementById("dark_mode_checkbox").checked = true;
    if (document.getElementById("navbarMain")) {
      document.getElementById("navbarMain").classList.add("navbar-dark")
      document.getElementById("navbarMain").classList.remove("navbar-light")
    }
    document.getElementById("bodyMain").classList.add("dark-mode")
} else {
    document.getElementById("swal_css").href = "plugins/sweetalert2/sweetalert2.min.css"
    document.getElementById("dark_mode_checkbox").checked = false;
    if (document.getElementById("navbarMain")) {
      document.getElementById("navbarMain").classList.remove("navbar-dark")
      document.getElementById("navbarMain").classList.add("navbar-light")
    }
    document.getElementById("bodyMain").classList.remove("dark-mode")
}