
int pwm1[] = {3, 5, 6};
int pwm2[] = {9, 10, 11};

int plex[] = {2, 4, 7};

int count = 0;

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
	analogWrite(pwm1[0], 250);
	analogWrite(pwm2[0], 250);
}

void loop()
{
	lightUp(count);
	count = count + 1 % 8;
	delayMicroseconds(1700);
}

void lightUp(int count)
{
	for(int i = 0; i < 3; i++) {
		digitalWrite(plex[i], (count & 0b001 << i) == 0b001 << i ? HIGH : LOW);
		analogWrite(pwm1[i], (count & 0b001 << i) == 0b001 << i ? 200 : 0); 
		analogWrite(pwm1[i], (count & 0b100 >> i) == 0b100 >> i ? 200 : 0); 
    }
}
