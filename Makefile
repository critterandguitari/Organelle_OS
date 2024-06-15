CXX = g++

CXXFLAGS += -std=c++11

objects =  \
	main.o \
	AppData.o \
	Timer.o \
	MainMenu.o \
	OledScreen.o \
	SLIPEncodedSerial.o \
	Serial.o Socket.o \
	UdpSocket.o \
	OSC/OSCData.o \
	OSC/OSCMatch.o \
	OSC/OSCMessage.o \
	OSC/OSCTiming.o \
	OSC/SimpleWriter.o

SDL_CFLAGS := $(shell pkg-config --cflags sdl2)
SDL_LIBS := $(shell pkg-config --libs sdl2)

default :
	@echo "platform not specified"

sdlpi : CXXFLAGS += $(SDL_CFLAGS) -DSDLPI_HW -DORGANELLE_HW_WIDTH=800 -DORGANELLE_HW_HEIGHT=600
sdlpi : $(objects) hw_interfaces/SDLPi.o
	$(CXX) $(SDL_LIBS) -o fw_dir/mother $(objects) hw_interfaces/SDLPi.o

organelle : CXXFLAGS += -DSERIAL_HW
organelle : $(objects) hw_interfaces/SerialMCU.o
	$(CXX) -o fw_dir/mother $(objects) hw_interfaces/SerialMCU.o

organelle_m : CXXFLAGS += -DCM3GPIO_HW -DMICSEL_SWITCH -DPWR_SWITCH -DOLED_30FPS -DBATTERY_METER -DFIX_ABL_LINK
organelle_m : $(objects) hw_interfaces/CM3GPIO.o
	$(CXX) -o fw_dir/mother $(objects) hw_interfaces/CM3GPIO.o -l wiringPi

organelle_4 : CXXFLAGS += -DCM4OG4_HW -DOLED_30FPS 
organelle_4 : $(objects) hw_interfaces/CM4OG4.o
	$(CXX) -o fw_dir/mother $(objects) hw_interfaces/CM4OG4.o -l wiringPi

.PHONY : clean

clean :
	rm -f main $(objects) fw_dir/mother hw_interfaces/SDLPi.o hw_interfaces/SerialMCU.o hw_interfaces/CM3GPIO.o

IMAGE_BUILD_VERSION = $(shell cat fw_dir/version)
IMAGE_BUILD_TAG = $(shell cat fw_dir/buildtag)
IMAGE_VERSION = $(IMAGE_BUILD_VERSION)$(IMAGE_BUILD_TAG)
IMAGE_DIR = UpdateOS-$(IMAGE_VERSION)

organelle_deploy : organelle
	@echo "Updating OS to $(IMAGE_VERSION)"
	fw_dir/scripts/remount-rw.sh
	@echo "copying fw files to /root"
	rm -fr /root/fw_dir
	mkdir /root/fw_dir
	cp -fr fw_dir/* /root/fw_dir
	@echo "copying version file to root for backwards compatiblility"
	cp -fr fw_dir/version /root
	@echo "copying system files"
	cp -fr platforms/organelle/rootfs/* /
	sync

organelle_m_deploy : organelle_m
	@echo "Updating OS to $(IMAGE_VERSION)"
	@echo "copying common fw files"
	rm -fr /home/music/fw_dir
	mkdir /home/music/fw_dir
	cp -fr fw_dir/* /home/music/fw_dir
	@echo "copying platform fw files"
	cp -fr platforms/organelle_m/fw_dir/* /home/music/fw_dir
	chown -R music:music /home/music/fw_dir
	@echo "copying version file to root for backwards compatiblility"
	cp -fr fw_dir/version /root
	@echo "copying systems files"
	mkdir tmp
	cp -r platforms/organelle_m/rootfs tmp/
	chown -R root:root tmp/rootfs
	chown -R music:music tmp/rootfs/home/music
	cp -fr --preserve=mode,ownership tmp/rootfs/* /
	rm -fr tmp
	sync

organelle_4_deploy : organelle_4
	@echo "Updating OS to $(IMAGE_VERSION)"
	@echo "copying common fw files"
	rm -fr /home/music/fw_dir
	mkdir /home/music/fw_dir
	cp -fr fw_dir/* /home/music/fw_dir
	@echo "copying platform fw files"
	cp -fr platforms/organelle_4/fw_dir/* /home/music/fw_dir
	chown -R music:music /home/music/fw_dir
	@echo "copying version file to root for backwards compatiblility"
	cp -fr fw_dir/version /root
	@echo "copying systems files"
	mkdir tmp
	cp -r platforms/organelle_4/rootfs tmp/
	chown -R root:root tmp/rootfs
	chown -R music:music tmp/rootfs/home/music
	cp -fr --preserve=mode,ownership tmp/rootfs/* /
	rm -fr tmp
	sync

# Generate with g++ -MM *.c* OSC/*.* 
AppData.o: AppData.cpp AppData.h OledScreen.h
MainMenu.o: MainMenu.cpp MainMenu.h AppData.h OledScreen.h
OledScreen.o: OledScreen.cpp OledScreen.h fonts.h simple_svg_1.0.0.hpp
SLIPEncodedSerial.o: SLIPEncodedSerial.cpp SLIPEncodedSerial.h Serial.h \
  UdpSocket.h Socket.h
Serial.o: Serial.cpp Serial.h
Socket.o: Socket.cpp Socket.h
Timer.o: Timer.cpp Timer.h
UdpSocket.o: UdpSocket.cpp UdpSocket.h Socket.h
main.o: main.cpp OSC/OSCMessage.h OSC/OSCData.h OSC/OSCTiming.h \
  OSC/SimpleWriter.h Serial.h UdpSocket.h Socket.h SLIPEncodedSerial.h \
  OledScreen.h MainMenu.h AppData.h Timer.h
serialdump.o: serialdump.c
test.o: test.cpp
OSCData.o: OSC/OSCData.cpp OSC/OSCData.h OSC/OSCTiming.h
OSCMatch.o: OSC/OSCMatch.c OSC/OSCMatch.h
OSCMessage.o: OSC/OSCMessage.cpp OSC/OSCMessage.h OSC/OSCData.h \
  OSC/OSCTiming.h OSC/SimpleWriter.h OSC/OSCMatch.h
OSCTiming.o: OSC/OSCTiming.cpp OSC/OSCTiming.h
SimpleWriter.o: OSC/SimpleWriter.cpp OSC/SimpleWriter.h
SerialMCU.o: hardwares/SerialMCU.cpp hardwares/SerialMCU.h \
 hardwares/../OledScreen.h
SDLPi.o: hardwares/SDLPi.cpp hardwares/SDLPi.h \
 hardwares/../OledScreen.h
