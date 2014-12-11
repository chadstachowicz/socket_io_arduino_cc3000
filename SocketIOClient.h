/*
	socket.io-arduino-client: a Socket.IO client for the Arduino

	Based on the Kevin Rohling WebSocketClient

	Copyright 2013 Bill Roy

	Permission is hereby granted, free of charge, to any person
	obtaining a copy of this software and associated documentation
	files (the "Software"), to deal in the Software without
	restriction, including without limitation the rights to use,
	copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the
	Software is furnished to do so, subject to the following
	conditions:
	
	The above copyright notice and this permission notice shall be
	included in all copies or substantial portions of the Software.
	
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
	OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
	NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
	WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
	OTHER DEALINGS IN THE SOFTWARE.
*/

/**
 * Includes
 */
#include "Arduino.h"
#include <Adafruit_CC3000.h>
#include <avr/wdt.h>
#include "SPI.h"

// Length of static data buffers
#define DATA_BUFFER_LEN 120
#define SID_LEN 24

class SocketIOClient {
	public:
		typedef void (*DataArrivedDelegate)(SocketIOClient client, char *data);
		bool connect(Adafruit_CC3000 cc3000, char hostname[], int port = 80);
        bool connected();
        void disconnect();
		void monitor(Adafruit_CC3000 cc3000);
		void setDataArrivedDelegate(DataArrivedDelegate dataArrivedDelegate);
		void sendMessage(char *data);
        void sendEvent(char *event, char *data);
	private:
        void sendHandshake(char hostname[]);
        Adafruit_CC3000_Client client;
        uint32_t ip;
        DataArrivedDelegate dataArrivedDelegate;
        bool readHandshake(Adafruit_CC3000 cc3000s);
		void readLine();
		char *dataptr;
		char databuffer[DATA_BUFFER_LEN];
		char sid[SID_LEN];
		char *hostname;
		int port;
		void findColon(char which);
		void terminateCommand(void);
		bool waitForInput(void);
		void eatHeader(void);
};
