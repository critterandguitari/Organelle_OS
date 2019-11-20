import os.path
import time
import glob
import json
import cherrypy
import urllib
import time
import socket
from cherrypy.lib import static
import imp

current_dir = os.path.dirname(os.path.abspath(__file__))

info = imp.load_source('info', current_dir + '/info.py')

config = { '/': 
        {
        }
}
base = '/info'
name = 'Info'

class Root(object):

    def flash(self):
        os.system("oscsend localhost 4001 /led/flash i 4")
        return "done"
    flash.exposed = True

    @cherrypy.expose
    def index(self):
        info.get_info()
        stuff = ''
        for item in info.items.values() :
            stuff += '<b>' + item[0] +': </b>' + item[1] + '</br></br>'
        return """
<html>
<head>
<title>Organelle Info</title>
<link rel="stylesheet" href="/static/bootstrap.min.css">
<link rel="shortcut icon" href="/favicon.ico" type="image/x-icon" />
</head>
<body style="margin:20px;">

<span style="float:right;font-size: 1.5em; padding-top: .3em;"> 
    <a id="home-but" href="/"><span class="glyphicon glyphicon-home"></span></a>
</span>

<div style="border:1px solid; border-radius: 6px; padding: 16px; width: 500px; margin:16px auto;">
<h3>Info</h3>
</br>
<div>
""" + stuff + """
</div>
</br>
</body>
</html>
"""
