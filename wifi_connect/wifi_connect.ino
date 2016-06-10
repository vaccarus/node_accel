/*
 *  This sketch sends data via HTTP GET requests to data.sparkfun.com service.
 *
 *  You need to get streamId and privateKey at data.sparkfun.com and paste them
 *  below. Or just customize this script to talk to other HTTP servers.
 *
 */

#include <ESP8266WiFi.h>

const char* ssid     = "PPI";
const char* password = "2020@Pr3c1s1on!";
IPAddress host(10,20,0,184);
const int port = 1337;
// Use WiFiClient class to create TCP connections
WiFiClient client;

void setup() {
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.print("connecting to server");
  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    return;
  }
}

int value = 0;
unsigned char cnt=0;


void loop() {
  delay(50);
  ++value;

  //Check to see if we are still connected
  if(!client.connected()) {
    // connect again
    Serial.print("Disconnected... connecting to server");
    if (!client.connect(host, port)) {
      Serial.println("connection failed");
      return;
    }
  }

  
  cnt++;
  
  // This will send the request to the server
  client.write((const uint8_t*)&cnt,1);
  //client.println(cnt);
//  unsigned long timeout = millis();
//  while (client.available() == 0) {
//    if (millis() - timeout > 5000) {
//      Serial.println(">>> Client Timeout !");
//      client.stop();
//      return;
//    }
//  }
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    Serial.println("Received data");
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  

}

