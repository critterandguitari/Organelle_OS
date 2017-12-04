
    uint8_t serialIn[64];
    for (;;){
        len = serial.readBuffer(serialIn, 64);
        if (len == -1) {
            //    printf("Error reading from serial port\n");
        }
        else if (len == 0) {
            //printf("No more data\n");
        }
        else {
            for (i = 0;  i < len; i++) {
                printf("%d \n", serialIn[i]);
            }
        }
        usleep(100);
 
    }


