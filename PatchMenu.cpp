
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <locale.h>
#include <string.h>


#include "PatchMenu.h"


PatchMenu::PatchMenu(){

    num_patches = 0;
    selected_patch = 0;
    patchlist_offset = 0;
    curser_offset = 0;
 
}

void PatchMenu::encoderUp(void) {
//    if (!(patchlist_offset >= (num_patches - 1))) patchlist_offset++;
//    printf("patch: %d\n", patchlist_offset);
    if (curser_offset == 4) {
        if (!(patchlist_offset >= (num_patches - 1))) patchlist_offset++;
    }
    if (!(curser_offset >= 4)) curser_offset++;
    
}

void PatchMenu::encoderDown(void) {
//    if (!(patchlist_offset < 1)) patchlist_offset--;
//    printf("patch: %d\n", patchlist_offset);
    if (curser_offset == 0) {
        if (!(patchlist_offset < 1)) patchlist_offset--;
    }
    if (!(curser_offset < 1)) curser_offset--;
}

void PatchMenu::encoderPress(void){
    char cmd[256];
    
    selected_patch =  patchlist_offset + curser_offset;
    printf("selected patch: %d, %s\n", selected_patch, patches[selected_patch]);

    
    if (!strcmp(patches[selected_patch], "Reload")){
        printf("RELOADING !!!!!");
        getPatchList();
    }
    
 
    if (!strcmp(patches[selected_patch], "Shutdown")){
        printf("SHUTTING DOWN !!!!!");
        sprintf(cmd, "shutdown -h now");
        system(cmd);
    }
    
    if (selected_patch >= 8) { 
        // check for x
        if(system("/root/check-for-x.sh")){
            printf("starting in GUI mode");
            sprintf(cmd, "/usr/bin/pd -rt /mnt/usbdrive/patches/mother.pd /mnt/usbdrive/patches/%s/main.pd &", patches[selected_patch]);
        }
        else {
            printf("starting in NON GUI mode");
            sprintf(cmd, "/usr/bin/pd -rt -nogui /mnt/usbdrive/patches/mother.pd /mnt/usbdrive/patches/%s/main.pd &", patches[selected_patch]);
        }

        //sprintf(cmd, "/usr/bin/pd -rt -nogui /mnt/usbdrive/Mother_Linux/tester.pd /mnt/usbdrive/patches/%s/main.pd &", patches[selected_patch]);
        //sprintf(cmd, "/root/pd-0.46-6/bin/pd -jack -nogui -rt /mnt/usbdrive/Mother_Linux/tester.pd /mnt/usbdrive/patches/%s/main.pd &", patches[selected_patch]);

        // first kill any other PD
        system("killall pd");
        system(cmd);
    }
}

void PatchMenu::encoderRelease(void){

}



void PatchMenu::drawPatchList(OledScreen &screen){
    char line[256];
    int i;
    for (i=0; i<5; i++) {
        sprintf(line, "%s", patches[i + patchlist_offset]);
        screen.setLine(i + 1, line);
    }

    screen.invertLine(curser_offset);   

}

void PatchMenu::getPatchList(void){


    // find patches
    struct dirent **namelist;
    int n;
    int i;
  
    // clear em out
    for (i = 0; i < 127; i++){
        strcpy(patches[i], "");
    }

    // inititial patches
    num_patches = 0;
    strcpy(patches[0], "");
    num_patches++;
    strcpy(patches[1], "");
    num_patches++;
    strcpy(patches[2], "--- SYSTEM ---");
    num_patches++;

    strcpy(patches[3], "Reload");
    num_patches++;
    strcpy(patches[4], "Shutdown");
    num_patches++;
    strcpy(patches[5], "");
    num_patches++;
    strcpy(patches[6], "");
    num_patches++;
    strcpy(patches[7], "--- PATCHES ---");
    num_patches++;


    // set locale so sorting happens in right order
    setlocale(LC_ALL, "en_US.UTF-8");

    //n = scandir("/home/debian/Desktop/patches", &namelist, NULL, alphasort);
    n = scandir("/mnt/usbdrive/patches", &namelist, NULL, alphasort);
    if (n<0)
        perror("scandir");
    else {

       while (n--) {
            if (namelist[n]->d_type == DT_DIR && strcmp (namelist[n]->d_name, "..") != 0 && strcmp (namelist[n]->d_name, ".") != 0) {
                strcpy(patches[num_patches], namelist[n]->d_name);
                num_patches++;
                num_patches &= 0x7f;  // 128 max num patches
            }
            //printf("%s\n", namelist[n]->d_name);
            free(namelist[n]);
        }
        free(namelist);
    }
    for (i=0; i<num_patches; i++) {
        printf("patch[%d]: %s\n", i, patches[i]);
    }
    
   
}



