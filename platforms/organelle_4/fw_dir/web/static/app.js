var appBaseURL = 'http://' + location.host
var fsurl = appBaseURL + '/fmdata'
//var workingDir = '/sdcard/';
var workingDir = '/';
var clipboard = {};

var editor = null
var openFiles = []; // Array to hold open files and their editor sessions
var currentFile = null; // Currently active file

function getFile(fpath) {
    $.get(appBaseURL + '/get_file?fpath='+encodeURIComponent(fpath), function(data) {
        editor.setValue(data)
        editor.gotoLine(1)
        $("#title").html(fpath)
    }, 'text');  // force respose to be treated as text
}

function getAceMode(extension) {
    if (extension === "py") return "python";
    if (extension === "lua") return "lua";
    if (extension === "ck") return "chuck";
    // Add more mappings as needed
    return "text";
}

function openFile(path) {
    // Check if the file is already open
    var existingFile = openFiles.find(file => file.path === path);
    if (existingFile) {
        // Switch to this tab
        switchTab(path);
        return;
    }

    // Determine the file extension
    var extension = path.split('.').pop().toLowerCase();

    // List of allowed text-based extensions
    var textExtensions = ['pd', 'ck', 'txt', 'md', 'html', 'css', 'js', 'json', 'xml', 'csv', 'log', 'py', 'c', 'cpp', 'java', 'sh'];

    // List of image extensions
    var imageExtensions = ['jpg', 'jpeg', 'png', 'gif', 'bmp', 'svg'];

    if (imageExtensions.includes(extension)) {
        // For image files, add a tab that displays the image
        addImageTab(path);
    } else if (textExtensions.includes(extension)) {
        // Fetch and display text-based files
        $.get(appBaseURL + '/get_file?fpath=' + encodeURIComponent(path), function(data) {
            var fileName = path.split('/').pop();
            addTab(path, fileName, data);
        }, 'text');
    } else {
        // Ignore unsupported file types
        //alert("This file type cannot be displayed.");
    }
}

function addTab(path, fileName, content) {
    // Split the path into parts
    var pathParts = path.split('/');

    // Get the file name (last part of the path)
    var fileName = pathParts.pop();

    // Get the parent directory (one level up)
    var parentDir = pathParts.length > 0 ? pathParts.pop() : '';

    // Create the tab label with parent directory and file name
    // actually use full path to differentiate usb from sd
    var tabLabel = path//parentDir ? parentDir + '/' + fileName : fileName;

    // Create a new tab element with the updated label
    var $tab = $('<div class="tab"></div>').text(tabLabel);
    $tab.attr('data-file-path', path);

    // Add a close button to the tab
    var $closeButton = $('<span class="close-tab">&times;</span>');
    $tab.append($closeButton);

    // Event listeners remain the same...
    $tab.on('click', function() {
        switchTab(path);
    });

    $closeButton.on('click', function(e) {
        e.stopPropagation();
        closeTab(path);
    });

    // Append the tab to the tabs container
    $('#tabs-container').append($tab);

    // Create a new editor session for this file
    var extension = path.split('.').pop();
    var session = ace.createEditSession(content, 'ace/mode/' + getAceMode(extension));

    // Add the file to the openFiles array
    var fileObj = {
        path: path,
        name: fileName,
        content: content,
        tabElement: $tab,
        editorSession: session,
        unsaved: false
    };
    openFiles.push(fileObj);

    // Switch to the new tab
    switchTab(path);
}

function addImageTab(path) {
    // Create a new tab element
    var fileName = path.split('/').pop();

    // Create the tab label
    var $tab = $('<div class="tab"></div>').text(fileName);
    $tab.attr('data-file-path', path);

    // Add a close button to the tab
    var $closeButton = $('<span class="close-tab">&times;</span>');
    $tab.append($closeButton);

    // Event listeners for tab click and close
    $tab.on('click', function() {
        switchTab(path);
    });

    $closeButton.on('click', function(e) {
        e.stopPropagation();
        closeTab(path);
    });

    // Append the tab to the tabs container
    $('#tabs-container').append($tab);

    // Create an object representing the image file
    var fileObj = {
        path: path,
        name: fileName,
        tabElement: $tab,
        isImage: true
    };
    openFiles.push(fileObj);

    // Switch to the new tab
    switchTab(path);
}

