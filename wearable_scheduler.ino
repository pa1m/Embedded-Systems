//#include <U8g2lib.h>
#include <WiFiClientSecure.h>
#include <base64.h>

//WiFiClientSecure is a big library. It can take a bit of time to do that first compile

// Set up the oled object
//U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI oled(U8G2_R0, 5, 17, 16);

#define DELAY 1000
#define SAMPLE_FREQ 8000                           // Hz, telephone sample rate
#define SAMPLE_DURATION 3                         // duration of fixed sampling
#define NUM_SAMPLES SAMPLE_FREQ*SAMPLE_DURATION    // number of of samples
#define ENC_LEN (NUM_SAMPLES + 2 - ((NUM_SAMPLES + 2) % 3)) / 3 * 4  // Encoded length of clip

/* CONSTANTS */
//Prefix to POST request:
const String PREFIX = "{\"config\":{\"encoding\":\"MULAW\",\"sampleRateHertz\":8000,\"languageCode\": \"en-US\"}, \"audio\": {\"content\":\"";
const String SUFFIX = "\"}}"; //suffix to POST request
const int AUDIO_IN = A0; //pin where microphone is connected
const int BUTTON_PIN = 13; //pin where button is connected
const int capacitanceMaxima = 50;
const int serial_update_period = 20;
const String API_KEY = "e16abd3a89940c2fe0d984603e9d5cf8051c65e0";
//bb9db807e0deeb12b61cef9431b4d4c7fa8d4e94
//e16abd3a89940c2fe0d984603e9d5cf8051c65e0
//AIzaSyC2nT5F69sBBaldwhMkcf_nLxzpexAMslg

/* Global variables*/
int button_state; //used for containing button state and detecting edges
int old_button_state; //used for detecting button edges
unsigned long time_since_sample;      // used for microsecond timing
int serial_counter;
int mediaT0;
unsigned long timer;
String speech_data; //global used for collecting speech data
const char* ssid     = "Hotspot";     // your network SSID (name of wifi network)
const char* password = "plasticsurgery"; // your network password
const char*  server = "speech.google.com";  // Server URL

WiFiClientSecure client; //global WiFiClient Secure object

//Below is the ROOT Certificate for Google Speech API authentication (we're doing https so we need this)
//don't change this!!
const char* root_ca= \
"-----BEGIN CERTIFICATE-----\n" \
"MIIC+jCCAeKgAwIBAgIIVg8y7CT5Eh4wDQYJKoZIhvcNAQEFBQAwIDEeMBwGA1UE\n" \
"AxMVMTEzMDU2NTUyNDUxNDc1MzExODY2MB4XDTE4MTAyOTA1NDYzNloXDTI4MTAy\n" \
"NjA1NDYzNlowIDEeMBwGA1UEAxMVMTEzMDU2NTUyNDUxNDc1MzExODY2MIIBIjAN\n" \
"BgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAo9Mw7Ic9rOJwOePkNuMrqjmQ3DnK\n" \
"1KUDdXzzbrcQ5oI4sar3wXfYElVATbC/a/2UJytKTf5CYLCQL/g7mTVwATSR8KNS\n" \
"/5oJAbhzHNPPCq7pmBU/AQ+BBcgyqihKkRzvMzNXXvfvuJlnvriFsgQR4K8IzEzG\n" \
"RJ1QjLjpe8o70hq9Az+tJERRpZYzxyUIBg5iHRVW4y/J/h9GH3k/W5YsJKVAqrQ5\n" \
"sTibhmRt/5NENsjBT0816DnYdSexpGzv9uKdKzKSXUf9gb/kbLxdUGoe1/91x93R\n" \
"/SMpY9O+q2EGVdElIUl8v28NFAly/LmMOSehSbe0xk6hM/dJ20tCFY8AQQIDAQAB\n" \
"ozgwNjAMBgNVHRMBAf8EAjAAMA4GA1UdDwEB/wQEAwIHgDAWBgNVHSUBAf8EDDAK\n" \
"BggrBgEFBQcDAjANBgkqhkiG9w0BAQUFAAOCAQEAcOZTxx5v3y7xUMWpAPPj63g6\n" \
"b/4i0XOi8IJomYp9/SCVNh0EiE1z0iFfuzeDIYgMBDQl4cwhwf80ci59PcS6gwMs\n" \
"pb/+wAzIxgAqpKmL0Nqyxchhya2KaNdeVDzH9eCOZys2kgTFVpTSAm/OYV3Lwf8K\n" \
"E6BLg8+RknvbJjSs12jMV95OxcxagGrK561sLjW29T0Zn178ze9ts5r+bYAcS/NR\n" \
"dB304S4HDCtkCgV7faX9lbIKEIKfOQ48SkJJAxH5sT1pu0e4JqSDkoKru5B/lV7+\n" \
"OKWRzMX6WvmGPJRx6QUGBhJHFyZpaRsMBuNZuweGDov/d3kRj24O8pDek0iWHQ==\n" \
"-----END CERTIFICATE-----\n";


void setup() {
  Serial.begin(115200);               // Set up serial port
  speech_data.reserve(PREFIX.length()+ENC_LEN+SUFFIX.length());
  WiFi.begin(ssid,password); //attempt to connect to wifi 
  int count = 0; //count used for Wifi check times
  serial_counter = 0;
  timer = micros();
  while (WiFi.status() != WL_CONNECTED && count<6) {
    delay(1000);
    Serial.print(".");
    count++;
  }
  delay(2000);
  if (WiFi.isConnected()) { //if we connected then print our IP, Mac, and SSID we're on
    Serial.print("Connected to ");
    Serial.println(ssid);
    delay(500);
  } else { //if we failed to connect just ry again.
    Serial.println(WiFi.status());
    ESP.restart(); // restart the ESP
  }

//  oled.begin();
//  oled.setFont(u8g2_font_5x7_tf);  //set font on oled  
//  oled.setCursor(0,15);
//  oled.print("Ready. Press to Record");
//  oled.setCursor(0,30);
//  oled.print("See serial monitor");
//  oled.setCursor(0,45);
//  oled.print("for debugging information");
//  oled.sendBuffer();  
//  pinMode(BUTTON_PIN, INPUT_PULLUP);
  old_button_state = touchRead(BUTTON_PIN);
}

