#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <sys/stat.h>

#include "OledScreen.h"
#include "Timer.h"
#include "AppData.h"

// include hardware interface
// default to organelle original
#ifdef CM3GPIO_HW
    #include "hw_interfaces/CM3GPIO.h"
#endif

#ifdef CM4OG4_HW
    #include "hw_interfaces/CM4OG4.h"
#endif

#ifdef SDLPI_HW
    #include "hw_interfaces/SDLPi.h"
#endif

#ifdef SERIAL_HW
    #include "hw_interfaces/SerialMCU.h"
#endif

// hardware interface controls
// default to organelle original
#ifdef CM3GPIO_HW
    CM3GPIO controls;
#endif

#ifdef CM4OG4_HW
    CM4OG4 controls;
#endif

#ifdef SDLPI_HW
    SDLPi controls;
#endif

#ifdef SERIAL_HW
    SerialMCU controls;
#endif

// global app states, flags, screens, etc...
AppData app;

int main(int argc, char* argv[]) {
    printf("Organelle Splash Screen - build date " __DATE__ "   " __TIME__ "\n");
    
    Timer screenTimer;
    screenTimer.reset();
    
    // Initialize hardware controls
    controls.init();
    
    // Setup OLED screen
    app.oled(AppData::MENU).showInfoBar = false;
    app.oled(AppData::MENU).clear();
    
    // Display "hello" message
    app.oled(AppData::MENU).setLine(3, "Welcome to Organelle");
    app.oled(AppData::MENU).newScreen = 1;
    
    // Update the OLED display
    controls.updateOLED(app.oled(AppData::MENU));
    
    printf("Splash screen displayed\n");
    
    // Keep the splash screen up for a few seconds
    // You can adjust this timing or remove it entirely if you want
    // the splash to stay until the main program takes over
    int displayTime = 1000; // 3 seconds in milliseconds
    
    while (screenTimer.getElapsed() < displayTime) {
        // Small delay to prevent busy waiting
        usleep(50000); // 50ms
        
        // Optional: you can add simple animations here later
        // For now, just keep the display updated
        //if (screenTimer.getElapsed() > 100) { // Update every 100ms
        //    controls.updateOLED(app.oled(AppData::MENU));
            // Reset timer for next update (if you want periodic updates)
        //}
    }
    
    printf("Splash screen finished\n");
    return 0;
}