function switchTab(path) {
    // Update the currentFile to the new file
    currentFile = openFiles.find(file => file.path === path);

    // Update active tab styling
    $('.tab').removeClass('active');
    currentFile.tabElement.addClass('active');

    // Update the title
    $("#title").html(path);

    // Clear the editor or image container
    $('#editor').hide();
    $('#image-container').hide();

    if (currentFile.isImage) {
        // Display the image
        $('#image-container').empty(); // Clear any previous content

        // Create the image element
        var img = $('<img>').attr('src', appBaseURL + '/get_file?fpath=' + encodeURIComponent(currentFile.path));
        $('#image-container').append(img);

        $('#image-container').show();
    } else {
        // Display the editor with the file's session
        editor.setSession(currentFile.editorSession);
        editor.focus();
        $('#editor').show();
    }
}

function closeTab(path) {
    var index = openFiles.findIndex(file => file.path === path);
    if (index !== -1) {
        var fileToClose = openFiles[index];

        // If there are unsaved changes, prompt the user
        if (fileToClose.unsaved) {
            // Use custom modal instead of confirm()
            closeFileConfirmDialog(path);
            return; // Exit the function; actual closing will happen after confirmation
        }

        // Proceed to actually close the tab
        closeTabConfirmed(path);
    }
}

function closeTabConfirmed(path) {
    var index = openFiles.findIndex(file => file.path === path);
    if (index !== -1) {
        var fileToClose = openFiles[index];

        // Remove tab element
        fileToClose.tabElement.remove();

        // Remove from openFiles array
        openFiles.splice(index, 1);

        // If the closed tab was active, switch to another tab
        if (currentFile && currentFile.path === path) {
            if (openFiles.length > 0) {
                switchTab(openFiles[openFiles.length - 1].path);
            } else {
                currentFile = null;
                editor.setValue('');
                $('#editor').hide();
                $('#image-container').hide();
                $("#title").html('...');
            }
        }
    }
}

function editorSetSyntax(extension) {
    editor.getSession().setMode("ace/mode/" + getAceMode(extension));
}

function refreshWorkingDir(){
    $.get(fsurl+'?operation=get_node', { 'path' : workingDir})
    .done(function (d) {
	if (typeof d === 'string') {d = JSON.parse(d);}
        renderFilesTable(d);
        renderBreadcrumb();
    })
    .fail(function () {
        console.log('problem refreshing');
    });
}

function getWorkingDir() {
    console.log("current dir:" + workingDir);
    return workingDir;
}

function getSelectedNodes(){
    var selectedNodes = [];
    $(".checkbox > input").each(function(){
        if ($(this).is(":checked")) {
            var node = {
                'path' : $(this).closest('tr').data('path'),
                'type' : $(this).closest('tr').data('type'),
            }
            selectedNodes.push(node);
        }
    });
    return selectedNodes
}

function selectedIsOneFile(){
    var selectedNodes = getSelectedNodes();
    console.log(selectedNodes);
    if (selectedNodes.length == 1 && selectedNodes[0].type == 'file') return true;
    else return false;
}

function nodeNameWithIcon(path, type){
    var basename = path.split('/').pop();

    console.log(type);
    if (type == "file"){
        var extension = basename.split('.').pop();
        var img = '';
        if (extension == 'pd') img = "/static/assets/pd.png";
        else if (extension == 'wav') img = "/static/assets/wav.png";
        else img = "/static/assets/txt.png";
    } else {
        img = "/static/assets/folder.png";
    }
    return $('<div class="fname-icon"><img src="'+img+'" width=20/></div><div class="fname-name">' + basename + '</div><div style="clear:both;"/>');
}

