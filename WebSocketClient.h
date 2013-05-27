#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H_

#include <string.h>
#include <stdlib.h>
#include <WString.h>
#include <Ethernet.h>
#include "Arduino.h"

//Uncomment this to use WIFLY Client
#define WIFLY true

class WebSocketClient {
	public:
		typedef void (*DataArrivedDelegate)(WebSocketClient client, String data);
		bool connect(char hostname[], char path[] = "/", int port = 80);
        bool connected();
        void disconnect();
		void setDataArrivedDelegate(DataArrivedDelegate dataArrivedDelegate);
		void send(String data);
		void sendMessage(String str);
		void catchMessages();
	private:
        String getStringTableItem(int index);
        void sendHandshake(char hostname[], char path[]);
        EthernetClient _client;
        DataArrivedDelegate _dataArrivedDelegate;
        bool readHandshake();
        String readLine();
	void genFrame(String str, int opcode);
};


#endif