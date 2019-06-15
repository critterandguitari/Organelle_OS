import json
import os
import shutil
import cherrypy

BASE_DIR = "/"

user_dir = os.getenv("USER_DIR", "/sdcard")

def add_network(name, pw):
    wifi_file = user_dir + "/wifi.txt"
    with open(wifi_file, "a") as wf:
        wf.write(name + "\n")
        wf.write(pw + "\n")
    return '{"ok":"ok"}'

def delete_network(name):
    f = open(user_dir + "/wifi.txt", "r")
    lines = f.readlines()
    f.close()
    print name
    for i in range(len(lines)):
        print lines[i]
        if lines[i].rstrip() == name :
            print "MATHC"
            del lines[i:i+2]
            break

    f = open(user_dir + "/wifi.txt", "w")
    f.writelines(lines)
    f.close()
    return '{"ok":"ok"}'

def get_networks():
    network_names = []
    # check for wifi file, create one if not found
    wifi_file = user_dir + "/wifi.txt"
    if os.path.exists(wifi_file):
        f = open(user_dir + "/wifi.txt", "r")
    else :
        print "wifi file not found, creating"
        f = open(user_dir + "/wifi.txt", "w")
        #f.write("\n")
        #f.write("\n")
        f.close()
        f = open(user_dir + "/wifi.txt", "r")
    try :
        networks = f.readlines()
        networks = [x.strip() for x in networks] 
        ssids = networks[0::2]
        pws = networks[1::2]
        for i in range(len(ssids)) :
            if (ssids[i] != '') :
                network_names += [{'name': ssids[i]}]
                ssid = ssids[i]
    except : 
        print "bad wifi file" 

    return json.dumps(network_names, indent=4, encoding='utf-8')