function renderFilesTable(d){
    $("#ftable").empty();
    var path = '';

    d.forEach(function(c){
        var basename = c.path.split('/').pop();
        var sizeType = 'Folder'  // display size or Folder for folder
        if (c.type == 'folder'){
            sizeType = 'Folder'
            var trow = $('<tr class="fsdir">');
            var tdata = $('<td class="fsdirname"></td>');
            tdata.append(nodeNameWithIcon(c.path, c.type));
        } else {
            sizeType = c.size;
            var trow = $('<tr class="fsfile">');
            var tdata = $('<td class="fsfilename">');
                        
            var dlButton = $('<div class="dl-but"><span class="material-icons download-icon">download</span></div>');
            dlButton.on("click", function(event) {
                event.preventDefault(); // Prevent the default behavior

                let fileUrl = appBaseURL + "/download?fpath=" + encodeURIComponent(c.path) + "&cb=cool";

                // Create a temporary hidden anchor element
                let a = document.createElement("a");
                a.href = fileUrl;
                a.download = ""; // Let the server define the filename
                document.body.appendChild(a);
                a.click();
                document.body.removeChild(a);
            });

            tdata.append(dlButton);
            tdata.append(nodeNameWithIcon(c.path, c.type));
        }
        trow.data("path", c.path);
        trow.data("type", c.type);
        var checkbox = $('<td><div class="checkbox ff-select"><input type="checkbox" value=""></div></td>');
        trow.append(checkbox);
        trow.append(tdata);
        //trow.append('<td>'+sizeType+'</td>');
        $("#ftable").append(trow);
    });
    window.scrollTo(0,0);
}

function renderBreadcrumb () {
    $("#fsbreadcrumb").empty();
    var absPath = '';
    var path = workingDir.split('/');
    var count = 0;
    var breadelement = $('<li class="fsdir">&nbsp; / </li>');
    breadelement.data("path", "/");
    $("#fsbreadcrumb").append(breadelement);
    path.forEach(function(p) {
        if (p) {
            absPath +=  p + '/';
            var breadelement = $('<li class="fsdir">' + p + '/</li>');
            count++;
            breadelement.data("path", absPath);
            $("#fsbreadcrumb").append(breadelement);
        }
    });
}

// init the modal dialog with title
function newModal(title){
    $('#modal-dialog-contents').empty();
    $('#modal-dialog-contents').append('<div id="modal-dialog-title">'+title+'</div>');
    $('#modal-dialog-contents').append('<div id="modal-dialog-body"></div>');
}

// add to the modal body
function addModalBody(stuff){
    $('#modal-dialog-body').append(stuff);
}

// add button to the modal
function addModalButton(name, callback){
    button = $('<div id="modal-button-'+name+'"class="modal-button">'+name+'</div>').click(callback);
    $('#modal-dialog-contents').append(button);
}

function showModal(){
    // for the buttons floating left
    $('#modal-dialog-contents').append('<div style="clear:both"></div>');
    $('body').addClass("dialog");
}

function hideModal(){
    $('body').removeClass("dialog");
}

function alertDialog(msg){
    newModal('Atención');
    addModalBody('<p>'+msg+'</p>');
    addModalButton('Cancel', hideModal);
    showModal();
}

function pasteCopyDialog(){
    newModal('Copy');
    addModalBody('<p>Copy files: </p>');   
    clipboard.nodes.forEach(function(n) {
        addModalBody(nodeNameWithIcon(n.path,n.type));   
    });       
    addModalBody('<p>to current folder?</p>');   
    addModalButton('Cancel', hideModal)
    addModalButton('Paste', function(){
        hideModal();
        clipboard.nodes.forEach(function(n) {
            $.get(fsurl+'?operation=copy_node', { 'src' : n.path, 'dst' : workingDir })
            .done(function () {
                console.log('copied 1');
                refreshWorkingDir();
            })
            .fail(function () {
                console.log('problem copying');
            });
        });
        clipboard = {};
    });
    showModal();
}

