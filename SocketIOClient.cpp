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
#include <SocketIOClient.h>

bool SocketIOClient::connect(Adafruit_CC3000 cc3000, char thehostname[], int theport) {
  ip = 0;
  uint8_t timeoutRetry = 0;
  while (ip == 0) {
    if (!cc3000.getHostByName(thehostname, &ip)) {
      timeoutRetry++;
      if (timeoutRetry < 3) {
        // wdt_reset();
      }
    }
    delay(200);
  }

  unsigned long lastRead = millis();
  do {
    client = cc3000.connectTCP(ip, 80);
  } while((!client.connected()) && ((millis() - lastRead) < 3000));

  hostname = thehostname;
  port = theport;
  sendHandshake(hostname);

  return readHandshake(cc3000);
}

bool SocketIOClient::connected() {
  return client.connected();
}


void SocketIOClient::disconnect() {
  client.close();
}

/**
 * Find the nth colon starting from dataptr
 */
void SocketIOClient::findColon(char which) {    
  while (*dataptr) {
    if (*dataptr == ':') {
      if (--which <= 0) return;
    }
    ++dataptr;
  }
}

/**
 * Terminate command at dataptr at closing double quote
 */
void SocketIOClient::terminateCommand(void) {
  dataptr[strlen(dataptr)-3] = 0;
}

void SocketIOClient::monitor(Adafruit_CC3000 cc3000) {
  *databuffer = 0;

  if (!client.connected()) {
    connect(cc3000, hostname, port);
  }

  if (!client.available()) {
    return;
  }

  char which;
  while (client.available()) {
    readLine();
    dataptr = databuffer;
    dataptr[strlen(dataptr)-1] = 0;
    switch (databuffer[0]) {    

    case '1': // connect: []
      which = 6;
      break;

    case '2': // heartbeat: [2::]
      client.print((char)0);
      client.print("2::");
      client.print((char)255);
      continue;

    case '5': // event: [5:::{"name":"ls"}]
      which = 4;
      break;

    default: 
      Serial.print("Drop ");
      Serial.println(dataptr);
      continue;
    }

    findColon(which);

    if (dataArrivedDelegate != NULL) {
      dataArrivedDelegate(*this, databuffer);
    }
  }
}

void SocketIOClient::setDataArrivedDelegate(DataArrivedDelegate newdataArrivedDelegate) {
    dataArrivedDelegate = newdataArrivedDelegate;
}

void SocketIOClient::sendHandshake(char hostname[]) {
  Serial.println(client.connected());
  client.fastrprint(F("GET /socket.io/1/ HTTP/1.1\r\n"));
  client.fastrprint(F("Host: "));
  client.fastrprint(hostname);
  client.fastrprint(F("\r\n"));
  
  //other headers
  client.fastrprint(F("Origin: Arduino\r\n"));

  //end headers
  client.fastrprint(F("\r\n"));
  client.println();
}

bool SocketIOClient::waitForInput(void) {
  unsigned long now = millis();
  while (!client.available() && ((millis() - now) < 30000UL)) {;}
  return client.available();
}

void SocketIOClient::eatHeader(void) {
  while (client.available()) { // consume the header
    readLine();
    if (strlen(databuffer) == 0) break;
  }
}

bool SocketIOClient::readHandshake(Adafruit_CC3000 cc3000) {
  if (!waitForInput()) return false;

  // check for happy "HTTP/1.1 200" response
  readLine();

  if (atoi(&databuffer[9]) != 200) {
    while (client.available()) readLine();
    client.close();
    return false;
  }

  eatHeader();
  readLine(); // read first line of response
  readLine(); // read sid : transport : timeout

  char *iptr = databuffer;
  char *optr = sid;
  while (*iptr && (*iptr != ':') && (optr < &sid[SID_LEN-2])) *optr++ = *iptr++;
  *optr = 0;

  Serial.print(F("Connected. SID="));
  Serial.println(sid); // sid:transport:timeout 

  while (client.available()) readLine();
  client.close();
  delay(1000);

  // reconnect on websocket connection
  Serial.print(F("WS Connect..."));
  ip = 0;
  uint8_t timeoutRetry = 0;
  while (ip == 0) {
    if (!cc3000.getHostByName(hostname, &ip)){
      timeoutRetry++;
      if (timeoutRetry < 3) {
        // wdt_reset();
      }
    }
    delay(200);
  }

  unsigned long lastRead = millis();
  do {
    client = cc3000.connectTCP(ip, 80);
  } while((!client.connected()) && ((millis() - lastRead) < 3000));

  Serial.println(F("Reconnected."));
  wdt_reset();

  client.print(F("GET /socket.io/1/websocket/"));
  client.print(sid);
  client.println(F(" HTTP/1.1"));
  client.print(F("Host: "));
  client.println(hostname);
  client.println(F("Origin: ArduinoSocketIOClient"));
  client.println(F("Upgrade: WebSocket"));
  client.println(F("Connection: Upgrade\r\n"));

  if (!waitForInput()) return false;

  readLine();
  if (atoi(&databuffer[9]) != 101) {
    while (client.available()) readLine();
    client.close();
    return false;
  }
  eatHeader();
  monitor(cc3000); // treat the response as input
  return true;
}

void SocketIOClient::readLine() {
  dataptr = databuffer;
  
  while (client.available() && (dataptr < &databuffer[DATA_BUFFER_LEN-2])) {
    char c = client.read();
    if (c == 0) {;}
    else if (c == 255) Serial.print(F("0x255"));
    else if (c == '\r') {;}
    else if (c == '\n') break;
    else *dataptr++ = c;
  }
  *dataptr = 0;
}

void SocketIOClient::sendMessage(char *data) {
  client.print((char)0);
  client.print("3:::");
  client.print(data);
  client.print((char)255);
}
void SocketIOClient::sendEvent(char *event, char *data) {
  client.print((char)0);
  client.print("5:::{\"name\":\"");
  client.print(event);
  client.print("\", \"args\":[\"");
  client.print(data);
  client.print("\"]}");
  client.print((char)255);
}
