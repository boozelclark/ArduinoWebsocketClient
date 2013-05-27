# ArduinoWebsocketClient, an Arduino client for connecting and messaging with Websockets


## Caveats

This library has been modified to support RFC6455. At present the Sec-WebSocket-Key is hardcoded to a valid key .Because Arduino doesn't support SSL, this library also doesn't support the use of Websockets over https.  If you're interested in learning more about the Websocket spec I recommend checking out the [Wikipedia Page](http://en.wikipedia.org/wiki/WebSocket).  Now that I've got that out of the way, I've been able to successfully use this to connect to several hosted Websocket services, including: [echo.websocket.org](http://websocket.org/echo.html).

## Installation instructions

Once you've cloned this repo locally, copy the ArduinoWebsocketClient directory into your Arduino Sketchbook directory under Libraries then restart the Arduino IDE so that it notices the new library.  Now, under File\Examples you should see ArduinoWebsocketClient.  To use the library in your app, select Sketch\Import Library\ArduinoWebsocketClient.

## How To Use This Library

```
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
char server[] = "echo.websocket.org";
WebSocketClient client;

void setup() {
  Serial.begin(9600);
  Ethernet.begin(mac);
  client.connect(server);
  client.setDataArrivedDelegate(dataArrived);
  client.send("Hello World!");
}

void loop() {
  client.catchMessages();
}

void dataArrived(WebSocketClient client, String data) {
  Serial.println("Data Arrived: " + data);
}
```

## Examples

There example included with this library, EchoExample, will connect to echo.websocket.org, which hosts a service that simply echos any messages that you send it via Websocket.  This example sends the message "Hello World!".  If the example runs correctly, the Arduino will receive this same message back over the Websocket and print it via Serial.println.