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

file_operations = imp.load_source('file_operations', current_dir + '/file_operations.py')

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

    def flash(self):
        os.system("oscsend localhost 4001 /led/flash i 4")
        return "done"
    flash.exposed = True
         
    def fmdata(self, **data):
        ret = ''
        if 'operation' in data :
            cherrypy.response.headers['Content-Type'] = "application/json"

            if data['operation'] == 'get_networks' :
                return file_operations.get_networks()
            if data['operation'] == 'add_network' :
                return file_operations.add_network(data['name'], data['pw'])
            if data['operation'] == 'delete_network' :
                return file_operations.delete_network(data['name'])
            if data['operation'] == 'edit_ap' :
                return file_operations.edit_ap(data['name'], data['pw'])
        else :
            cherrypy.response.headers['Content-Type'] = "application/json"
            return "no operation specified"

    fmdata.exposed = True


