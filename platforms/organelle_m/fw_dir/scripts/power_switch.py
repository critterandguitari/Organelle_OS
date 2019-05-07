import time
import RPi.GPIO as GPIO
import os

GPIO.setmode(GPIO.BCM)

GPIO.setup(13, GPIO.IN, pull_up_down=GPIO.PUD_UP)

time.sleep(1)

while True:
        if not GPIO.input(13):
            print "shutting down....................................................................................................................................................................................."
            os.system("sudo halt")
        time.sleep(1)

