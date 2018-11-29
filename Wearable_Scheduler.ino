// TODO
// Get the real time to make decision - parse_db
// get_GPS()

#include <Wire.h>
#include "SSD1306.h"
#include <WiFiClientSecure.h>
#include <base64.h>
#include <string.h>
#include <TimeLib.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>

#define MAX_NUM_OF_ENTRIES 20
#define VELOCITY_CONSTANT 0.2
#define POLL_TIME 2

String Places[11] = {"department", "hostel", "store", "beach", "temple", "auditorium", "ground", "mess", "bus", "library","class"};
double LATITUDE[11] = {13.011026, 13.007889, 13.012758, 13.009666, 13.003948, 13.008837, 13.009846, 13.007393, 13.011632, 13.009867, 13.010227};
double LONGITUDE[11] = {74.792558, 74.794571, 74.796541, 74.788786, 74.790696, 74.795663, 74.797973, 74.795131, 74.793013, 74.794223, 74.792716};
int no_of_places = 11;
String Works[8] = {"date", "meeting", "lecture", "eat", "work", "shop", "buy", "go"};
int no_of_works = 8;

struct memoryLocation
{
  boolean flag;
  double lat;
  double lon;
  long hr;
  long mn;
  String what;
  String where;
} memLoc[MAX_NUM_OF_ENTRIES];

//WiFiClientSecure is a big library. It can take a bit of time to do that first compile

// Set up the oled object
SSD1306 display(0x3c, 21, 22); 
HardwareSerial GPS(1);
TinyGPSPlus gps;

#define DELAY 1000
#define SAMPLE_FREQ 8000                                             // Hz, telephone sample rate
#define SAMPLE_DURATION 7                                            // duration of fixed sampling
#define NUM_SAMPLES SAMPLE_FREQ*SAMPLE_DURATION                      // number of of samples
#define ENC_LEN (NUM_SAMPLES + 2 - ((NUM_SAMPLES + 2) % 3)) / 3 * 4  // Encoded length of clip

/* CONSTANTS */
//Prefix to POST request:
const String PREFIX = "{\"config\":{\"encoding\":\"MULAW\",\"sampleRateHertz\":8000,\"languageCode\": \"en-US\"}, \"audio\": {\"content\":\"";
const String SUFFIX = "\"}}";                                        // suffix to POST request
const int AUDIO_IN = A0;                                             // pin where microphone is connected
const int BUTTON_PIN = 13;                                            // pin where button is connected
const int capacitanceMaxima = 50;
const int serial_update_period = 20;
const String API_KEY = "AI***********************************ts";

//API keys
//AI***********************************ts  

/* Global variables*/
int button_state;                                                    // used for containing button state and detecting edges
int old_button_state;                                                // used for detecting button edges
unsigned long time_since_sample;                                     // used for microsecond timing
int serial_counter;                                                  //     
int mediaT0;
unsigned long timer;
String speech_data;                                                  // global used for collecting speech data
const char* ssid     = "Samarth";                              // your network SSID (name of wifi network)
const char* password = "1234567890";                                 // your network password
//const char* ssid     = "airtel_9E624A";                              // your network SSID (name of wifi network)
//const char* password = "JSFFDJ5CJj";                                 // your network password
//const char* ssid     = "Hotspot";                              // your network SSID (name of wifi network)
//const char* password = "plasticsurgery";                                 // your network password
const char*  server = "speech.googleapis.com";                       // Server URL
unsigned long parse_watch;
double originLat = 74.788786;
double originLon = 13.009666;

WiFiClientSecure client;                                             //global WiFiClient Secure object

