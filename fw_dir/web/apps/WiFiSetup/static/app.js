
var appBaseURL = '/wifi'
var fsurl = appBaseURL + '/fmdata'
var toDelete = ''

function alertDialog(msg){
    $('#info-modal').modal({backdrop: false});
    $('#info-modal-msg').empty();   
    $('#info-modal-msg').append('<p>'+msg+'</p>');   
}

function showNetworks(d){
        $("#wifi-table").empty();
        d.forEach(function (net) { 
            tr = $('<tr class="wifi-row"><td class="col-xs-3">'+net.name+'</td><td class="col-xs-1"><button type="button" class="btn btn-danger del-but">Delete</button> </td></tr>'); 
	        tr.data("name", net.name);
            $("#wifi-table").append(tr);
        });

    $(".del-but").click(function(){
        console.log('del');
        $('#del-modal').modal({backdrop: false});
        $('#del-network').empty();
        toDelete = $(this).closest('tr').data("name");
        $('#del-network').append(toDelete);
        
    });
}

function refresh(){
    $.get(fsurl+'?operation=get_networks', {})
    .done(function (d) {
        showNetworks(d);
    }).fail(function () {
        console.log('oops');
    });    
}

$(document).ajaxStart(function(){
	$.LoadingOverlay("show", {
    	fade  : [50, 50],
        color : "rgba(255, 255, 255, 0)",
        //image : "./assets/spinner.gif"
	});
});
$(document).ajaxStop(function(){
    $.LoadingOverlay("hide");
});

$(function () {

    // button actions
    $("#resync-but").click(function(){
        $.get(appBaseURL+'/resync')
        .done(function (d) {
            console.log('resync done');
        })
        .fail(function () {
            console.log('problem with resync');
        });
    });

    $("#flash-but").click(function(){
        $.get(appBaseURL+'/flash')
        .done(function (d) {
            console.log('flashed');
        })
        .fail(function () {
            console.log('problem with flash');
        });
    });
    
    $("#new-net-but").click(function(){
            $('#new-net-modal').modal({backdrop: false});
    });
 
    $("#ap-net-but").click(function(){
            $('#edit-ap-modal').modal({backdrop: false});
    });

    $("#confirm-edit-ap").click(function(){
        $('#edit-ap-modal').modal('hide');
        $.get(fsurl+'?operation=edit_ap', {  'name' : $('#ap-ssid').val(), 'pw' : $('#ap-pw').val()  })
        .done(function () {
            console.log('added');
            refresh();
        })
        .fail(function () {
            console.log('problem adding');
        });
    });

    $("#confirm-add").click(function(){
        $('#new-net-modal').modal('hide');
        $.get(fsurl+'?operation=add_network', {  'name' : $('#new-ssid').val(), 'pw' : $('#new-pw').val()  })
        .done(function () {
            console.log('added');
            refresh();
        })
        .fail(function () {
            console.log('problem adding');
        });
    });

    $("#confirm-delete").click(function(){
        $('#del-modal').modal('hide');
        $.get(fsurl+'?operation=delete_network', { 'name' : toDelete })
        .done(function () {
            console.log('deleted 1');
    		refresh();
        })
        .fail(function () {
            console.log('problem deleting');
        });
    });

    refresh();
});


