#include <Arduino.h>

void setup();
void initLeds();
void setAllLedsLow();
void setRedHigh();
void loop();
void lightUp(int count);
#line 1 "src/sketch.ino"

int pwm1 = {3, 5, 6};
int pwn2 = {9, 10, 11};

int plex = {2, 4, 7};

int count;

void setup()
{
	initLeds();
	setAllLedsLow();
	setRedHigh();
}

void initLeds()
{
	for(int i = 0; i < 3; i++) {
		pinMode(pwm1[i], OUTPUT);
		pinMode(pwm2[i], OUTPUT);
		pinMode(plex[i], OUTPUT);
	}
}

void setAllLedsLow()
{
	for(int i = 0; i < 3; i++) {
		analogWrite(pwm1[i], 0);
		analogWrite(pwm2[i], 0);
		digitalWrite(plex[i], LOW);
	}
}

void setRedHigh()
{
	analogWrite(pwm1[0], 200);
	analogWrite(pwm2[0], 200);
}

void loop()
{
	lightUp(count)	
	count = count + 1 % 8;
	delay(1000);
}

void lightUp(int count)
{
	for(int i = 0; i < 3; i++) 
		digitalWrite(plex[i], (count & 0b001 << i) == 0b001 << i ? HIGH : LOW);
}