//Below is the ROOT Certificate for Google Speech API authentication (we're doing https so we need this)
//don't change this!!
const char* root_ca= \
"-----BEGIN CERTIFICATE-----\n"\
"MIIEXDCCA0SgAwIBAgINAeOpMBz8cgY4P5pTHTANBgkqhkiG9w0BAQsFADBMMSAw\n"\
"HgYDVQQLExdHbG9iYWxTaWduIFJvb3QgQ0EgLSBSMjETMBEGA1UEChMKR2xvYmFs\n"\
"U2lnbjETMBEGA1UEAxMKR2xvYmFsU2lnbjAeFw0xNzA2MTUwMDAwNDJaFw0yMTEy\n"\
"MTUwMDAwNDJaMFQxCzAJBgNVBAYTAlVTMR4wHAYDVQQKExVHb29nbGUgVHJ1c3Qg\n"\
"U2VydmljZXMxJTAjBgNVBAMTHEdvb2dsZSBJbnRlcm5ldCBBdXRob3JpdHkgRzMw\n"\
"ggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDKUkvqHv/OJGuo2nIYaNVW\n"\
"XQ5IWi01CXZaz6TIHLGp/lOJ+600/4hbn7vn6AAB3DVzdQOts7G5pH0rJnnOFUAK\n"\
"71G4nzKMfHCGUksW/mona+Y2emJQ2N+aicwJKetPKRSIgAuPOB6Aahh8Hb2XO3h9\n"\
"RUk2T0HNouB2VzxoMXlkyW7XUR5mw6JkLHnA52XDVoRTWkNty5oCINLvGmnRsJ1z\n"\
"ouAqYGVQMc/7sy+/EYhALrVJEA8KbtyX+r8snwU5C1hUrwaW6MWOARa8qBpNQcWT\n"\
"kaIeoYvy/sGIJEmjR0vFEwHdp1cSaWIr6/4g72n7OqXwfinu7ZYW97EfoOSQJeAz\n"\
"AgMBAAGjggEzMIIBLzAOBgNVHQ8BAf8EBAMCAYYwHQYDVR0lBBYwFAYIKwYBBQUH\n"\
"AwEGCCsGAQUFBwMCMBIGA1UdEwEB/wQIMAYBAf8CAQAwHQYDVR0OBBYEFHfCuFCa\n"\
"Z3Z2sS3ChtCDoH6mfrpLMB8GA1UdIwQYMBaAFJviB1dnHB7AagbeWbSaLd/cGYYu\n"\
"MDUGCCsGAQUFBwEBBCkwJzAlBggrBgEFBQcwAYYZaHR0cDovL29jc3AucGtpLmdv\n"\
"b2cvZ3NyMjAyBgNVHR8EKzApMCegJaAjhiFodHRwOi8vY3JsLnBraS5nb29nL2dz\n"\
"cjIvZ3NyMi5jcmwwPwYDVR0gBDgwNjA0BgZngQwBAgIwKjAoBggrBgEFBQcCARYc\n"\
"aHR0cHM6Ly9wa2kuZ29vZy9yZXBvc2l0b3J5LzANBgkqhkiG9w0BAQsFAAOCAQEA\n"\
"HLeJluRT7bvs26gyAZ8so81trUISd7O45skDUmAge1cnxhG1P2cNmSxbWsoiCt2e\n"\
"ux9LSD+PAj2LIYRFHW31/6xoic1k4tbWXkDCjir37xTTNqRAMPUyFRWSdvt+nlPq\n"\
"wnb8Oa2I/maSJukcxDjNSfpDh/Bd1lZNgdd/8cLdsE3+wypufJ9uXO1iQpnh9zbu\n"\
"FIwsIONGl1p3A8CgxkqI/UAih3JaGOqcpcdaCIzkBaR9uYQ1X4k2Vg5APRLouzVy\n"\
"7a8IVk6wuy6pm+T7HT4LY8ibS5FEZlfAFLSW8NwsVz9SBK2Vqn1N0PIMn5xA6NZV\n"\
"c7o835DLAFshEWfC7TIe3g==\n"\
"-----END CERTIFICATE-----\n";

