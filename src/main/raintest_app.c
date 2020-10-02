/*
rain_getValue() return Y or N or X
if rain sensor give RAINING Signal, Rain_getValue() return 'Y'
if rain sensor give NOT RAINING Signal, Rain_getValue() return 'N'
if rain sensor give UNKNOWN Signal, Rain_getValue() return 'E'
*/

#include "raintest_app.h"

#define RAIN_PATH "/dev/raintest_dev" //device file path
#define BUFFER_LENGTH 4

int rain_fd; //file descriptor for raindrop device file.
char msg[BUFFER_LENGTH]; //string from device file.
char status; //return raindrop sensor value to main . 

char rain_getValue() {
  int ret; //for write() read() operation
  
  rain_fd = open(RAIN_PATH, O_RDWR); //open device file O_RDWR
  if(rain_fd < 0) {
    perror("RAIN open() ERROR\n");
    return errno;
  }
  printf("Reading from device file of raindrop sensor\n");

  ret = write(rain_fd, "SIGNAL", strlen("SIGNAL")); //Write any string to Device file so device file start taking raindrop sensor value
  if (ret < 0 ) {
    perror("FAIL TO READ THE CONTENTS\n");
    close(rain_fd);
    return errno;
  }

  ret = read(rain_fd, msg, BUFFER_LENGTH); //Read device file content. device file has raindrop sensor value Y or N
  if(ret < 0 ) {
    perror("FAIL TO READ THE CONTENTS\n");
    close(rain_fd);
    return errno;
  }

  if(!strcmp(msg, "Y")) {  //there is water on raindrop sensor.
    status = 'Y';
    printf("status : RAINING\n");
  }
  else (!strcmp(msg, "N")) { //there is no water on raindrop sensor.
    status = 'N';
    printf("status : NOT RAINING\n");
  }

  close(rain_fd);
  
  return status;
}

