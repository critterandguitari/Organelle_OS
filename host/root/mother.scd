// mother host output
~motherHost = NetAddr("127.0.0.1", 4001);
~motherHost.sendMsg("/patchLoaded",1);
~motherHost.sendMsg("/oled/setscreen",3);

// oled function
~oled=();
~oled[\screen] = {
    arg self,line,str;
    ~motherHost.sendMsg(format("/oled/line/%",line), str);
};

~oled.screen(1,"SC loading");


// keys callback
~keys=();
~keys[\key_hit] = { 
    arg self,key,val;
    self.changed(\key,key,val);
};

OSCdef( \OrganelleKeys,
    {
        arg msg, time, addr, recvPort; 
        ~keys.key_hit(msg[1],msg[2]);
    },
    "/key",
    recvPort:4000
);

// knobs callbacks
~knobs=();
~knobValues = [0,0,0,0,0];

~knobs[\knob_changed] = { 
    arg self, knob,val;
    if(~knobValues[knob] != val,
    {
        ~knobValues[knob] = val;
        self.changed(\knob,knob,val);
    });
};

OSCdef( \OrganelleKnobs,
    {
        arg msg, time, addr, recvPort; 
        ~knobs.knob_changed(0,msg[1]/1023.0);
        ~knobs.knob_changed(1,msg[2]/1023.0);
        ~knobs.knob_changed(2,msg[3]/1023.0);
        ~knobs.knob_changed(3,msg[4]/1023.0);
        ~knobs.knob_changed(4,msg[5]/1023.0);
    },
    "/knobs",
    recvPort:4000
);

// boot the server
(

    s.waitForBoot { (
        s.initTree;
        "/tmp/patch/main.scd".load;
    )};
)