function pasteMoveDialog(){
    newModal('Move');
    addModalBody('<p>Move files: </p>');   
    clipboard.nodes.forEach(function(n) {
        addModalBody(nodeNameWithIcon(n.path,n.type));  
    });       
    addModalBody('<p>to current folder?</p>');  
    addModalButton('Cancel', hideModal);
    addModalButton('Move',  function(){
        hideModal();
        clipboard.nodes.forEach(function(n) {
            $.get(fsurl+'?operation=move_node', { 'src' : n.path, 'dst' : workingDir })
            .done(function () {
                console.log('moved 1');
                refreshWorkingDir();
            })
            .fail(function () {
                console.log('problem moving');
            });
        });
        clipboard = {};
    });
    showModal();
}

function closeFileConfirmDialog(path){
    newModal('Close?');
    addModalBody('This file has unsaved changes. Close anyway?');   
    addModalButton('Cancel', hideModal);
    addModalButton('Yes', function() {
        hideModal();
        closeTabConfirmed(path);
    });
    showModal();
}

function deleteDialog(){
    var selectedNodes = getSelectedNodes(); 
    
    if (selectedNodes.length > 0) {
        newModal('Delete');
        addModalBody('<p>Permanentamentally remove these files?</p>');
        
        selectedNodes.forEach(function(n) {
            addModalBody(nodeNameWithIcon(n.path,n.type));   
        });

        addModalButton('Cancel', hideModal);
        addModalButton('Delete', function(){
            hideModal();
            var selectedNodes = getSelectedNodes();
            selectedNodes.forEach(function(n) {
                $.get(fsurl+'?operation=delete_node', { 'path' : n.path })
                .done(function () {
                    console.log('deleted 1');
                    refreshWorkingDir();
                })
                .fail(function () {
                    console.log('problem deleting');
                });
            });
        });
        showModal();
    }
    else alertDialog("Choose one or more files to delete.");
}

function zipDialog(){
    var selectedNodes = getSelectedNodes();
    var gotaZip = false;
    if (selectedNodes.length == 1) {
        var path = selectedNodes[0].path;
        var basename = path.split('/').pop();
        if (selectedNodes[0].type == 'folder') {
            gotaZip = true;
            newModal('Zip Folder');
            addModalBody('<p>Zip <b>'+basename+'?</b></p>');   
            addModalButton('Cancel', hideModal)
            addModalButton('Zip', function(){
                hideModal();
                var selectedNodes = getSelectedNodes();
                n = selectedNodes[0];
                $.get(fsurl+'?operation=zip_node', { 'path' : n.path })
                .done(function () {
                    console.log('zipped 1');
                    refreshWorkingDir();
                })
                .fail(function () {
                    console.log('problem zipping');
                });
            });
            showModal();
        }
    } 
    if (!gotaZip) alertDialog('<p>Choose one folder to zip.</p>');   
}

function unzipDialog(){
    var selectedNodes = getSelectedNodes();
    var gotaZip = false;
    if (selectedNodes.length == 1) {
        var path = selectedNodes[0].path;
        var basename = path.split('/').pop();
        var extension = basename.split('.').pop();

        if (extension == 'zip') {
            gotaZip = true;
            newModal('Unzip');
            addModalBody('<p>Unzip <b>'+basename+'</b> into current folder?</p>');   
            addModalButton('Cancel', hideModal)
            addModalButton('Unzip', function(){
                hideModal();
                var selectedNodes = getSelectedNodes();
                n = selectedNodes[0];
                $.get(fsurl+'?operation=unzip_node', { 'path' : n.path })
                .done(function () {
                    console.log('unzipped 1 going to refresh');
                    refreshWorkingDir();
                })
                .fail(function () {
                    console.log('problem unzipping');
                });
            });
            showModal();
        }
    } 
    if (!gotaZip) alertDialog('<p>Choose one .zip file to unzip.</p>');   
}

