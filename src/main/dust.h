#ifndef __DUST_H_
#define __DUST_H_

#include <errno.h>
#include <fcntl.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <unistd.h>
#include <math.h>
#include <stdint.h>
#include <pthread.h>

int set_interface_attribs(int fd, int speed);
void set_mincount(int fd, int mcount);
void set_passive(int fd);					//set sensor to passive mode to reduce power consump
void wakeup_sensor(int fd);					//send msg to sensor to wake up, and wait 30sec to stabilize
int sendReadCmd(int fd);					//send msg to sensor to read data, return sndMsgBytes
int sync_read(int fd, unsigned char *frame);//sync with sensor and read, return 0 on success
int check_frame(unsigned char *frame);		//parse and calc checksum, return 0 on success
void sleep_sensor(int fd);					//send msg to sensor to sleep
void *robust_sensor_main(void* threadArgs);
int sensor_init();							//initiate sensor using set_interface_attribs

#define DUST_SLEEP 5
#define WAITTIME 1

struct PMS_Args {
    int dust_fd;
    char status;
    unsigned short int pm2_5;
    unsigned short int pm10;
};

#endif

/*
<PMS7003 SENSOR SPEC>
DATA RATE: 9600bps
TRANSMIT INTERVAL: 1sec
FRAME LENGTH: 32Bytes
ACTIVE CURRENT <= 100 milliAmps = 100,000 microAmps
STANDBY CURRENT <= 200 microAmps

------------------------------------------------
|PREAMBLE(2) | LENGTH(2) | DATA(26) | CHECK(2) |   (Bytes)
------------------------------------------------
PREAMBLE: "BM"
LENGTH  : 28Bytes = DATA + CHECK
DATA    : 13fields, each is 2Bytes
CHECK   : OK if (CHECK == SUM all the bytes except CHECK bytes)
MOREINFO: http://eleparts.co.kr/goods/view?no=4208690
          https://github.com/teusH/MySense/blob/master/docs/pms7003.md

<WHO fine dust criteria>
    PM 2.5    PM 10 
좋음 0~15     0~30
보통 16~25    31~50
나쁨 26~50    51~100
극한 51~      101~
*/
