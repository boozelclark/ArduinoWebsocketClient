#include "Arduino.h"
#include <Ethernet.h>
#include <SPI.h>
#include <WebSocketClient.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
char server[] = "echo.websocket.org";
char path[] = "/";
int port = 80;
WebSocketClient client;

void setup() {
  Serial.begin(9600);
  Ethernet.begin(mac);

  client.connect(server,path,port);
  client.setDataArrivedDelegate(dataArrived);
  
  client.sendMessage("Hello World");
}

void loop() {
  client.catchMessages();
}

void dataArrived(WebSocketClient client, String data) {
  Serial.println("Data Arrived: " + data);
}

