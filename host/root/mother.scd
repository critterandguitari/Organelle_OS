~motherHost = NetAddr("127.0.0.1", 4001);
~motherHost.sendMsg("/patchLoaded",1);
~motherHost.sendMsg("/oled/setscreen",3);
~motherHost.sendMsg("/oled/line/1","SC loading");


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

~knobs=();
~knobValues = [0,0,0,0,0];

~knobs[\knob_changed] = { 
    arg self, knob,val;
    ~knobValues[knob] = val;
    self.changed(\knob,knob,val);
};

OSCdef( \OrganelleKnobs,
    {
        arg msg, time, addr, recvPort; 
        ~knobs.knob_changed(0,msg[1]);
        ~knobs.knob_changed(1,msg[2]);
        ~knobs.knob_changed(2,msg[3]);
        ~knobs.knob_changed(3,msg[4]);
        ~knobs.knob_changed(4,msg[5]);
    },
    "/knobs",
    recvPort:4000
);

(

    s.waitForBoot { (
        s.initTree;
        g = File("/tmp/patch/main.scd".standardizePath,"r");
        g.readAllString.interpret;
        g.close;
    )};
)

