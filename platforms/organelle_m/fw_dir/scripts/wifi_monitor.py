import time
import os
import subprocess

def wifi_connected():
    """Check if WiFi is connected - simplified version for root process"""
    try:
        # No sudo needed since we're running as root
        result = subprocess.run(['wpa_cli', '-i', 'wlan0', 'status'], 
                              capture_output=True, 
                              text=True, 
                              timeout=5)
        if result.returncode == 0:
            wifi_info = result.stdout.splitlines()
            return any("ip_address" in s for s in wifi_info)
    except:
        return False
    return False

def send_wifi_status(connected):
    """Send WiFi status via OSC without sudo"""
    status = 1 if connected else 0
    try:
        subprocess.run(['oscsend', 'localhost', '4001', '/wifiStatus', 'i', str(status)], 
                      capture_output=True, timeout=2)
    except:
        pass  # Silently handle oscsend failures

# Main monitoring loop
while True:
    connected = wifi_connected()
    send_wifi_status(connected)
    time.sleep(2)
