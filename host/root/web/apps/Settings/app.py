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
Robot = imp.load_source('Robot', current_dir + '/Robot.py')
LEFT_TRIM   = 0
RIGHT_TRIM  = 0
robot = Robot.Robot(left_trim=LEFT_TRIM, right_trim=RIGHT_TRIM)



def get_immediate_subdirectories(dir) :
    return [name for name in os.listdir(dir)
            if os.path.isdir(os.path.join(dir, name))]

config = { '/': 
        {
 		'tools.staticdir.on': True,
		'tools.staticdir.dir': current_dir + '/static/',
		'tools.staticdir.index': 'index.html',
        }
}
base = '/robot-control'
name = 'Robot Controller'



class Root():

    def tester(self, name):
        return "TESTdf"
        print "cool"
    tester.exposed = True
  
    def control(self, **data):
        
        ret = ''
        if 'operation' in data :
            cherrypy.response.headers['Content-Type'] = "application/json"
            if data['operation'] == 'move' :
                if data['dir'] == 'fwd' :
                    robot.forward(150, .5)   
                if data['dir'] == 'rev' :
                    robot.reverse(150, .5) 
                if data['dir'] == 'left' :
                    robot.left(150, .5) 
                if data['dir'] == 'right' :
                   robot.right(150, .5) 
            return "cool"  
        else :
            cherrypy.response.headers['Content-Type'] = "application/json"
            return "no operation specified"

    control.exposed = True


