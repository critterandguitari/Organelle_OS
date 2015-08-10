
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <locale.h>
#include <string.h>


#include "PatchMenu.h"


void PatchMenu::up(void) {
    if (!(patchlist_offset >= (num_patches - 1))) patchlist_offset++;
    printf("patch: %d\n", patchlist_offset);
}

void PatchMenu::down(void) {
    if (!(patchlist_offset < 1)) patchlist_offset--;
    printf("patch: %d\n", patchlist_offset);
}

void PatchMenu::drawPatchList(OledScreen &screen){
    char line[256];
    int i;
    for (i=0; i<5; i++) {
        sprintf(line, "%s", patches[i + patchlist_offset]);
        screen.setLine(i + 1, line);
    }
}

void PatchMenu::getPatchList(void){


    // find patches
    struct dirent **namelist;
    int n;
    int i;

    num_patches = 0;
    selected_patch = 0;
    patchlist_offset = 10;
   
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



