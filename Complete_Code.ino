#include <Wire.h>
#include "SSD1306.h"
#include <WiFiClientSecure.h>
#include <base64.h>

//WiFiClientSecure is a big library. It can take a bit of time to do that first compile

// Set up the oled object
SSD1306 display(0x3c, 21, 22); 

#define DELAY 1000
#define SAMPLE_FREQ 8000                                             // Hz, telephone sample rate
#define SAMPLE_DURATION 5                                            // duration of fixed sampling
#define NUM_SAMPLES SAMPLE_FREQ*SAMPLE_DURATION                      // number of of samples
#define ENC_LEN (NUM_SAMPLES + 2 - ((NUM_SAMPLES + 2) % 3)) / 3 * 4  // Encoded length of clip

/* CONSTANTS */
//Prefix to POST request:
const String PREFIX = "{\"config\":{\"encoding\":\"MULAW\",\"sampleRateHertz\":8000,\"languageCode\": \"en-US\"}, \"audio\": {\"content\":\"";
const String SUFFIX = "\"}}";                                        // suffix to POST request
const int AUDIO_IN = A0;                                             // pin where microphone is connected
const int BUTTON_PIN = 15;                                            // pin where button is connected
const int capacitanceMaxima = 50;
const int serial_update_period = 20;
const String API_KEY = "AI***********************************ts";

//AI***********************************lg  (Lab API key)

//My keys
//AI***********************************ts  

/* Global variables*/
int button_state;                                                    // used for containing button state and detecting edges
int old_button_state;                                                // used for detecting button edges
unsigned long time_since_sample;                                     // used for microsecond timing
int serial_counter;                                                  //     
int mediaT0;
unsigned long timer;
String speech_data;                                                  // global used for collecting speech data
const char* ssid     = "airtel_9E624A";                              // your network SSID (name of wifi network)
const char* password = "JSFFDJ5CJj";                                 // your network password
const char*  server = "speech.googleapis.com";                       // Server URL

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

