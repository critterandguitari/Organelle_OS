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

config = { '/': 
        {
 		'tools.staticdir.on': True,
		'tools.staticdir.dir': current_dir + '/static/',
		'tools.staticdir.index': 'index.html',
        }
}
base = '/wifi'
name = 'WiFi Setup'

class Root():

    def tester(self, name):
        return "TESTdf"
        print "cool"
    tester.exposed = True
  
    def control(self, **data):
        
        ret = ''
        if 'operation' in data :
            cherrypy.response.headers['Content-Type'] = "application/json"
            return '{"ok":"ok"}'
        else :
            cherrypy.response.headers['Content-Type'] = "application/json"
            return '{"error":"no op specified"}'

    control.exposed = True


