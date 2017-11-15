
var appBaseURL = '/robot-control'
var controlurl = appBaseURL + '/control'

function robotControl(dir){
        console.log('moving')
        $.ajaxSetup({async: false});
        $.get(controlurl+'?operation=move', { 'dir' : dir })
        .done(function () {
            console.log('went fwd');
        })
        .fail(function () {
            console.log('problem');
        });
        $.ajaxSetup({async: true});

}


$(function () {
    $("#control-fwd").click(function(){
	    robotControl('fwd')
    });
    $("#control-right").click(function(){
	    robotControl('right')
    });
    $("#control-left").click(function(){
	    robotControl('left')
    });
    $("#control-rev").click(function(){
	    robotControl('rev')
    });
});
