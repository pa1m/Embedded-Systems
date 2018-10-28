#include <WiFi.h>

const char* ssid = "airtel_9E624A";
const char* password = "JSFFDJ5CJj";
const char* server = "numbersapi.com";

WiFiClient client;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  WiFi.begin(ssid, password);
  int count = 5;
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED && count > 0)
  {
    Serial.print(".");
    delay(500);
    count--;
  }

  delay(1000);
  if(WiFi.isConnected())
  {
    Serial.println("Connected to:");
    Serial.print("SSID:");
    Serial.println(ssid);
    Serial.print("IP:");
    Serial.println(WiFi.localIP().toString());
  }
  else
  {
    Serial.println(WiFi.status());
    ESP.restart();
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  
  if(client.connect(server, 80))
  {
    client.println("GET http://numbersapi.com/42/trivia HTTP/1.1");
    client.println("Host: numbersapi.com");
    client.print("\r\n"); 

    String line;
    while(client.connected())
    {
      line = client.readStringUntil('\n');
      Serial.println(line);
      if(line == "\r")
        Serial.println("Headers Received");
      Serial.println("");
      break;     
    }
  
    String response;
    while(client.available())
    {
      response += (char)client.read();
    }
    Serial.println(response);
    client.stop();
  }
  else
  {
    Serial.println("Connection failed!");
  }  
}
