/*
Servo_setValue() give a instruction to servo motor file
Main function give a string OPEN or CLOS.
if Servo_setValue() take "OPEN", servo motor will open the window.
if Servo_setValue() take "CLOS", servo motor will close the window.
*/

#include "servotest_app.h"

#define SERVO_PATH "/dev/servotest_dev"

int servo_fd; //file descriptor for servo motor device file.
char string[10]; //save instruction from main function().

int servo_setValue(char *input) {
    int ret;

    strcpy(string, input); //copy instruction to string[]

    servo_fd = open(SERVO_PATH, O_RDWR|O_NONBLOCK); //open servo motor device file
    if(servo_fd < 0){
        perror("SERVO open() ERROR\n");
        close(servo_fd);
        return errno;
    }
    
    //OPENING
    if(!strcmp(string, "OPEN"))
        ret = write(servo_fd, "OPEN", strlen("OPEN"));
    //CLOSING
    if(!strcmp(string, "CLOS"))
        ret = write(servo_fd, "CLOS", strlen("CLOS"));

    close(servo_fd);

    return 0;
}




