import os
import subprocess
import imp
import sys
import time

#vars
exprMin=0
exprMax=1023
switchMode=0

# usb or sd card
user_dir = os.getenv("USER_DIR", "/usbdrive")

# imports
current_dir = os.path.dirname(os.path.abspath(__file__))
og = imp.load_source('og', current_dir + '/og.py')

# UI elements
menu = og.Menu()

def run_cmd(cmd) :
    ret = False
    try:
        ret = subprocess.check_output(['bash', '-c', cmd], close_fds=True)
    except: 
        pass
    return ret

def quit():
    og.end_app()

# build main menu
menu.items = []
menu.header='Pedal Setup'

switchtypes=["Patch","Favourites"]

def switchType(i) :
    return switchtypes[i]

def getStrVal(key, dval) :
    s = run_cmd("grep '# " + key +",' < "+user_dir+"/pedal_cfg.sh| awk -F, ' { print $2 }'").strip()
    if(len(s)>0) :
        return s
    return dval

def getIntVal(key, dval) :
    s = run_cmd("grep '# " + key +",' < "+user_dir+"/pedal_cfg.sh| awk -F, ' { print $2 }'").strip()
    if(len(s)>0) :
        return int(s)
    return dval

def ExprMinSelect():
        global exprMin
        og.clear_screen()
        og.println(1,"Expr Min")
        og.println(2,str(exprMin))
        og.flip()
        og.enc_but_flag = False
        while True :
            og.enc_input()
            if (og.enc_turn_flag): 
                if(og.enc_turn and exprMin < 1023): 
                    exprMin += 1
                elif(og.enc_turn==0 and exprMin > 0):
                    exprMin -= 1
                og.clear_screen()
                og.println(1,"Expr Min")
                og.println(2,str(exprMin))
                og.flip()
            elif (og.enc_but_flag and og.enc_but==1):
                menu.items[menu.selection][0] = 'Expr Min : ' + str(exprMin)
                break

def ExprMaxSelect():
        global exprMax
        og.clear_screen()
        og.println(1,"Expr Max")
        og.println(2,str(exprMax))
        og.flip()
        og.enc_but_flag = False
        while True :
            og.enc_input()
            if (og.enc_turn_flag): 
                if(og.enc_turn and exprMax < 1023): 
                    exprMax += 1
                elif(og.enc_turn==0 and exprMax > 0):
                    exprMax -= 1
                og.clear_screen()
                og.println(1,"Expr Max")
                og.println(2,str(exprMax))
                og.flip()
            elif (og.enc_but_flag and og.enc_but==1):
                menu.items[menu.selection][0] = 'Expr Max : ' + str(exprMax)
                break

def SwitchModeSelect():
        global switchMode
        og.clear_screen()
        og.println(1,"Switch Mode")
        og.println(2,switchType(switchMode))
        og.flip()
        og.enc_but_flag = False
        while True :
            og.enc_input()
            if (og.enc_turn_flag): 
                if(og.enc_turn and switchMode < (len(switchtypes)-1)): 
                    switchMode += 1
                elif(og.enc_turn==0 and switchMode > 0):
                    switchMode -= 1
                og.clear_screen()
                og.println(1,"Switch Mode")
                og.println(2,switchType(switchMode))
                og.flip()
            elif (og.enc_but_flag and og.enc_but==1):
                menu.items[menu.selection][0] = 'Switch : ' + switchType(switchMode)
                break

def save():
    try:
        og.clear_screen()
        og.flip()
        
        # Check if user_dir exists and is writable
        if not os.path.exists(user_dir):
            og.clear_screen()
            og.println(1,"Error:")
            og.println(2,"Storage not found")
            og.flip()
            time.sleep(2)
            return
            
        if not os.access(user_dir, os.W_OK):
            og.clear_screen()
            og.println(1,"Error:")
            og.println(2,"Storage read-only")
            og.flip()
            time.sleep(2)
            return
        
        f = open(user_dir + "/pedal_cfg.sh", "w")
        # write parameters for possible reading
        f.write("# PEDAL PARAMETERS:START\n")
        f.write("# exprMin," + str(exprMin) + "\n")
        f.write("# exprMax," + str(exprMax) + "\n")
        f.write("# switchMode," + str(switchMode) + "\n")
        f.write("# PEDAL PARAMETERS:END\n")
        # write script to be executed
        f.write("oscsend localhost 4001 /pedal/exprMin i " + str(exprMin) + "\n")
        f.write("oscsend localhost 4001 /pedal/exprMax i " + str(exprMax) + "\n")
        f.write("oscsend localhost 4001 /pedal/switchMode i " + str(switchMode) + "\n")
        f.close()
        os.system("chmod +x "+user_dir+"/pedal_cfg.sh")
        os.system(user_dir+"/pedal_cfg.sh")
        og.clear_screen()
        og.println(1,"Pedal configuration")
        og.println(2,"SAVED")
        og.flip()
        os.system('oscsend localhost 4001 /pedalConfig i 1')
        time.sleep(0.5)    
    except Exception as e:
        og.clear_screen()
        og.println(1,"Save Error:")
        og.println(2,str(e)[:20])  # Truncate error message to fit screen
        og.flip()
        time.sleep(2)

# MAIN EXECUTION WITH FAILSAFE
def main():
    global exprMin, exprMax, switchMode
    
    # start it up
    og.start_app()

    exprMin=getIntVal('exprMin',0)
    exprMax=getIntVal('exprMax',1023)
    switchMode=getIntVal('switchMode',0)

    menu.items.append(['Expr Min : ' + str(exprMin) , ExprMinSelect])
    menu.items.append(['Expr Max : ' + str(exprMax) , ExprMaxSelect])
    menu.items.append(['Switch : ' + switchType(switchMode) , SwitchModeSelect])
    menu.items.append(['Save', save])
    menu.items.append(['< Home', quit])
    menu.selection = 0

    og.redraw_flag = True

    # enter menu
    menu.perform()

# Execute with failsafe
if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        # Log error if possible (could write to stderr or a log file)
        try:
            og.clear_screen()
            og.println(1,"System Error")
            og.println(2,"Exiting...")
            og.flip()
            time.sleep(1)
        except:
            pass  # Even error display failed, just exit cleanly
    finally:
        # ALWAYS call og.end_app() no matter what happens
        try:
            og.end_app()
        except:
            pass  # If og.end_app() itself fails, we've done our best