void setup() {
  
  Serial.begin(115200);                                                 // Set up serial port
  GPS.begin(19200, SERIAL_8N1, 16, 17);
  while(!Serial);
  for(int i=0; i< MAX_NUM_OF_ENTRIES; i++)
    memLoc[i].flag = 0;
    
  speech_data.reserve(PREFIX.length()+ENC_LEN+SUFFIX.length());
  WiFi.begin(ssid,password);                                            // attempt to connect to wifi 
  int count = 0;                                                        //count used for Wifi check times
  serial_counter = 0;
  timer = micros();
  while (WiFi.status() != WL_CONNECTED && count<6)
  {
    delay(1000);
    Serial.print(".");
    count++;
  }
  delay(2000);
  if (WiFi.isConnected()) 
  { 
    //if we connected then print our IP, Mac, and SSID we're on
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP Address ");
    Serial.println(WiFi.localIP());
    delay(500);
  } 
  else 
  { 
    //if we failed to connect just ry again.
    Serial.println(WiFi.status());
    ESP.restart();                                                     // restart the ESP
  }

  display.init();
  display.setFont(ArialMT_Plain_10);                                  // set font on oled 
  display.drawString(0, 0, "Look at the Serial Monitor");
  display.drawString(0, 20, "for Debugging Infromation");  
  display.display();
  delay(5000);
  display.clear();
  display.drawString(0, 0, "Ready. Touch to Record");
  display.display();
  delay(5000); 
  //pinMode(BUTTON_PIN, INPUT_PULLUP);
  old_button_state = touchRead(BUTTON_PIN);
  parse_watch=millis();
  Serial.println("Setup done");
}

//main body of code
void loop() 
{  
  int flag_button_pressed = 0;
  button_state = touchRead(BUTTON_PIN);
  if (button_state < 100 && button_state != old_button_state)
  {
    flag_button_pressed = 1;
    client.setCACert(root_ca);
    delay(200);
    display.clear();
    Serial.println("Listening...");
    display.drawString(0, 0, "Listening...");
    display.display();     
    record_audio();
    Serial.println("Sending...");
    display.clear();    
    display.drawString(0,0,"Sending...");
    display.display();
    Serial.print("\nStarting connection to server...");
    delay(300);
    bool conn = false;
    for (int i=0; i<10; i++)
    {
      if (client.connect(server,443))
      {
        conn = true;
        break;
      }
      Serial.print(".");
      delay(300);
    }
    if (!conn)
    {
        Serial.println("Connection failed!");
        return;
    }
    else 
    {
      Serial.println("Connected to server!");
      int len = speech_data.length();
      int ind = 0;
      int jump_size=3000;
      
      // Make a HTTP request:
      delay(200);
      client.println("POST https://speech.googleapis.com/v1/speech:recognize?key="+API_KEY+"  HTTP/1.1");
      client.println("Host: speech.googleapis.com");
      client.println("Content-Type: application/json");
      client.println("Cache-Control: no-cache");
      //client.println("Connection: close");
      client.println("Content-Length: " + String(speech_data.length()));
      client.print("\r\n");
      while (ind<len)
      {
        delay(100);
        if (ind+jump_size<len) 
          client.print(speech_data.substring(ind,ind+jump_size));
        else 
          client.print(speech_data.substring(ind));
        ind+=jump_size;
      }
      client.print("\r\n\r\n");
      unsigned long count = millis();
      // Wait for same duration as the number of samples
      // delay((SAMPLE_DURATION+1)*1000);
      while (client.connected()) 
      {
        String line = client.readStringUntil('\n');
        Serial.println(line);
        if (line == "\r")
        { 
          //got header of response
          Serial.println("headers received");
          break;
        }
        if (millis()-count>4000) 
          break;
      }
      Serial.println("Response...");
      count = millis();
      while (!client.available()) 
      {
        delay(100);
        Serial.print(".");
        if (millis()-count>4000) break;
      }
      Serial.println();
      Serial.println("-----------");
      String op;
      while (client.available()) 
      {
        op+=(char)client.read();
      }
      Serial.println(op);
      int trans_id = op.indexOf("transcript");
      int starto, endo;
      if (trans_id != -1)
      {
        int foll_coll = op.indexOf(":",trans_id);
        starto = foll_coll+2;                                           // starting index
        endo = op.indexOf("\"",starto+1);                               // ending index
        display.clear();
        display.drawString(0, 0, "Transcript: ");
        display.drawString(0, 10, op.substring(starto+1,endo));
        display.display();     
        delay(5000);
      }
      get_information(op.substring(starto+1, endo));
      Serial.println(op.substring(starto+1, endo));
      Serial.println("-----------");
      client.stop();
      Serial.println("done");
    }
  }
  old_button_state = button_state;
  if(millis()-parse_watch > POLL_TIME*60000)
  {
    parse_db();
    parse_watch=millis();
  }
  
}

