#include "Arduino.h"
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#include "secrets.h"

char* ssid = SECRET_SSID;
char* pw = SECRET_PASS;

// outptus are wired: D1, D2, D4, D6, D7, D8, D9, D10
int relayOutputs[] = {5, 4, 2, 12, 13, 15, 3, 1};

ESP8266WebServer server(80);

void getStatus() {
  StaticJsonDocument<JSON_OBJECT_SIZE(8)> response;
  String responseString;
  
  for (int i = 0; i < sizeof(relayOutputs) - 1; i++) {
    Serial.println("Read: " + String(relayOutputs[i]));
    response[String(i)] = digitalRead(relayOutputs[i]);
  }

  serializeJson(response, responseString);

  server.send(200, "text/json", responseString);
}

void setupRoutes() {
  server.on(F("/status"), HTTP_GET, getStatus);
  server.on(F("/set"), HTTP_GET, setRelay);
  server.on(F("/monoflop"), HTTP_GET, handleMonoflop);
  server.on(F("/ping"), HTTP_GET, handlePing);
}

void handleNotFound() {
  server.send(404, "text/plain", "");
}

void monoflop(int id, unsigned long milliseconds) {
  digitalWrite(relayOutputs[id], LOW);
  delay(milliseconds);
  digitalWrite(relayOutputs[id], HIGH);
}

void handlePing() {
  server.send(200, "text/plain", "pong");
}

void handleMonoflop() {
  if(server.hasArg("id") && server.hasArg("ms") && (server.arg("id") != "") && (server.arg("ms") != "")){
    int relayId = server.arg("id").toInt();
    int relayValue = server.arg("ms").toInt();
    server.send(200, "text/plain", ""); 
    monoflop(relayId, relayValue);
  } else {
    server.send(400, "text/plain", "");   
  }
}

void setRelay() {
  if(server.hasArg("id") && server.hasArg("value") && (server.arg("id") != "") && (server.arg("value") != "")){
    int relayId = server.arg("id").toInt();
    int relayValue = server.arg("value").toInt();
    digitalWrite(relayOutputs[relayId], relayValue);

    server.send(200, "text/plain", ""); 
  } else {
    server.send(400, "text/plain", "");   
  }
}


void setup() {
  Serial.begin(115200);
  
  Serial.print("Setting relay outputs..");
  for (int i = 0; i < sizeof(relayOutputs) - 1; i++) {
    pinMode(relayOutputs[i], OUTPUT);
    digitalWrite(relayOutputs[i], HIGH);
  }
  Serial.println("Done");

  Serial.print("Connecting to WiFi..");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pw);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("CONNECTED TO ACCESS POINT : ");
  Serial.println(ssid);
  Serial.println(WiFi.localIP());

  setupRoutes();
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
