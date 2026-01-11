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

# Simplified objects for splash screen (no OSC, networking, or menu)
splash_objects = \
	splash.o \
	AppData.o \
	Timer.o \
	OledScreen.o

SDL_CFLAGS := $(shell pkg-config --cflags sdl2)
SDL_LIBS := $(shell pkg-config --libs sdl2)

default :
	@echo "platform not specified"

# Pattern rules for building into obj directories
obj/cm3/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

obj/cm3/%.o: %.c
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

obj/cm4/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

obj/cm4/%.o: %.c
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

obj/splash/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

obj/splash/%.o: %.c
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

obj/sdlpi/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Convert object list to cm3 paths
cm3_objects = $(addprefix obj/cm3/, $(objects))
cm4_objects = $(addprefix obj/cm4/, $(objects))
splash_cm_objects = $(addprefix obj/splash/, $(splash_objects))
sdlpi_objects = $(addprefix obj/sdlpi/, $(objects))

sdlpi : CXXFLAGS += $(SDL_CFLAGS) -DSDLPI_HW -DORGANELLE_HW_WIDTH=800 -DORGANELLE_HW_HEIGHT=600
sdlpi : $(sdlpi_objects) obj/sdlpi/hw_interfaces/SDLPi.o
	@mkdir -p fw_dir
	$(CXX) $(SDL_LIBS) -o fw_dir/mother $(sdlpi_objects) obj/sdlpi/hw_interfaces/SDLPi.o

organelle_cm3 : CXXFLAGS += -DCM3GPIO_HW -DMICSEL_SWITCH -DPWR_SWITCH -DOLED_30FPS -DBATTERY_METER -DFIX_ABL_LINK
organelle_cm3 : $(cm3_objects) obj/cm3/hw_interfaces/CM3GPIO.o
	@mkdir -p fw_dir
	$(CXX) -o fw_dir/mother_cm3 $(cm3_objects) obj/cm3/hw_interfaces/CM3GPIO.o -l wiringPi

organelle_cm4 : CXXFLAGS += -DCM4OG4_HW -DOLED_30FPS -DMICSEL_SWITCH -DBATTERY_METER -DSTORAGE_INDICATOR -DPWR_SWITCH -DMICSEL_SWITCH -DFIX_ABL_LINK
organelle_cm4 : $(cm4_objects) obj/cm4/hw_interfaces/CM4OG4.o
	@mkdir -p fw_dir
	$(CXX) -o fw_dir/mother_cm4 $(cm4_objects) obj/cm4/hw_interfaces/CM4OG4.o -l wiringPi

organelle_cm_splash : CXXFLAGS += -DCM4OG4_HW -DOLED_30FPS -DMICSEL_SWITCH -DBATTERY_METER -DPWR_SWITCH -DMICSEL_SWITCH -DFIX_ABL_LINK
organelle_cm_splash : $(splash_cm_objects) obj/splash/hw_interfaces/CM4OG4.o
	@mkdir -p fw_dir
	$(CXX) -o fw_dir/splash $(splash_cm_objects) obj/splash/hw_interfaces/CM4OG4.o -l wiringPi

.PHONY : clean

clean :
	rm -rf obj fw_dir/splash fw_dir/mother_cm3 fw_dir/mother_cm4 fw_dir/mother

IMAGE_BUILD_VERSION = $(shell cat fw_dir/version)
IMAGE_BUILD_TAG = $(shell cat fw_dir/buildtag)
IMAGE_VERSION = $(IMAGE_BUILD_VERSION)$(IMAGE_BUILD_TAG)
IMAGE_DIR = UpdateOS-$(IMAGE_VERSION)

organelle_cm_deploy : organelle_cm3 organelle_cm4 organelle_cm_splash
	@echo "Updating OS to $(IMAGE_VERSION)"
	@echo "copying common fw files"
	rm -fr /home/music/fw_dir
	mkdir /home/music/fw_dir
	cp -fr fw_dir/* /home/music/fw_dir
	@echo "copying platform fw files"
	cp -fr platforms/organelle_cm/fw_dir/* /home/music/fw_dir
	chown -R music:music /home/music/fw_dir
	@echo "copying version file to root for backwards compatiblility"
	cp -fr fw_dir/version /root
	@echo "copying systems files"
	mkdir tmp
	cp -r platforms/organelle_cm/rootfs tmp/
	chown -R root:root tmp/rootfs
	chown -R music:music tmp/rootfs/home/music
	cp -fr --preserve=mode,ownership tmp/rootfs/* /
	rm -fr tmp
	@echo "copying test patch"
	cp -r platforms/organelle_cm/Test /sdcard/Patches/Utilities/
	chown -R music:music /sdcard/Patches/Utilities/Test
	sync
