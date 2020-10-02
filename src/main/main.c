/*
Main program take value from sensors and give instruction to servo motor.
레인테스트 return Y(비옴) N(비안옴) E(에러)
서보테스트는 모듈에 OPEN or CLOS 신호 전달
SERVO PIN 13
RAIN PIN 15
DHT PIN 7
*/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <wiringPi.h>
#include "raintest_app.h"
#include "servotest_app.h"
#include "dust.h"
#include "dht11_app.h"

#define BUFFER_LENGTH 10
#define SLEEPTIME 5

//LCD connetion
#define LCD_RS 11
#define LCD_E 10
#define LCD_D4 6
#define LCD_D5 5
#define LCD_D6 4
#define LCD_D7 1

int main() 
{
  int ret;
  
  /*Initialize Dust sensor*/
  pthread_t dust_thread;
  int dust_thread_id;
  struct PMS_Args forThread;
  struct PMS_Args *threadArgs = &forThread;

  threadArgs->dust_fd = sensor_init();
  if(threadArgs->dust_fd < 0) 
    printf("sensor init err\n");
  threadArgs->status = 'G';
  threadArgs->pm2_5 = 50;
  threadArgs->pm10 = 50;    

  dust_thread_id = pthread_create(&dust_thread, NULL, robust_sensor_main, (void *)threadArgs);
  if(dust_thread_id < 0)
    printf("thread create err\n");

  ret = pthread_detach(dust_thread);
  switch(ret){
  case 0 : break;
  case ESRCH: printf("no thread id match\n"); break;
  case EINVAL: printf("already detached\n"); break;
  }

  //Initialize variable
  unsigned short int dht_val[2];  //temperature and humidity sensor value
  char rain = 'N';
  char status_before = 'C'; //save prior status. it's 'C'LOSE
  char status_now;
  char lcd_string[32];
  int lcd_fd;

  dht_val[0] = 0;
  dht_val[1] = 0;

  /*initialize LCD*/ 
  wiringPiSetup();

  lcd_fd = lcdInit(2, 16, 4, LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7, 0, 0, 0, 0);

  sleep(2);//wait for dust sensor value

  printf("SMART WINDOW ON\n");

//=======================================================================================================

  while(1) {
    //read dht_sensor
    for(int i = 0 ; i < 3 ; i++) {
      ret = read_dhtValue(dht_val) ; //get dht value
      sleep(1);
    }

    //read rain_sensor
    rain = rain_getValue();

    //Set LCD text
    sprintf(lcd_string, "PM2.5:%2d|PM10:%2dTemp:%2dC  RAIN:%c", threadArgs->pm2_5, threadArgs->pm10, dht_val[1], rain);
    lcdPosition(lcd_fd,0,0);
    lcdPuts(lcd_fd, lcd_string);   

    //Not Rain && Good airquality && Not Cold temperature => 'O'PEN THE WINDOWS
    if(rain == 'N' && threadArgs->status == 'G' && dht_val[1] > 10) {
      status_now = 'O';
    }
  
    else {
      status_now = 'C';
    }

    //compare status_now and status_before
  if(status_now == status_before) {
    printf("Rain: %c\tAir Conditional: %c\tHumidity: %d\tTemperature: %d\n", rain, threadArgs->status, dht_val[0], dht_val[1]);
    printf("NO OPERATION\n");
  }

    //if status is changed from before, control the servo motor 
    else {
      if(status_now == 'O') {//OPENING
        printf("Rain: %c\tAir Conditional: %c\tHumidity: %d\tTemperature: %d\n", rain, threadArgs->status, dht_val[0], dht_val[1]);
        printf("OPENING WINDOW!\n");
        status_before = status_now;
        ret = servo_setValue("OPEN");
        if(ret != 0)
          printf("SERVO MOTOR ERROR\n");
    
      }//inner if

      else { //CLOSING
        printf("Rain: %c\tAir Conditional: %c\tHumidity: %d\tTemperature: %d\n", rain, threadArgs->status, dht_val[0], dht_val[1]);
        printf("CLOSING WINDOW!\n");
        status_before = status_now;
        ret = servo_setValue("CLOS");
        if( ret != 0) {
        printf("SERVO MOTOR ERROR\n");
       }
      }//else
    }//outer if

  sleep(SLEEPTIME); //waiting time
}//while

return 0;
}


