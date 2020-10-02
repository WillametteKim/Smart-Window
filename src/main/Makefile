all: main.o dust.o raintest_app.o servotest_app.o dht11_app.o
	gcc -o main main.o dust.o raintest_app.o servotest_app.o dht11_app.o -lwiringPi -lpthread -lwiringPiDev

main.o: main.c
	gcc -c main.c

dust.o: dust.c dust.h
	gcc -c dust.c

raintest_app.o: raintest_app.c raintest_app.h
	gcc -c raintest_app.c

servotest_app.o: servotest_app.c servotest_app.h
	gcc -c servotest_app.c

dht11_app.o: dht11_app.c dht11_app.h
	gcc -c dht11_app.c
	
clean:
	rm *.o
