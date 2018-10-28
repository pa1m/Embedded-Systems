#include <base64.h>

#define DELAY 1000
#define SAMPLE_FREQ 8000                           
#define SAMPLE_DURATION 3                         
#define NUM_SAMPLES SAMPLE_FREQ*SAMPLE_DURATION    
#define ENC_LEN (NUM_SAMPLES + 2 - ((NUM_SAMPLES + 2) % 3)) / 3 * 4  

const int touchPin = 13;
const int audioIn = A0;

const int serial_update_period = 20;
const int capacitanceMaxima = 50;

int serial_counter;
int mediaT0;

String speech_data;

unsigned long timer;
int buttonState;  
int oldButtonState;
unsigned long time_since_sample;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  serial_counter = 0;
  timer = micros();

  Serial.println("Starting");
  oldButtonState = touchRead(touchPin);
}

void loop() {
  // put your main code here, to run repeatedly:
  buttonState = touchRead(touchPin);
  Serial.println(buttonState, oldButtonState);
  if(buttonState < 100 && buttonState != oldButtonState)
  {
    Serial.println("Listening");
    record_audio();   
  }
  oldButtonState = buttonState;
}

void record_audio() {
  int sampleNum = 0;      
  float samplePeriod = 1000000/SAMPLE_FREQ;
  int value = 0;
  uint8_t raw_samples[3];  
  time_since_sample = micros();
  Serial.println(NUM_SAMPLES);
  while (sampleNum < NUM_SAMPLES) {   
    value = analogRead(audioIn); 
    raw_samples[sampleNum % 3] = mulaw_encode(value - 993); 
    sampleNum++;
    
    if (sampleNum % 3 == 0) {
      speech_data += base64::encode(raw_samples, 3);
    }

    while (micros()-time_since_sample <= samplePeriod);
      time_since_sample = micros();
  }
  Serial.println(speech_data);
}

int8_t mulaw_encode(int16_t sample) 
{
  int mu = 255;
  
  double abs_y = double(abs(sample)) / 32768;
  double z = log(1 + mu*abs_y) / log(1+mu);
  if (sample < 0) {
    z *= -1;
  }
  return (int8_t) (z*128);
}
