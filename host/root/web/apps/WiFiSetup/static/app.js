
var appBaseURL = '/wifi'
var controlurl = appBaseURL + '/control'

function wifiControl(op, dat, callback){
        console.log('controlling')
        $.get(controlurl+'?operation='+op, dat)
        .done(function (data) {
            console.log('controlled it');
            callback(data)
        })
        .fail(function () {
            console.log('problem');
        });
}


$(function () {
    $("#list-networks").click(function(){
	    wifiControl('list-networks', '', function(data){
            console.log(data);
        });
    });
});