//function used to record audio at sample rate for a fixed nmber of samples
void record_audio() {
  int sample_num = 0;                                                 // counter for samples
  int enc_index = PREFIX.length()-1;                                  // index counter for encoded samples
  float time_between_samples = 1000000/SAMPLE_FREQ;
  int value = 0;
  uint8_t raw_samples[3];                                             // 8-bit raw sample data array
  String enc_samples;                                                 // encoded sample data array
  time_since_sample = micros();
  Serial.println(NUM_SAMPLES);
  speech_data = PREFIX;
  while (sample_num<NUM_SAMPLES) {                                    // read in NUM_SAMPLES worth of audio data
    value = analogRead(AUDIO_IN);                                     // make measurement
    raw_samples[sample_num%3] = mulaw_encode(value-995);              // remove 1.0V offset (from 12 bit reading)
    sample_num++;
    if (sample_num%3 == 0) {
      speech_data+=base64::encode(raw_samples, 3);
    }

    // wait till next time to read
    while (micros()-time_since_sample <= time_between_samples);       // wait...
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

void append_db(double lat, double lon, long hr, long mn, String what, String where)
{
  for(int i=0;i<MAX_NUM_OF_ENTRIES;i++)
    if(memLoc[i].flag==0)
      {
        memLoc[i].lat = lat;
        memLoc[i].lon = lon;
        memLoc[i].hr = hr;
        memLoc[i].mn = mn;
        memLoc[i].what = what;
        memLoc[i].where = where;
        memLoc[i].flag = 1;
        break;
      }
  Serial.println("Successfully appended to database");
  display.clear();
  display.drawString(0, 0, "Successfully appended to");
  display.drawString(0, 10, "database");
  display.display(); 
}

void delete_db(int to_be_deleted)
{
  memLoc[to_be_deleted].flag=0;
}

void get_GPS()
{
  // double gps_[2] = {74.792558,13.011026}; //Dept
  // float gps_[2] = {74.794571, 13.007889}; // Hostel
    while (GPS.available() > 0) {
      gps.encode(GPS.read());
  }
  originLat = 13.0064237; // gps.location.lat();
  Serial.print("Current latitude: ");
  Serial.println(originLat);
  originLon = 74.7937679; // gps.location.lng();
  Serial.print("Current longitude: ");
  Serial.println(originLon);
}

double ETA(double lat1, double lon1, double lat2, double lon2)
{
  lat1 = (lat1-74)*10000;
  lat2 = (lat2-74)*10000;
  lon1 = (lon1-13)*10000;
  lon2 = (lon2-13)*10000;
  //Serial.println(lat1);
  
  double to_time = (abs(lat1-lat2) + abs(lon1-lon2))*VELOCITY_CONSTANT;
  return to_time; // RETURN MINS
}

void parse_db()
{
  get_GPS();
  //Serial.println(gps_co_ord[0]);
  //Serial.println(gps_co_ord[1]);
  //float gps_co_opd[2] = {13.011026, 74.792558};
  int closest_time = 2000;
  int closest_index = 21;
  
  for(int i=0; i < MAX_NUM_OF_ENTRIES; i++)
    if(memLoc[i].flag==1)
    {
      if((memLoc[i].hr*60 + memLoc[i].mn) < closest_time){
        closest_time = memLoc[i].hr*60 + memLoc[i].mn;
        closest_index = i;
      }
    }

//   if(abs(closest_time-(hour()*60 + minute()))  < POLL_TIME) // Need to get real time properly.
//   {
//    //display oled;
//    
//    display.drawString(0, 0, memLoc[closest_index].what);
//    display.drawString(0,10, memLoc[closest_index].where);
//    display.drawString(0,10, "Hurry Up!");
//    display.display();
//    
//    delete_db(closest_index);
//    return;
//   }
//  else
  //{
    double closest_eta = ETA(memLoc[closest_index].lat, memLoc[closest_index].lon, originLat, originLon);
    if ((gps.time.hour()*60 + gps.time.minute() + 330) + closest_eta > closest_time)
    {
      display.clear();
      display.drawString(0, 0, memLoc[closest_index].what);
      display.display();
      display.drawString(0,10, memLoc[closest_index].where);
      display.display();
      display.drawString(0, 20, String(memLoc[closest_index].hr));
      display.display();
      display.drawString(2, 20, ":");
      display.display();
      display.drawString(3, 20, String(memLoc[closest_index].mn));
      display.display();
      display.drawString(0,30, String(closest_eta,4));
      display.display();
      delay(5000);
      Serial.print("Time: ");
      Serial.print(memLoc[closest_index].hr);
      Serial.print(":");
      Serial.println(memLoc[closest_index].mn);
      Serial.print("What: ");
      Serial.println(memLoc[closest_index].what);
      Serial.print("Where: ");
      Serial.println(memLoc[closest_index].where);
      Serial.print("Time to reach: ");
      Serial.println(String(closest_eta, 2));
      delete_db(closest_index);
      return;
    }
 // }
}

void get_information(String transcript)
{
    memoryLocation temp;
    temp.hr=23;
    temp.mn=0;
    transcript.toLowerCase();
    //Serial.println(transcript);
    int flag_place = 0;
    for(int place_iterator=0; place_iterator<no_of_places;place_iterator++) 
    {
      if(transcript.indexOf(Places[place_iterator])!=-1)
        {     
          temp.where = Places[place_iterator];
          temp.lon = LONGITUDE[place_iterator];
          temp.lat = LATITUDE[place_iterator];
          //Serial.println(Places[place_iterator]);
          //Serial.println(LONGITUDE[place_iterator]*10000);
          //Serial.println(LATITUDE[place_iterator]*10000);
          flag_place=1;
          break;
        }
    }
    
     if(flag_place==0)
     {
        temp.lat = 13.009666;
        temp.lon = 74.788786;
        temp.where = "beach";
     }

   int flag_work = 0;
   for(int work_iterator=0; work_iterator<no_of_works;work_iterator++)
   {
    if(transcript.indexOf(Works[work_iterator])!=-1)
    {
      temp.what=Works[work_iterator];
      flag_work=1;
      break;
    }
   }
     if(flag_work==0)
     {
        temp.what="meeting";
     }

    int flag_time = 0;

    String time_temp[10] = {"1","2","3","4","5","6","7","8","9","0"};
    int colon_index = transcript.indexOf(":");
    if(colon_index!=-1)
      {
      for(int time_iterator=0; time_iterator<10;time_iterator++)
      {
        int time_index = transcript.substring(0,colon_index).indexOf(time_temp[time_iterator]);
        if(time_index!=-1)
        {
          temp.hr = transcript.substring(time_index, colon_index).toInt();
          temp.mn = transcript.substring(colon_index+1, colon_index+3).toInt();
          break;
        }
      }  
    }
    else 
    {
      for(int time_iterator=0; time_iterator<10;time_iterator++)
      {
        int time_index = transcript.substring(0).indexOf(time_temp[time_iterator]);
        if(time_index!=-1)
        {
          temp.hr = transcript.substring(time_index, time_index + 1).toInt();
          temp.mn=0;
          break;
        }
      }    
    }
    if(transcript.indexOf("p.m")!=-1)
      temp.hr+=12;
    Serial.print("Time: ");
    Serial.print(temp.hr);
    Serial.print(":");
    Serial.println(temp.mn);
    Serial.print("What: ");
    Serial.println(temp.what);
    Serial.print("Where: ");
    Serial.println(temp.where);
//    display.clear();
//    display.drawString(0, 0, "Where: ");
//    display.drawString(8, 0, temp.where);
//    display.drawString(0, 10, "What: ");
//    display.drawString(7, 10, temp.what);
//    display.drawString(0, 20, "When: ");
//    display.drawString(7, 20, String(temp.hr));
//    display.drawString(9, 20, ":");
//    display.drawString(10, 20, String(temp.mn));
//    display.display();
//    delay(5000);
    append_db(temp.lat, temp.lon, temp.hr, temp.mn, temp.what, temp.where);      
}
