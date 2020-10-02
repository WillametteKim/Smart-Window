/*
Ajou Univ., Dept of Software, Hyunsoo Kim
2018. 06. 06
WARN NEVER DECLARE CONST IN HEADER FILE, DO THAT ONLY VARIABLES, SEE MORE https://stackoverflow.com/questions/9846920/define-array-in-c
*/

#include "dust.h"

#define DUST_BUFLEN 8
#define DUST_FRAMELEN 32
#define DUST_CMDLEN 7
#define DUST_CMDANSLEN 8

const unsigned char cmdSetPassive [DUST_CMDLEN]  = {0x42, 0x4D, 0xE1, 0x00, 0x00, 0x01, 0x70};  
const unsigned char cmdWakeUp[DUST_CMDLEN]       = {0x42, 0x4d, 0xe4, 0x00, 0x01, 0x01, 0x74};  //expecting NO ANS
const unsigned char cmdPassiveRead[DUST_CMDLEN]  = {0x42, 0x4d, 0xe2, 0x00, 0x00, 0x01, 0x71};  //expecting whole frame
const unsigned char cmdSleep[DUST_CMDLEN]        = {0x42, 0x4d, 0xe4, 0x00, 0x00, 0x01, 0x73};  
const unsigned char cmdSleep_ans[DUST_CMDANSLEN] = {0x42, 0x4d, 0x00, 0x04, 0xe4, 0x00, 0x01, 0x77}; //expecting 42 4D 00 04 E4 00 01 77 
const char preamble1 = 'B'; //0x42 in ASCII
const char preamble2 = 'M'; //0x4d in ASCII
const char *dust_portname = "/dev/ttyAMA0";

int set_interface_attribs(int fd, int speed)
{
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
        printf("Error from tcgetattr: %s\n", strerror(errno));
        return -1;
    }

    cfsetospeed(&tty, (speed_t)speed);
    cfsetispeed(&tty, (speed_t)speed);

    tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;         /* 8-bit characters */
    tty.c_cflag &= ~PARENB;     /* no parity bit */
    tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
    tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

    /* setup for non-canonical mode */
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_oflag &= ~OPOST;

    /* fetch bytes as they become available */
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 1;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        printf("Error from tcsetattr: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}


void set_mincount(int fd, int mcount)
{
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
        printf("Error tcgetattr: %s\n", strerror(errno));
        return;
    }

    tty.c_cc[VMIN] = mcount ? 1 : 0;
    tty.c_cc[VTIME] = 5;        /* half second timer */

    if (tcsetattr(fd, TCSANOW, &tty) < 0)
        printf("Error tcsetattr: %s\n", strerror(errno));
}


//SET SENSOR TO PASSIVE MODE
void set_passive(int fd)
{
    int wrlen, rdlen;
    int tryCount = 0;
    unsigned char buf[DUST_BUFLEN +1];
    wrlen = write(fd, cmdSetPassive, DUST_CMDLEN);
    if(wrlen < DUST_CMDLEN) printf("set_passive write err\n");

    return ;    
}


//WAKE UP SENSOR
void wakeup_sensor(int fd)
{
    int retval;
    int tryCount = 0;

    while(1){
        if((retval = write(fd, cmdWakeUp, DUST_CMDLEN)) != DUST_CMDLEN) {
            tryCount++;
            printf("Err from wakeup_sensor(), tried %d times\n", tryCount); 
        }

        else 
            break;
    }
    return ;    
}


int sendReadCmd(int fd)
{
    int retval;

    if((retval = write(fd, cmdPassiveRead, DUST_CMDLEN)) != DUST_CMDLEN) {
        printf("Err from sendReadCmd, failed to send READ CMD\n");
    }
    return retval;    
}