function renameDialog() {
    var selectedNodes = getSelectedNodes();
    if (selectedNodes.length == 1) {
        var path = selectedNodes[0].path;
        var basename = path.split('/').pop();
        newModal('Rename');
        addModalBody('<input type="text" id="rename-node" value="'+basename+'"></input>');
        addModalButton('Cancel', hideModal);
        addModalButton('Rename', function(){
            hideModal();
            var selectedNodes = getSelectedNodes();
            n = selectedNodes[0];
            $.get(fsurl+'?operation=rename_node', { 'path' : n.path, 'name' : $('#rename-node').val() })
            .done(function () {
                console.log('renamed 1');
                refreshWorkingDir();
            })
            .fail(function () {
                console.log('problem moving');
            });
            clipboard = {};
        });
        showModal();
    } 
    else alertDialog('<p>Choose one item to rename.</p>');  
}

function newFolderDialog() {
    newModal('New Folder');
    addModalBody('<input type="text" id="new-folder-name" value="Untitled"></input>');
    addModalButton('Cancel', hideModal);
    addModalButton('New Folder', function(){
        hideModal();
        $.get(fsurl+'?operation=create_node', { 'path' : workingDir, 'name' : $('#new-folder-name').val() })
        .done(function () {
            console.log('created 1');
        	refreshWorkingDir();
        })
        .fail(function () {
            console.log('problem creating folder');
        });
    });
    showModal();
}

function openFileDialog(path) {
    var extension=path.split(".").pop();
    newModal('Open File');
    addModalBody('Are you sure you want to open this file? Unsaved changes to the current file will be lost!');
    addModalButton('Cancel', hideModal);
    addModalButton('Open', function () {
        hideModal();
        console.log('going to get file: ' + path);
        editorSetSyntax(extension);
        getFile(path);
    });
    showModal();
}


function saveMode() {
    if (currentFile) {
        var content = editor.getValue();
        $.post(appBaseURL + '/save', {
            fpath: currentFile.path,
            content: content
        }, function(response) {
            // Handle response, show message, etc.
            currentFile.unsaved = false;
            currentFile.tabElement.removeClass('unsaved');
            console.log('File saved successfully.');
        });
    } else {
        alertDialog('No file to save.');
    }
}

function refreshPatchlist() {
  $.post(appBaseURL + "/refresh_patchlist")
  .done(function(data) {
    console.log(data);
  });
}

