#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "DHT.h"

#define DHTPIN 4
#define DHTTYPE DHT11 

#define LED_WHITE 14
#define LED_BLUE 12
//#define LED_GREEN 13

const char* ssid = "VeritechWifi2";
const char* password = "vetronicsltd.";

ESP8266WebServer server(80);
DHT dht(DHTPIN, DHTTYPE);

long int time1,time2;
int t,h;
int co2;
char cmd[]={0xFE,0x04,0x00,0x03,0x00,0x01,0xD5,0xC5};
char rec[10];

void handleCO2() {
  digitalWrite(LED_BLUE, 1);
  String mss = "Answer=";
  mss = mss + String(rec[0],HEX) + " " +String(rec[1],HEX) + " " +String(rec[2],HEX) + " " +String(rec[3],HEX) + " " +String(rec[4],HEX);
  server.send(200, "text/plain", mss);
  digitalWrite(LED_BLUE, 0);  
}

void handleRoot() {
  digitalWrite(LED_BLUE, 1);
  String mss;
  mss = "Temp=" + String(t) + " humidity=" + String(h) + " co2=" + String(co2) + " time from start min=" + String(time1/1000/60);
  server.send(200, "text/plain", mss);
  digitalWrite(LED_BLUE, 0);
}

void handleNotFound(){
  digitalWrite(LED_BLUE, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(LED_BLUE, 0);
}

void setup(void){
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_WHITE, OUTPUT);
//  pinMode(LED_GREEN, OUTPUT);
  
  digitalWrite(LED_BLUE, 0);
  digitalWrite(LED_WHITE, 0);
//  digitalWrite(LED_GREEN, 0);
  
  Serial.begin(115200); //!!!!
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BLUE, 1);
    delay(125);
    digitalWrite(LED_BLUE, 0);
    delay(125);
    digitalWrite(LED_BLUE, 1);
    delay(125);
    digitalWrite(LED_BLUE, 0);
    delay(125);    
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/co2",handleCO2);

  server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
  dht.begin();
  time1 = millis();
  
  Serial.end();
  delay(100);
  Serial.begin(9600); // connect to S8 Air
  Serial.swap();

 
}


void loop(void){
  server.handleClient();

  time2 = millis();
  if( (time2-time1) > 5000 )
  {
    digitalWrite(LED_WHITE, 1); 
    h = (int)dht.readHumidity();
    t = (int)dht.readTemperature(); 

    for(int i=0;i<8;i++)
    {
      Serial.print(cmd[i]);
    }
     delay(150);
     while(!Serial.available())
     {
       delay(50);
     }
    Serial.readBytes(rec, 7); 

    co2 = ((int)rec[3])*256 + rec[4];  
        
    time1 = time2; 
    digitalWrite(LED_WHITE, 0);
  }
}
