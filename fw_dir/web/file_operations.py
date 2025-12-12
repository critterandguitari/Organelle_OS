import json
import os
import shutil
import stat
import pwd

import liblo

# Set up OSC target for organelle app
try:
    osc_target = liblo.Address(4001)
except liblo.AddressError as err:
    print(err)
    osc_target = None

BASE_DIR = "/"

def check_path(path) :
    path = os.path.normpath(path)
    print(path)
    if path.startswith("/usbdrive") or path.startswith("/sdcard") : return True
    else : return False

def check_and_inc_name(path) :
    newpath = path
    count = 2
    while os.path.isdir(newpath) or os.path.isfile(newpath):
        p, e = os.path.splitext(path)
        newpath = p + " " + str(count) + e
        count += 1

    return newpath

def rename(old, new):
    src = BASE_DIR + old 
    dst = os.path.dirname(src) + '/' + new
    if src != dst :
        dst = check_and_inc_name(dst)
        os.rename(src, dst)
    if osc_target:
        liblo.send(osc_target, "/reload", 1)
    return '{"ok":"ok"}'

def zip(folder):
    folder = BASE_DIR + folder
    zipname = os.path.basename(folder)+".zip"
    if os.path.isdir(folder):
        os.system("cd \""+os.path.dirname(folder)+"\" && zip -r \""+zipname+"\" \""+os.path.basename(folder)+"\" 2>&1 | systemd-cat --identifier=Organelle")
    return '{"ok":"ok"}'

def create(dst, name):
    dst = BASE_DIR + dst + '/' + name
    dst = check_and_inc_name(dst)
    os.mkdir(dst)
    if osc_target:
        liblo.send(osc_target, "/reload", 1)
    return '{"ok":"ok"}'

def move(src, dst):
    src = BASE_DIR + src
    dst = BASE_DIR + dst + '/' + os.path.basename(src)  
    dst = check_and_inc_name(dst)
    shutil.move(src, dst)
    if osc_target:
        liblo.send(osc_target, "/reload", 1)
    return '{"ok":"ok"}'

def unzip(zip_path):
    zip_path = BASE_DIR + zip_path
    zip_parent_folder = os.path.dirname(zip_path)
    os.system("unzip -o \""+zip_path+"\" -d \""+zip_parent_folder+"\" -x '__MACOSX/*' 2>&1 | systemd-cat --identifier=Organelle")
    if osc_target:
        liblo.send(osc_target, "/reload", 1)
    return '{"ok":"ok"}'

def copy(src, dst):
    src = BASE_DIR + src
    dst = BASE_DIR + dst 
    dst = dst + '/' + os.path.basename(src)
    dst = check_and_inc_name(dst)
    if os.path.isfile(src) :
        shutil.copy(src, dst)
    if os.path.isdir(src) :
        shutil.copytree(src, dst)
    if osc_target:
        liblo.send(osc_target, "/reload", 1)
    return '{"ok":"ok"}'

def delete(src):
    src = BASE_DIR + src 
    if os.path.isfile(src) :
        os.remove(src)
    if os.path.isdir(src) :
        shutil.rmtree(src)
    if osc_target:
        liblo.send(osc_target, "/reload", 1)
    return '{"ok":"ok"}'

def create_file(dst, name):
    dst = BASE_DIR + dst + '/' + name
    dst = check_and_inc_name(dst)
    # Create an empty file
    open(dst, 'a').close()
    return '{"ok":"ok"}'

def get_node(fpath):
    if fpath == '#' :
        return get_files(BASE_DIR)
    else :
        fpath = fpath
        return get_files(BASE_DIR + fpath)

def convert_bytes(num):
    for x in ['bytes', 'KB', 'MB', 'GB', 'TB']:
        if num < 1024.0:
            if x == 'bytes' : return "%d %s" % (int(num), x)
            else : return "%3.1f %s" % (num, x)
        num /= 1024.0

def file_to_dict(fpath):
    return {
        'name': os.path.basename(fpath),
        'children': False,
        'type': 'file',
        'size': str(convert_bytes(os.stat(fpath).st_size)), 
        'path': fpath.split(BASE_DIR,1)[1],
        }

def folder_to_dict(fpath):
    return {
        'name': os.path.basename(fpath),
        'children': True,
        'type': 'folder',
        'path': fpath.split(BASE_DIR,1)[1],
        }

def get_files(rootpath):
    root, folders, files = next(os.walk(rootpath))
    contents = []

    # Fix root path issue
    if root == "//":
        root = "/"

    # If we are in the root directory, only allow 'sdcard' and 'usbdrive'
    if root == "/":
        folders = [f for f in folders if f in ("sdcard", "usbdrive")]

    # Sort folders and files
    folders = sorted(folders, key=lambda s: s.lower())
    files = sorted(files, key=lambda s: s.lower())

    # Helper function to check if item is owned by root
    def is_owned_by_root(path):
        try:
            file_stat = os.stat(path)
            return file_stat.st_uid == 0  # UID 0 is root
        except (OSError, IOError):
            # If we can't stat the file, exclude it for safety
            return True

    # Add filtered folders to the list
    for folder in folders:
        if not folder.startswith('.') and folder != "__pycache__":
            path = os.path.join(root, folder)
            # Skip if owned by root
            if not is_owned_by_root(path):
                contents.append(folder_to_dict(path))

    # Add files (only if not in root, otherwise ignore them)
    if root != "/":
        for ffile in files:
            if not ffile.startswith('.'):
                path = os.path.join(root, ffile)
                # Skip if owned by root
                if not is_owned_by_root(path):
                    contents.append(file_to_dict(path))

    return json.dumps(contents, indent=4)
