/*
 WebsocketClient, a websocket client for Arduino
 Copyright 2011 Kevin Rohling
 http://kevinrohling.com
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */

#include <WebSocketClient.h>
#include <WString.h>
#include <string.h>
#include <stdlib.h>

prog_char stringVar[] PROGMEM = "{0}";
prog_char clientHandshakeLine1[] PROGMEM = "GET {0} HTTP/1.1";
prog_char clientHandshakeLine2[] PROGMEM = "Upgrade: WebSocket";
prog_char clientHandshakeLine3[] PROGMEM = "Connection: Upgrade";
prog_char clientHandshakeLine4[] PROGMEM = "Host: {0}";
prog_char clientHandshakeLine5[] PROGMEM = "Origin:null";
prog_char clientHandshakeLine6[] PROGMEM = "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==";
prog_char clientHandshakeLine7[] PROGMEM = "Sec-WebSocket-Protocol: chat";
prog_char clientHandshakeLine8[] PROGMEM = "Sec-WebSocket-Version: 13";
prog_char serverHandshake[] PROGMEM = "HTTP/1.1 101";

PROGMEM const char *WebSocketClientStringTable[] =
{   
    stringVar,
    clientHandshakeLine1,
    clientHandshakeLine2,
    clientHandshakeLine3,
    clientHandshakeLine4,
    clientHandshakeLine5,
    clientHandshakeLine6,
    clientHandshakeLine7,
    clientHandshakeLine8,
    serverHandshake
};

String WebSocketClient::getStringTableItem(int index) {
    char buffer[75];
    strcpy_P(buffer, (char*)pgm_read_word(&(WebSocketClientStringTable[index])));
    return String(buffer);
}

bool WebSocketClient::connect(char hostname[], char path[], int port) {
    bool result = false;

    if (_client.connect(hostname, port)) {
        sendHandshake(hostname, path);
        result = readHandshake();
    }
    
	return result;
}


bool WebSocketClient::connected() {
    return _client.connected();
}

void WebSocketClient::disconnect() {
    _client.stop();
}

void WebSocketClient::catchMessages () {

	 byte character;
    boolean masked=false;
    int noOfPaloadBytes = 0;
    int length;
    String data = "";
    byte mask [4];
    
	//if (_client.available() > 0 && (character = _client.read()) == 0) {
      if (_client.available() > 0) {
        character = _client.read();
        
        if(character == B10000001)
        { 
          character = _client.read();
          
          if(bitRead(character,7)==1)
          {  
            //UNMASK
            masked=true;
          }
          //ckear mask if set to read payload
          bitClear(character,7);
          
          //TODO add support for larger message
          if(character<126)
          {noOfPaloadBytes=1;}
          else if(character == 126)
          {noOfPaloadBytes=2;}
          else if(character == 127)
          {noOfPaloadBytes=3;}
          
          length = character; 
          
          if(masked){
            mask[0]=_client.read();
            mask[1]=_client.read();
            mask[2]=_client.read();
            mask[3]=_client.read();
          }
          
          for (int i =0;i<length;i++)
          {
            character=_client.read();
            
            if(masked){
            //MASKING FORMULA
            //j                   = i MOD 4
            // transformed-octet-i = original-octet-i XOR masking-key-octet-j
            int j = i%4;
            byte transformed =   character ^ mask[j];
    
            character = transformed;
            }          
            
            data += char(character);
          }
        
	
        if (_dataArrivedDelegate != NULL) {
            _dataArrivedDelegate(*this, data);
        }        

        }
        
    }
}

void WebSocketClient::setDataArrivedDelegate(DataArrivedDelegate dataArrivedDelegate) {
	  _dataArrivedDelegate = dataArrivedDelegate;
}


void WebSocketClient::sendHandshake(char hostname[], char path[]) {
    String stringVar = getStringTableItem(0);
    String line1 = getStringTableItem(1);
    String line2 = getStringTableItem(2);
    String line3 = getStringTableItem(3);
    String line4 = getStringTableItem(4);
    String line5 = getStringTableItem(5);
    String line6 = getStringTableItem(6);
    String line7 = getStringTableItem(7);
    String line8 = getStringTableItem(8);
    
    line1.replace(stringVar, path);
    line4.replace(stringVar, hostname);
    
    _client.println(line1);
    _client.println(line2);
    _client.println(line3);
    _client.println(line4);
    _client.println(line5);
    _client.println(line6);
    _client.println(line7);
    _client.println(line8);
    _client.println();
}

bool WebSocketClient::readHandshake() {
    bool result = false;
    char character;
    String handshake = "", line;
    int maxAttempts = 300, attempts = 0;
    
    while(_client.available() == 0 && attempts < maxAttempts) 
    { 
        delay(100); 
        attempts++;
    }
    
    while((line = readLine()) != "") {
        handshake += line + '\n';
    }
    
    String response = getStringTableItem(9);
    result = handshake.indexOf(response) != -1;
    
    if(!result) {
        _client.stop();
    }
    
    return result;
}

String WebSocketClient::readLine() {
    String line = "";
    char character;
    
    while(_client.available() > 0 && (character = _client.read()) != '\n') {
        if (character != '\r' && character != -1) {
            line += character;
        }
    }
    
    return line;
}

void WebSocketClient::send (String data) {
	
	_client.print((char)0);
	_client.print(data);
    _client.println((char)255);
}

void WebSocketClient::sendMessage(String message)
{
	genFrame(message,1);
}

void WebSocketClient::genFrame (String str, int opcode) {
	//Variables
  int payloadLengthByte = 0; 
  int maskIndex = 0;
  
  //Convert incoming string into a char array
  int payloadLength = str.length() + 1;
  char payload[payloadLength];
  str.toCharArray(payload, payloadLength);
  
  //Determine how may bytes are required to hold the payload length  
  if(payloadLength < 126)
  { payloadLengthByte = 1;
  }else if (payloadLength == 126)
  { payloadLengthByte = 3;
  }else if (payloadLength == 127)
  { payloadLengthByte = 9;}
    
  //Get the size needed fot the entrire frame and then create a byte array for that frame  
  int frameByteCount = 1 + (payloadLengthByte) + 4 +  payloadLength; //1 for fin byte, then bytes for the payload length (1,3 or 9), then the mask bytes followed by the payload
  byte frame[frameByteCount];  
    
  //set Fin bit and opcode into first byte
  frame[0]=(B10000000)+opcode;
  
  //set the payload length into the 2nd byte
  //TODO: support larger frames
  if(payloadLengthByte == 1)
  {frame[1] = (B10000000) + payloadLength;}
  
  //create mask
  //frame[1+payloadLengthByte] = B10000101;
  frame[1+payloadLengthByte]     = random(B11111111);
  frame[1+payloadLengthByte + 1] = random(B11111111);
  frame[1+payloadLengthByte + 2] = random(B11111111);
  frame[1+payloadLengthByte + 3] = random(B11111111);
  
  //Apply mask
  for (int index = 0; index < sizeof(payload); index++) {
    //MASKING FORMULA
    //j                   = i MOD 4
    // transformed-octet-i = original-octet-i XOR masking-key-octet-j
    int j = index%4;
    byte transformed =   payload[index] ^ frame[1+payloadLengthByte + j];
    
    frame[1+payloadLengthByte+4+index] = transformed;
  }
  
  for (int index = 0; index < sizeof(frame); index++) {
    _client.write(frame[index]);
  }
}

