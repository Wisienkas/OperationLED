#include <Process.h>

// Analog pin the microphone uses
const int SENSOR_PIN = A3;

int SELECTOR_COUNT = 3;
int SELECTOR_PINS[] = { 2, 4, 7 };

int PWM_PINS[2][3] = { { 3, 5, 6}, { 9, 10, 11} };


int color_array[8][2][3] = {
    { { 255, 0, 0 }, { 255, 255, 255 } },
    { { 255, 255, 255 }, { 255, 255, 255 } }, 
    { { 255, 255, 255 }, { 255, 255, 255 } },
    { { 255, 255, 255 }, { 255, 255, 255 } },
    { { 255, 255, 255 }, { 255, 255, 255 } },
    { { 255, 255, 255 }, { 255, 255, 255 } },
    { { 255, 255, 255 }, { 255, 255, 255 } },
    { { 255, 255, 255 }, { 255, 255, 255 } } 
  };
 

// there are 3 colors, R G B
const int COLOR_COUNT = 3;

// Number of LED's that are used
const int LED_COUNT = 16;
// Number of PWM sets that are used (3 PWM's per set)
const int PWM_COUNT = 2;

// the more PWM sets used the more LED's 
// can be changed at the same time
const int LOOP_SIZE = LED_COUNT/PWM_COUNT;

// Maximum analog value is 1023
// The microphones max is 704
const int MAX_SENSOR = 704;

// Gets filled in approximately 1 second
const int SENSOR_ELEMENTS_SIZE = 58;
// Gets filled in approximately 1 minute
const int BUFFER_SIZE = 60;

// number of reads that are in sensor_avg
byte sensor_elements = 0;
// sensor_elements == SENSOR_ELEMENTS_SIZE, 
// sensor_avg/sensor_elements will be the 
// average for a second
float sensor_avg = 0;

// the index that is free
byte bufferIndex = 0;
// this buffer contains each sensor_avg reading, one per second
float buffer[BUFFER_SIZE];

// used to serial output
long lastMillis = 0;
long lastWhole = 0;

// it is assumed that a server process is running already 
// this is reused across runs
Process pClient;

void setup() 
{
  Bridge.begin();
  Serial.begin(9600);
  
  for(int i = 0; i < COLOR_COUNT; i++)
  {
    pinMode(SELECTOR_PINS[i], OUTPUT);

    for (int j = 0; j < PWM_COUNT;j++)
    {
      pinMode(PWM_PINS[i][j], OUTPUT); 
    }
  }
  
  lastMillis = millis();
}

void loop() 
{
  float avg = processInputOutput();
  addToBuffers(avg);
  
  // only display once per second
  if (sensor_elements == 0)
  {
  calculateColors();
    // lastWhole is the number of seconds a complete buffer 0-60 took
    Serial.println("Second: " + String(bufferIndex, DEC) + ", avg: " + String(buffer[bufferIndex-1], DEC) + ", seconds: " + String(lastWhole, DEC));
  }
  
  // once the buffer is full, start sending data to local client
  if (bufferIndex >= BUFFER_SIZE)
  {
    lastWhole = millis() - lastMillis;
    
    bool clientRunning = pClient.running();
    
    Serial.print("Client running: ");
    Serial.println(clientRunning == true ? "true" : "false");
    
    // if the process is already running, this skips it in this case and resets the buffer
    if (clientRunning == false && 1 == 2)
    {      
      pClient.begin("/usr/bin/hwclient");
      
      for (int i = 0; i < bufferIndex; i++)
      {
        // each seconds reading is added as parameter
        pClient.addParameter(String(buffer[i], DEC));
      }
      
      // runs the client while the sketch continues
      pClient.runAsynchronously();
    }
    
    lastMillis = millis();
    bufferIndex = 0;
  }
}

float oldReading = 0;

// Reads the output from the microphone every 2 ms
// This is also where code to update the LED's will be placed later
// returns the average sensor value over LOOP_SIZE * 2 ms
float processInputOutput()
{
  float tmp = 0;
  
  
  for (int row = 0; row < LOOP_SIZE; row++) 
  {
    for (int pwm_i = 0; pwm_i < PWM_COUNT; pwm_i++)
    {
      for (int color = 0; color < COLOR_COUNT; color++)
      {
        analogWrite(PWM_PINS[pwm_i][color], 0);
      }
    } 
    for (int selector = 0; selector < SELECTOR_COUNT; selector++)
    {
      digitalWrite(SELECTOR_PINS[selector], bitRead(row, selector));
    }
    
    // 0: left, 1: right
    for (int pwm_i = 0; pwm_i < PWM_COUNT; pwm_i++)
    {
      for (int color = 0; color < COLOR_COUNT; color++)
      {
        analogWrite(PWM_PINS[pwm_i][color], color_array[row][pwm_i][color]);
      }
    }
    
    //tmp += analogRead(SENSOR_PIN);
    tmp += random(0, 720);
    delayMicroseconds(1700);
 

}

  // average over LOOP_SIZE*2 ms
  oldReading = tmp / LOOP_SIZE;
 
  return oldReading;
}

// add the avg to the buffers
void addToBuffers(int avg)
{
  sensor_avg += avg;
  sensor_elements++;
  
  if (sensor_elements >= SENSOR_ELEMENTS_SIZE)
  {
    buffer[bufferIndex++] = (sensor_avg / sensor_elements);
    
    sensor_avg = sensor_elements = 0;
  }
}

int percentOf(int value, int maxValue)
{
  int val = (value*100)/maxValue;
  return (val > 100) ? 100 : ((val < 0) ? 0 : val);
}
 
void calculateColors()
{
  color_array[0][0][(int)oldReading % 3] = 250;
  color_array[0][0][(int)(oldReading + 1) % 3] = 0;
  color_array[0][0][(int)(oldReading + 1) % 3] = 0;
}