//"-----BEGIN CERTIFICATE-----\n"\
//"MIIFWjCCA0KgAwIBAgIQbkepxUtHDA3sM9CJuRz04TANBgkqhkiG9w0BAQwFADBH\n"\
//"MQswCQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExM\n"\
//"QzEUMBIGA1UEAxMLR1RTIFJvb3QgUjEwHhcNMTYwNjIyMDAwMDAwWhcNMzYwNjIy\n"\
//"MDAwMDAwWjBHMQswCQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNl\n"\
//"cnZpY2VzIExMQzEUMBIGA1UEAxMLR1RTIFJvb3QgUjEwggIiMA0GCSqGSIb3DQEB\n"\
//"AQUAA4ICDwAwggIKAoICAQC2EQKLHuOhd5s73L+UPreVp0A8of2C+X0yBoJx9vaM\n"\
//"f/vo27xqLpeXo4xL+Sv2sfnOhB2x+cWX3u+58qPpvBKJXqeqUqv4IyfLpLGcY9vX\n"\
//"mX7wCl7raKb0xlpHDU0QM+NOsROjyBhsS+z8CZDfnWQpJSMHobTSPS5g4M/SCYe7\n"\
//"zUjwTcLCeoiKu7rPWRnWr4+wB7CeMfGCwcDfLqZtbBkOtdh+JhpFAz2weaSUKK0P\n"\
//"fyblqAj+lug8aJRT7oM6iCsVlgmy4HqMLnXWnOunVmSPlk9orj2XwoSPwLxAwAtc\n"\
//"vfaHszVsrBhQf4TgTM2S0yDpM7xSma8ytSmzJSq0SPly4cpk9+aCEI3oncKKiPo4\n"\
//"Zor8Y/kB+Xj9e1x3+naH+uzfsQ55lVe0vSbv1gHR6xYKu44LtcXFilWr06zqkUsp\n"\
//"zBmkMiVOKvFlRNACzqrOSbTqn3yDsEB750Orp2yjj32JgfpMpf/VjsPOS+C12LOO\n"\
//"Rc92wO1AK/1TD7Cn1TsNsYqiA94xrcx36m97PtbfkSIS5r762DL8EGMUUXLeXdYW\n"\
//"k70paDPvOmbsB4om3xPXV2V4J95eSRQAogB/mqghtqmxlbCluQ0WEdrHbEg8QOB+\n"\
//"DVrNVjzRlwW5y0vtOUucxD/SVRNuJLDWcfr0wbrM7Rv1/oFB2ACYPTrIrnqYNxgF\n"\
//"lQIDAQABo0IwQDAOBgNVHQ8BAf8EBAMCAQYwDwYDVR0TAQH/BAUwAwEB/zAdBgNV\n"\
//"HQ4EFgQU5K8rJnEaK0gnhS9SZizv8IkTcT4wDQYJKoZIhvcNAQEMBQADggIBADiW\n"\
//"Cu49tJYeX++dnAsznyvgyv3SjgofQXSlfKqE1OXyHuY3UjKcC9FhHb8owbZEKTV1\n"\
//"d5iyfNm9dKyKaOOpMQkpAWBz40d8U6iQSifvS9efk+eCNs6aaAyC58/UEBZvXw6Z\n"\
//"XPYfcX3v73svfuo21pdwCxXu11xWajOl40k4DLh9+42FpLFZXvRq4d2h9mREruZR\n"\
//"gyFmxhE+885H7pwoHyXa/6xmld01D1zvICxi/ZG6qcz8WpyTgYMpl0p8WnK0OdC3\n"\
//"d8t5/Wk6kjftbjhlRn7pYL15iJdfOBL07q9bgsiG1eGZbYwE8na6SfZu6W0eX6Dv\n"\
//"J4J2QPim01hcDyxC2kLGe4g0x8HYRZvBPsVhHdljUEn2NIVq4BjFbkerQUIpm/Zg\n"\
//"DdIx02OYI5NaAIFItO/Nis3Jz5nu2Z6qNuFoS3FJFDYoOj0dzpqPJeaAcWErtXvM\n"\
//"+SUWgeExX6GjfhaknBZqlxi9dnKlC54dNuYvoS++cJEPqOba+MSSQGwlfnuzCdyy\n"\
//"F62ARPBopY+Udf90WuioAnwMCeKpSwughQtiue+hMZL77/ZRBIls6Kl0obsXs7X9\n"\
//"SQ98POyDGCBDTtWTurQ0sR8WNh8M5mQ5Fkzc4P4dyKliPUDqysU0ArSuiYgzNdws\n"\
//"E3PYJ/HQcu51OyLemGhmW/HGY0dVHLqlCFF1pkgl\n"\
//"-----END CERTIFICATE-----\n";

void setup() {
  
  Serial.begin(115200);                                                 // Set up serial port
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
  display.setFont(ArialMT_Plain_10);                                   //set font on oled  
  display.drawString(0, 0, "Ready. Touch to Record");
  display.display();
  delay(5000);
  display.clear();
  display.drawString(0, 0, "Look at the Serial Monitor");
  display.drawString(0, 20, "for Debugging Infromation");
  display.display();
  delay(5000);
  display.clear(); 
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  old_button_state = digitalRead(BUTTON_PIN);
}

//main body of code
void loop() 
{  
  button_state = digitalRead(BUTTON_PIN);
  if (!button_state && button_state != old_button_state)
  {
    client.setCACert(root_ca);
    delay(200);
    Serial.println("Listening...");
    display.drawString(0, 0, "Listening...");
    display.display(); 
    display.clear();    
    record_audio();
    Serial.println("Sending...");    
    display.drawString(0,0,"Sending...");
    display.display();
    display.clear();
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
        display.drawString(0, 0, op.substring(starto+1,endo));
        display.display();     
        delay(5000);
        display.clear();
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
