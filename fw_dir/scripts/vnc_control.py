#!/usr/bin/env python3
import os
import subprocess
import imp
import sys
import time

# Get firmware directory from environment variable
fw_dir = os.getenv("FW_DIR", "/home/music/fw_dir")

# Import og module from firmware scripts directory
og = imp.load_source('og', fw_dir + '/scripts/og.py')

# UI elements
menu = og.Menu()

def run_cmd(cmd):
    ret = False
    try:
        ret = subprocess.check_output(['bash', '-c', cmd], stderr=subprocess.STDOUT, close_fds=True, text=True)
    except subprocess.CalledProcessError as e:
        ret = e.output
    except:
        pass
    return ret

def is_vnc_running():
    """Check if VNC server is running"""
    try:
        # Check for Xvnc or vncserver-virtual process
        result = subprocess.check_output(['pgrep', '-x', 'Xvnc'], close_fds=True)
        return True
    except subprocess.CalledProcessError:
        try:
            result = subprocess.check_output(['pgrep', '-f', 'vncserver-virtual'], close_fds=True)
            return True
        except subprocess.CalledProcessError:
            return False

def update_menu_items():
    """Update menu item text based on current VNC status"""
    if is_vnc_running():
        menu.items[0][0] = 'Status: Running'
        menu.items[1][0] = 'Stop Server'
    else:
        menu.items[0][0] = 'Status: Stopped'
        menu.items[1][0] = 'Start Server'

def stop_vnc():
    """Stop VNC server"""
    og.clear_screen()
    og.println(1, "Stopping VNC...")
    og.flip()
    
    run_cmd("pkill -f vncserver-virtual 2>&1 | systemd-cat --identifier=Organelle")
    run_cmd("pkill -x Xvnc 2>&1 | systemd-cat --identifier=Organelle")
    run_cmd("vncserver-virtual -kill :1 2>&1 | systemd-cat --identifier=Organelle")
    
    time.sleep(1)
    
    og.clear_screen()
    og.println(1, "VNC Server")
    og.println(2, "Stopped")
    og.flip()
    
    # restart mother and quit
    run_cmd(fw_dir + "/scripts/start-mother.sh")
    quit()


def start_vnc():
    """Start VNC server"""
    og.clear_screen()
    og.println(1, "Starting VNC...")
    og.flip()
    
    # Remount filesystem as read-write
    run_cmd(f"sudo {fw_dir}/scripts/remount-rw.sh 2>&1 | systemd-cat --identifier=Organelle")
    
    # Try to start VNC
    output = run_cmd("vncserver-virtual -geometry 1920x1080 2>&1 | systemd-cat --identifier=Organelle")
    
    quit()  # quit when it starts, bc this will get killed
    
    time.sleep(1)
    


def toggle_action():
    """Toggle VNC based on current state"""
    if is_vnc_running():
        stop_vnc()
    else:
        start_vnc()
       
def show_status():
    """Show VNC server status"""
    og.clear_screen()
    
    if is_vnc_running():
        og.println(1, "VNC Running")
        # Get IP address using the same method as info.py
        try:
            # Import wifi_control from scripts directory
            wifi = imp.load_source('wifi_control', fw_dir + '/scripts/wifi_control.py')
            wifi.initialize_state()
            
            if wifi.wifi_connected():
                ip = wifi.ip_address
                if ip and ip != "not connected":
                    og.println(2, f"{ip}:5901")
                else:
                    og.println(2, "Port: 5901")
            else:
                og.println(2, "Port: 5901")
        except:
            # Fallback if wifi module fails
            og.println(2, "Port: 5901")
    else:
        og.println(2, "Stopped")

    og.println(4, "< Back")
    og.invert_line(4)

    og.flip()
    og.enc_but_flag = False
    og.enc_turn_flag = False
    
    # Wait for encoder button press or turn to return
    while True:
        og.enc_input()
        if (og.enc_but_flag and og.enc_but == 1) or og.enc_turn_flag:
            break


        
def quit():
    og.end_app()

# MAIN EXECUTION WITH FAILSAFE
def main():
    # start it up
    og.start_app()

    # Build menu items - the action function is the same (toggle_action)
    # but the text will change based on status
    menu.items = []
    menu.header = 'VNC Server'
    
    # Initialize menu items
    if is_vnc_running():
        menu.items.append(['Status: Running', show_status])
        menu.items.append(['Stop Server', toggle_action])
    else:
        menu.items.append(['Status: Stopped', show_status])
        menu.items.append(['Start Server', toggle_action])
    
    menu.items.append(['< Home', quit])
    menu.selection = 0

    og.redraw_flag = True

    # enter menu - this runs until quit() is called
    menu.perform()

# Execute with failsafe
if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        # Log error if possible
        try:
            og.clear_screen()
            og.println(1, "System Error")
            og.println(2, "Exiting...")
            og.flip()
            time.sleep(2)
        except:
            pass
    finally:
        # ALWAYS call og.end_app() no matter what happens
        try:
            og.end_app()
        except:
            pass

