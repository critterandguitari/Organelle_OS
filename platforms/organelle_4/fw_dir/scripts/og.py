import liblo
import time 
import sys
import os

enc_turn = 0
enc_but = 0
enc_turn_flag = False
enc_but_flag = False
redraw_flag = False  # to break waiting for input for a screen update
osc_target = None
osc_server = None

# sometimes it takes a second to aquire the OSC port
# this provides a loading screen
def loading_screen ():
    os.system('oscsend localhost 4001 /oled/gClear ii 1 1')
    os.system('oscsend localhost 4001 /oled/aux/line/3 s "loading..."')
    os.system('oscsend localhost 4001 /enableauxsub i 1')
    os.system('oscsend localhost 4001 /oled/setscreen i 1')

def alert(s):
    os.system('oscsend localhost 4001 /oled/gClear ii 1 1')
    os.system('oscsend localhost 4001 /oled/aux/line/3 s "'+s+'"')
    os.system('oscsend localhost 4001 /enableauxsub i 1')
    os.system('oscsend localhost 4001 /oled/setscreen i 1')

# OSC and UI primitives 
def start_app ():
    loading_screen()
    init_osc()

def end_app ():
    # send this as system call just in case something happend to our liblo sender
    os.system('oscsend localhost 4001 /gohome i 1')
    os.system('oscsend localhost 4001 /enableauxsub i 0')
    osc_server.free()
    exit()

def invert_line(num) :
    liblo.send(osc_target, '/oled/gInvertArea', 1, 0, num*11+1, 127, 11)

def truncate_mid(s, n):
    if len(s) <= n:
        return s
    n_2 = int(n) / 2 - 3
    n_1 = n - n_2 - 3
    return '{0}...{1}'.format(s[:n_1], s[-n_2:])

def println(num, s) :
    #s = truncate_mid(s, 20)
    liblo.send(osc_target, '/oled/gPrintln', 1, 2, num*11 + 2, 8, 1, s[0:20])

def println_right(num, s) :
    #s = truncate_mid(s, 20)
    liblo.send(osc_target, '/oled/gPrintln', 1, 90, num*11 + 10, 8, 1, s[0:20])

def println16(num, s) :
    #s = truncate_mid(s, 20)
    liblo.send(osc_target, '/oled/gPrintln', 1, 2, num*11 + 6, 16, 1, s[0:20])

def clear_screen() :
    liblo.send(osc_target, '/oled/gClear', 1, 1)

def flip() :
    liblo.send(osc_target, '/oled/gFlip', 1)

def init_osc() :
    global osc_server, osc_target
    print ("config osc target")
    osc_target = liblo.Address(4001)
    print ("config osc osc_server")
    # make sure the port is available... ahh ok
    os.system("fuser -k 4002/udp")
    try:
        osc_server = liblo.Server(4002)
        osc_server.add_method("/encoder/turn", 'i', enc_turn)
        osc_server.add_method("/encoder/button", 'i', enc_press)

    except (liblo.ServerError, err):
        print (str(err))
        sys.exit()
    print ("done config osc_server")

def enc_turn(path, args) :
    global enc_turn_flag, enc_turn
    enc_turn_flag = True
    enc_turn = args[0]

def enc_press(path, args) :
    global enc_but_flag, enc_but
    enc_but_flag = True
    enc_but = args[0]

# wait for input, or for redraw flag to be set
def enc_input():
    global osc_server, enc_turn_flag, enc_but_flag, redraw_flag
    enc_turn_flag = False
    enc_but_flag = False
    redraw_flag = False
    while True :
        osc_server.recv(10)
        if (enc_turn_flag or enc_but_flag) : break
        if (redraw_flag) : break

def wait_for_turn():
    while True :
        enc_input()
        if (enc_turn_flag): break
    return enc_turn

def wait_for_press():
    while True :
        enc_input()
        if (enc_but_flag and (enc_but == 1)): break
    return enc_but

def wait_for_release():
    while True :
        enc_input()
        if (enc_but_flag and (enc_but == 0)): break
    return enc_but

# UI helpers