//main body of code
void loop() {
  button_state = touchRead(BUTTON_PIN);
  if (button_state<100 && button_state != old_button_state) {
    client.setCACert(root_ca);
    delay(200);
    Serial.println("listening...");
    //oled.clearBuffer();    //clear the screen contents
    //oled.drawStr(0,15,"listening...");
    //oled.sendBuffer();     // update the screen
    record_audio();
    Serial.println("sending...");    
    //oled.clearBuffer();    //clear the screen contents
    //oled.drawStr(0,15,"sending...");
    //oled.sendBuffer();     // update the screen
    Serial.print("\nStarting connection to server...");
    delay(300);
    bool conn = false;
    for (int i=0; i<10; i++){
      if (client.connect(server,443));{
        conn = true;
        break;
      }
      Serial.print(".");
      delay(300);
    }
    if (!conn){
        Serial.println("Connection failed!");
        return;
    }else {
      Serial.println("Connected to server!");
      // Make a HTTP request:
    delay(200);
    client.println("POST https://speech.googleapis.com/v1/speech:recognize?key="+API_KEY+"  HTTP/1.1");
    client.println("Host: speech.googleapis.com");
    client.println("Content-Type: application/json");
    client.println("Cache-Control: no-cache");
    client.println("Content-Length: " + String(speech_data.length()));
    client.print("\r\n");
    int len = speech_data.length();
    int ind = 0;
    int jump_size=3000;
    while (ind<len){
      delay(100);//experiment with this number!
      if (ind+jump_size<len) client.print(speech_data.substring(ind,ind+jump_size));
      else client.print(speech_data.substring(ind));
      ind+=jump_size;
    }
    //client.print("\r\n\r\n");
    unsigned long count = millis();
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      Serial.print(line);
      if (line == "\r") { //got header of response
        Serial.println("headers received");
        break;
      }
      if (millis()-count>4000) break;
    }
    Serial.println("Response...");
    count = millis();
    while (!client.available()) {
      delay(100);
      Serial.print(".");
      if (millis()-count>4000) break;
    }
    Serial.println();
    Serial.println("-----------");
    String op;
    while (client.available()) {
      op+=(char)client.read();
    }
    Serial.println(op);
    int trans_id = op.indexOf("transcript");
    int starto, endo;
    if (trans_id != -1){
      int foll_coll = op.indexOf(":",trans_id);
      starto = foll_coll+2; //starting index
      endo = op.indexOf("\"",starto+1); //ending index
      //oled.clearBuffer();    //clear the screen contents
      //oled.setCursor(0,15);
      //oled.print(op.substring(starto+1,endo));
      //oled.sendBuffer();     // update the screen 
      delay(2000);
    }
    Serial.println(op.substring(starto+1, endo));
    Serial.println("-----------");
    client.stop();
    Serial.println("done");
    }
  }
  old_button_state = button_state;
}

//function used to record audio at sample rate for a fixed nmber of samples
void record_audio() {
  int sample_num = 0;    // counter for samples
  int enc_index = PREFIX.length()-1;    // index counter for encoded samples
  float time_between_samples = 1000000/SAMPLE_FREQ;
  int value = 0;
  uint8_t raw_samples[3];   // 8-bit raw sample data array
  String enc_samples;     // encoded sample data array
  time_since_sample = micros();
  Serial.println(NUM_SAMPLES);
  speech_data = PREFIX;
  while (sample_num<NUM_SAMPLES) {   //read in NUM_SAMPLES worth of audio data
    value = analogRead(AUDIO_IN);  //make measurement
    raw_samples[sample_num%3] = mulaw_encode(value-995); //remove 1.0V offset (from 12 bit reading)
    sample_num++;
    if (sample_num%3 == 0) {
      speech_data+=base64::encode(raw_samples, 3);
    }

    // wait till next time to read
    while (micros()-time_since_sample <= time_between_samples); //wait...
    time_since_sample = micros();
  }
  speech_data += SUFFIX;
}

int8_t mulaw_encode(int16_t sample){
   const uint16_t MULAW_MAX = 0x1FFF;
   const uint16_t MULAW_BIAS = 33;
   uint16_t mask = 0x1000;
   uint8_t sign = 0;
   uint8_t position = 12;
   uint8_t lsb = 0;
   if (sample < 0)
   {
      sample = -sample;
      sign = 0x80;
   }
   sample += MULAW_BIAS;
   if (sample > MULAW_MAX)
   {
      sample = MULAW_MAX;
   }
   for (; ((sample & mask) != mask && position >= 5); mask >>= 1, position--)
        ;
   lsb = (sample >> (position - 4)) & 0x0f;
   return (~(sign | ((position - 5) << 4) | lsb));
}


char* parser(String transcript)
{
 // To get three 3 informations
}

void append_to_db(String when, String where, String what)
{
  // Append the three shits to the database.
}

void create_db()
{
  // To create a database.
}

// We need a look up table to obtain GPS coordinates of known places.

/* Do GPS shiz once every 10 or 15 mins.
 * Get current location.
 * Update ETA based on current location and places API
 * Do what we discussed. Based on ETA, Time of event and polling interval, current time. Decision is reminder or no reminder.
 */
