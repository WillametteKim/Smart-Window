/*
dht11은 습도값과 온도값을 라즈베리 파이로 전달한다.
라즈베리 파이가 low 18ms, high 20~40ms의 신호를 보내면 dht11은 총 83 bit의 신호를 보낸다.
첫 3bit는 response, pullup readt, start to transmit 신호이며, 나머지 80 bit가 data에 해당한다.
*/

#include "dht11_app.h"

#define MAXTIMINGS 83 //first 3 bits(response, pullup ready, start to transmit)
                      //40 data bit * 2(low, high)
#define DHTPIN 7 //pin to connect

int read_dhtValue(unsigned short int *dht_val) {
    
    uint8_t laststate = HIGH ;
    uint8_t counter = 0 ;
    uint8_t j = 0, i = 0 ;
    uint8_t flag = HIGH ;
    uint8_t state = 0 ;
    float f ;
    int dht11_dat[5];//save value from dht11 module
    int humidity;
    int temperature;

    dht11_dat[0] = dht11_dat[1] = dht11_dat[2] = dht11_dat[3] = dht11_dat[4] = 0 ; //initialize

     if (wiringPiSetup() == -1)  {
         printf("DHT CONNECTION ERROR\n");
         dht_val[0] = 0;
         dht_val[1] = -1;
         return -1;
        }

    pinMode(DHTPIN, OUTPUT) ;
    digitalWrite(DHTPIN, LOW) ;
    delay(18) ;
    digitalWrite(DHTPIN, HIGH) ;
    delayMicroseconds(40) ; //Send Start signal to DHT11 module. low 18ms and high 20~40ms

    pinMode(DHTPIN, INPUT) ; //ready to read from DTH11 module

    for (i = 0; i < MAXTIMINGS; i++) {
        counter = 0 ;

        //read data every 1ms.
        while ( digitalRead(DHTPIN) == laststate) {
            counter++ ;
            delayMicroseconds(1) ;

            if (counter == 255)
            break ;
        }

            laststate = digitalRead(DHTPIN) ;

            if (counter == 255) 
            break ; // if while breaked by timer, break for

            //i % 2 == 0 means HIGH
            if ((i >= 4) && (i % 2 == 0)) { //ignore first 3 response, fully ready, start to transmit bit.
                dht11_dat[j / 8] <<= 1 ; 
                    if (counter > 50) 
                    dht11_dat[j / 8] |= 1 ; 
                j++ ;
        }
    }

    //parity check
    if ((j >= 40) && (dht11_dat[4] == ((dht11_dat[0] + dht11_dat[1] + dht11_dat[2] + dht11_dat[3]) & 0xFF))) {
        if(dht11_dat[2] > 100) { //dht11이 연결되지 않은 경우 굉장히 높은 온도를 측정하게 되는데 
          return -1;             //이를 필터링하기 위함.
        }
        else {
        dht_val[0] = dht11_dat[0];//humidity
        dht_val[1] = dht11_dat[2];//temperature
        return 0;
        }
    }
    else 
    return -1;
}