class Menu :
    items = None
    selection = 0
    menu_offset = 0
    cursor_offset = 0
    back_flag = False
    header = ''

    def draw(self) :
        clear_screen()

        # header first line
        println(0, self.header)

        # menu entries for the rest
        sz = min(len(self.items),4)
        for i in range(0, sz) :
            println(i+1, self.items[i + self.menu_offset][0])
        invert_line(self.cursor_offset + 1)
        
        flip()
   
    def back(self):
        self.back_flag = True
 
    def enter(self) :
        self.perform()

    def enc_up(self) :
        if (self.cursor_offset == 3) :
            if not (self.menu_offset >= (len(self.items) - 4)) : self.menu_offset +=1
        if not (self.cursor_offset >= 3) : self.cursor_offset += 1
        self.selection = self.cursor_offset + self.menu_offset

    def enc_down(self) :
        if (self.cursor_offset == 0) :
            if not (self.menu_offset < 1) : self.menu_offset -= 1
        if not (self.cursor_offset < 1) : self.cursor_offset -= 1
        self.selection = self.cursor_offset + self.menu_offset

    def perform(self) :
        self.back_flag = False
        self.draw()
        while True :
            enc_input()
            if (enc_turn_flag) :
                i = enc_turn
                if i == 0 :
                    self.enc_down()
                if i == 1 :
                    self.enc_up()
                self.draw()
            if (enc_but_flag) :
                if (enc_but == 1) :
                    self.items[self.selection][1]()
                    if (self.back_flag) : break
                    else : self.draw()
            if (redraw_flag) :
                self.draw()

class InfoList :
    items = None
    menu_offset = 0
    header = ''

    def draw(self) :
        clear_screen()

        # header first line
        println(0, self.header)

        # menu entries for the rest
        sz = min(len(self.items),4)
        for i in range(0, sz) :
            println(i+1, self.items[i + self.menu_offset])
        
        flip()

    def enc_up(self) :
        if not (self.menu_offset >= (len(self.items) - 4)) : self.menu_offset +=1

    def enc_down(self) :
        if not (self.menu_offset < 1) : self.menu_offset -= 1

    def perform(self) :
        self.back_flag = False
        self.draw()
        while True :
            enc_input()
            if (enc_turn_flag) :
                i = enc_turn
                if i == 0 :
                    self.enc_down()
                if i == 1 :
                    self.enc_up()
                self.draw()
            if (enc_but_flag) :
                if (enc_but == 1) :
                    break
            if (redraw_flag) :
                self.draw()


class PasswordEntry:
    def __init__(self, header="Enter Password", max_length=16):
        self.header = header
        self.max_length = max_length
        self.password = ""
        self.back_flag = False
        self.done_flag = False

        # Character set for selection (no space, we'll add commands at the end)
        self.charset = (
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "0123456789"
            "!@#$%^&*()_+-=[]{}|;:,.<>?/~`"
        )

        # Commands that appear at the end of the character list
        self.commands = ["Space", "Delete", "Enter", "Cancel"]
        self.total_options = len(self.charset) + len(self.commands)
        self.char_index = 0

    def get_current_selection(self):
        if self.char_index < len(self.charset):
            return self.charset[self.char_index]
        else:
            # Return command name
            cmd_index = self.char_index - len(self.charset)
            return self.commands[cmd_index]

    def is_command(self):
        return self.char_index >= len(self.charset)

    def draw(self):
        clear_screen()

        # Header
        println(0, self.header)

        # Current password (always show actual characters)
        current_selection = f"   {self.get_current_selection()}"
        pwd_line = f"{self.password}"

        println16(1, current_selection[:20])  # Truncate to screen width

        println16(3, pwd_line[:20])  # Truncate to screen width

        # Show current position info
        pos_info = f"{self.char_index + 1}/{self.total_options}"
        println_right(1, pos_info)

        flip()

    def enc_up(self):
        self.char_index = (self.char_index + 1) % self.total_options

    def enc_down(self):
        self.char_index = (self.char_index - 1) % self.total_options

    def execute_selection(self):
        if self.is_command():
            cmd_index = self.char_index - len(self.charset)
            command = self.commands[cmd_index]

            if command == "Space":
                if len(self.password) < self.max_length:
                    self.password += " "
            elif command == "Delete":
                if len(self.password) > 0:
                    self.password = self.password[:-1]
            elif command == "Enter":
                self.done_flag = True
            elif command == "Cancel":
                self.back_flag = True
        else:
            # Regular character
            if len(self.password) < self.max_length:
                self.password += self.get_current_selection()

    def back(self):
        self.back_flag = True

    def done(self):
        self.done_flag = True

    def perform(self):
        self.back_flag = False
        self.done_flag = False
        self.password = ""
        self.char_index = 0

        self.draw()

        while True:
            enc_input()

            if enc_turn_flag:
                if enc_turn == 0:
                    self.enc_down()
                elif enc_turn == 1:
                    self.enc_up()
                self.draw()

            if enc_but_flag:
                if enc_but == 1:  # Button pressed
                    self.execute_selection()
                    if self.done_flag:
                        break
                    self.draw()

            if redraw_flag:
                self.draw()

            if self.back_flag:
                break

        return self.password if self.done_flag else None


