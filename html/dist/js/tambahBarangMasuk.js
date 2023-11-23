document.getElementById("namaBarcodeBarang").focus()



$(document).keydown(function(event) { 
    if (event.keyCode == 27) { 
      $('#modal-findBarang').modal("hide");
    }
});