#!/usr/bin/env python3
import os
import subprocess
import imp
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

def is_patch_editor_running():
    """Check if the Xpra Pd patch editor session is running"""
    try:
        subprocess.check_output(['systemctl', 'is-active', '--quiet', 'xpra-pd'], close_fds=True)
        return True
    except subprocess.CalledProcessError:
        return False

def stop_patch_editor():
    """Stop the Xpra Pd patch editor session"""
    og.clear_screen()
    og.println(1, "Stopping...")
    og.flip()

    run_cmd("sudo systemctl stop xpra-pd 2>&1 | systemd-cat --identifier=Organelle")

    time.sleep(1)

    og.clear_screen()
    og.println(1, "Patch Editor")
    og.println(2, "Stopped")
    og.flip()

    quit()

def start_patch_editor():
    """Start the Xpra Pd patch editor session"""
    og.clear_screen()
    og.println(1, "Starting...")
    og.flip()

    run_cmd("sudo systemctl start xpra-pd 2>&1 | systemd-cat --identifier=Organelle")

    time.sleep(1)

    quit()

def show_status():
    """Show patch editor status"""
    og.clear_screen()

    if is_patch_editor_running():
        og.println(1, "Patch Editor Running")
        # Get IP address using the same method as info.py / vnc_control.py
        try:
            wifi = imp.load_source('wifi_control', fw_dir + '/scripts/wifi_control.py')
            wifi.initialize_state()

            if wifi.wifi_connected():
                ip = wifi.ip_address
                if ip and ip != "not connected":
                    og.println(2, f"{ip}:10000")
                else:
                    og.println(2, "Port: 10000")
            else:
                og.println(2, "Port: 10000")
        except:
            og.println(2, "Port: 10000")
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
    og.start_app()

    menu.items = []
    menu.header = 'Patch Editor'

    if is_patch_editor_running():
        menu.items.append(['Status: Running', show_status])
        menu.items.append(['Stop', stop_patch_editor])
    else:
        menu.items.append(['Status: Stopped', show_status])
        menu.items.append(['Start', start_patch_editor])

    menu.items.append(['< Home', quit])
    menu.selection = 0

    og.redraw_flag = True

    menu.perform()

if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        try:
            og.clear_screen()
            og.println(1, "System Error")
            og.println(2, "Exiting...")
            og.flip()
            time.sleep(2)
        except:
            pass
    finally:
        try:
            og.end_app()
        except:
            pass