$(function () {
 
    // this disables page while loading things 
    $(document).ajaxStart (function() { 
            $('body').addClass("loading");
            console.log("ajax start")
    });
        // When ajaxStop is fired, rmeove 'loading' from body class
    $(document).ajaxStop (function() { 
            $('body').removeClass("loading"); 
            console.log("ajax stop");        
    });
        
    editor = ace.edit("editor");
    editor.setTheme("ace/theme/merbivore_soft");
    editor.getSession().setMode("ace/mode/python");
    document.getElementById('editor').style.fontSize='14px';

    // Mark current file as unsaved when editor content changes
    editor.on('change', function() {
        if (currentFile) {
            currentFile.unsaved = true;
            // Optionally, update the tab to indicate unsaved changes
            currentFile.tabElement.addClass('unsaved');
        }
    });

	    // Keyboard shortcuts for saving
    $(".ace_text-input").keydown(function(e) {
        // Save & reload on Cmd + P
        /*if (e.metaKey && e.which === 80) {
            e.preventDefault();
            saveMode();
            reloadMode(); // Ensure you have a reloadMode() function
        }*/

        // Save on Cmd + S
        if (e.metaKey && e.which === 83) {
            e.preventDefault();
            saveMode();
        }
    });

    $('#fileupload').fileupload({
		// DISABLE drag and drop uploading
       	dropZone: null,  
		url: appBaseURL + '/upload',
        dataType: 'json',
        formData: function() {
            return [{'name':'dst', 'value':getWorkingDir()}];
        },
        done: function (e, data) {
            $.each(data.result.files, function (index, file) {
                //$('<p/>').text(file.name).appendTo('#files');
            });
        },
        progressall: function (e, data) {
            var progress = parseInt(data.loaded / data.total * 100, 10);
            $('#progress-bar-progress').css(
                'width',
                progress + '%'
            );
        }
    }).prop('disabled', !$.support.fileInput)
    .parent().addClass($.support.fileInput ? undefined : 'disabled');
    $('#fileupload').bind('fileuploadstart', function (e) {
        newModal('Uploading...');
        addModalBody('<div id="progress-bar"><div id="progress-bar-progress"></div></div>');
        showModal();
    });
    $('#fileupload').bind('fileuploadstop', function (e, data) {
        hideModal();
        refreshWorkingDir()
        console.log(data);
    });

	$(document).bind('drop dragover', function (e) {
		e.preventDefault();
	});

    $("#show-settings").click(function(){
	if ( $("#settings-container").is(":hidden") ){
    	    $("#editor-container").hide();
    	    $("#settings-container").show();
            $("#title").html("EYESY Settings")
            $("#show-settings").html("Editor")
	} else {
    	    $("#editor-container").show();
    	    $("#settings-container").hide();
            $("#title").html(currentFile.path)
            $("#show-settings").html("Settings")
	}
    });

    $("#wifi-save-ap").click(function() {	 
	$.post(appBaseURL + "/wifi_save_ap", { name: $('#wifi-ap-name').val(), pw: $('#wifi-ap-pw').val() })
	.done(function(data) {
            console.log(data);
	});
    });

    $("#wifi-save-net").click(function() {	 
	$.post(appBaseURL + "/wifi_save_net", { name: $('#wifi-net-name').val(), pw: $('#wifi-net-pw').val() })
	.done(function(data) {
	    $.get(appBaseURL + '/wifi_get_net', function(data) {
		ap = JSON.parse(data);
		$('#wifi-net-name').val(ap.name)
		$('#wifi-net-pw').val(ap.pw)
	    })
            console.log(data);
	});
    });

    $("#compvid-save-format").click(function() {	 
	var fmt = 'ntsc'
    	if ($('input:radio[name=compvid]')[1].checked) { fmt = 'pal' }
	$.post(appBaseURL + "/compvid_save_format", { val: fmt })
	.done(function(data) {
            console.log(data);
	});
    });

    $("#start-video").click(function(){
        $.get(appBaseURL + '/start_video_engine', function(data) {
            console.log(data);
        });
    });

    $("#stop-video").click(function(){
        $.get(appBaseURL + '/stop_video_engine', function(data) {
            console.log(data);
        });
    });

    $("#refresh-patchlist").click(refreshPatchlist);

    $("#save").click(saveMode);

    $("#new-folder-but").click(newFolderDialog);

    $("#rename-but").click(renameDialog);

    $("#copy-but").click(function(){
        clipboard.operation = "copy";
        clipboard.nodes = getSelectedNodes();
        console.log(clipboard);
    });

    $("#cut-but").click(function(){
        clipboard.operation = "cut";
        clipboard.nodes = getSelectedNodes();
        console.log(clipboard);
    });

    $("#paste-but").click(function(){
        if (clipboard.nodes && clipboard.nodes.length > 0 ){
            if (clipboard.operation == "copy") pasteCopyDialog();
            else if (clipboard.operation == "cut") pasteMoveDialog();
        }
        else alertDialog('<p>Choose files then select Copy or Cut to move.</p>');   
    });
   
    $("#delete-but").click(deleteDialog);

    $("#zip-but").click(zipDialog);

    $("#unzip-but").click(unzipDialog);

    // Clear console when trash icon is clicked
    $('#clear-console').click(function() {
        $('#vconsole').empty();
    });

    // click on directory table row, excluding input elements
    $('body').on('click', '.fsdir', function(event) {
        var target = $(event.target);
        if (!target.is("input")) {
            workingDir = $(this).data("path");
            refreshWorkingDir();
        }
    });

    // Click handler for file items
    $('body').on('click', '.fsfile', function(event) {
        var target = $(event.target);
        if (!target.is("input") && !target.is("a") && !target.is("span")) {
            var path = $(this).data("path");
            openFile(path);
        }
    });

    $.get(fsurl+'?operation=get_node', { 'path' : workingDir})
    .done(function (d) {
	if (typeof d === 'string') {d = JSON.parse(d);}
	console.log('data:')
	console.log(d)
        renderFilesTable(d);
        renderBreadcrumb();
    })
    .fail(function () {
        console.log('oops');
    });    
    

});