//SYNC WITH PREAMBLE, READ DATA
int sync_read(int fd, unsigned char *frame)
{
    unsigned char buf[DUST_BUFLEN +1];
    int rdlen;
    int count; //loop count variable
    unsigned char *p;

    //SYNC AND READ
    while(1) {
        rdlen = read(fd, buf, DUST_BUFLEN);

        //IF WE GOT SYNC
        if(rdlen != DUST_BUFLEN) continue;

        else if (buf[0] == preamble1 && buf[1] == preamble2) {
         
            count = 0;

            if(memcpy(&frame[count*DUST_BUFLEN],buf,DUST_BUFLEN) < 0) {
                printf("Error memcpy at buf blk %i: %s\n", count, strerror(errno));
                return -1;
            }
        }

        //IF WE READ REST OF THE FRAME
        else {
            count = count %4; count++; 

            if(memcpy(&frame[count*DUST_BUFLEN],buf,DUST_BUFLEN) < 0) {
                printf("Error memcpy at buf blk %i: %s\n", count, strerror(errno));
                return -1;
            }

            //NOW WE READ WHOLE FRAME
            if(count >= DUST_FRAMELEN/DUST_BUFLEN-1)
                return 0;
        }

    }//while
}


//VALIDATE FRAME AND PARSE
int check_frame(unsigned char *frame)
{
    //CALC CHECKSUM
    int sum = 0;
    for(int i = 0; i<DUST_FRAMELEN -2; i++)
        sum += frame[i];
    int checksum = frame[30] * pow(16,2) + frame[31]; //conversion between Hex<->Dec

    if(sum == checksum) {
        return 0;
    }
    
    else{
        printf("CHECKSUM ERR, FLUSH PREVIOUS DATA\n");
        return -1;
    }
}


//SLEEP SENSOR 
void sleep_sensor(int fd)
{
    int wrlen, rdlen;
    int tryCount = 0;
    unsigned char buf[DUST_BUFLEN +1];

    while(1) {
        wrlen = write(fd, cmdSleep, DUST_CMDLEN);
        
        if(wrlen != DUST_CMDLEN) {
            tryCount++;
            printf("Err from set_passive(), tried %d times\n", tryCount); 
        }

        else {
            rdlen = read(fd, buf, DUST_CMDANSLEN);
            if(rdlen == DUST_CMDANSLEN && strncmp(buf, cmdSleep_ans, DUST_CMDLEN) == 0)
                break;
        }
    }//while
    return ;
}


void *robust_sensor_main(void* threadArgs) 
{
    int fd = ((struct PMS_Args *)threadArgs)->dust_fd;
    unsigned char frame[DUST_FRAMELEN +1];

    while(1) {
        char status;
        int pm2_5 = 0;
        int pm10  = 0;
        int statistic_loop=3;
        
        while(statistic_loop-- > 0) {
            while(1) {
                int retval;
                if( (retval = sendReadCmd(fd)) != DUST_CMDLEN)  {
                    printf("sendReadCmd: %d", retval);
                    continue;
                }
                if( (retval = sync_read(fd, frame)) != 0) {
                    printf("sync_read: %d", retval);
                    continue;
                }
                if( (retval = check_frame(frame)) != 0) {
                    printf("check_frame: %d", retval);
                    continue;
                }
                else break; //we acquired good frame!
            }//if err, continue;

            pm2_5 += frame[12]<<8 | frame[13];
            pm10 += frame[14]<<8 | frame[15];
        }//statistical loop for normalize data, read 3 frame and get mean val;
        
        sleep_sensor(fd);

        //okay with lose info about point
        pm2_5 = pm2_5 / 3;
        pm10 = pm10 / 3;

        if(pm2_5 <= 25 && pm10 <= 50) //air quality is Good or Bad 25 50
            status = 'G';
        else 
            status = 'B';

        ((struct PMS_Args *)threadArgs)->status = status;
        ((struct PMS_Args *)threadArgs)->pm2_5 = pm2_5;
        ((struct PMS_Args *)threadArgs)->pm10 = pm10;
        
        printf("from thre: %d\t%c\t%d\t%d\n", 
            ((struct PMS_Args *)threadArgs)->dust_fd, 
            ((struct PMS_Args *)threadArgs)->status, 
            ((struct PMS_Args *)threadArgs)->pm2_5,
            ((struct PMS_Args *)threadArgs)->pm10);



        wakeup_sensor(fd);
    }//while
}


int sensor_init() 
{
    int fd;
    int status;

    //OPEN DATA STREAM
    fd = open(dust_portname, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        printf("Error opening %s: %s\n", dust_portname, strerror(errno));
        return -1;
    }

    set_interface_attribs(fd, B9600);

    if(tcdrain(fd) != 0) {  //delay for output
        printf("Error at writing: %s\n", strerror(errno));
        return -1;
    }

    //SET PASSIVE MODE TO HIBERNATE
    set_passive(fd);
    
    return fd;
}
