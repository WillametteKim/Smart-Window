#ifndef _DHT11_APP_H_
#define _DHT11_APP_H_

#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

int read_dhtValue(unsigned short int *dht_val);

#endif
