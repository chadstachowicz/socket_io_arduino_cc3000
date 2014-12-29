# Socket.IO Arduino Client for Adafruit CC3000

This was based off Bill Roy’s [Ethernet shield Socket.IO](https://github.com/billroy/socket.io-arduino-client) version which I then edited to support the Adafruit CC3000 board.

Kevin's documentation is reproduced hereinafter, with changes as needed.

**This library only works with Socket.IO versions 9.x.x and below. There was a change in the protocol that makes it difficult to communicate with the new server. Fixes coming soon.**


## Caveats

This library doesn't support every inch of the Websocket spec, most notably the use of a Sec-Websocket-Key.  Also, because Arduino doesn't support SSL, this library also doesn't support the use of Websockets over https.  If you're interested in learning more about the Websocket spec I recommend checking out the [Wikipedia Page](http://en.wikipedia.org/wiki/WebSocket).  Now that I've got that out of the way, I've been able to successfully use this to connect to several hosted Websocket services, including: [echo.websocket.org](http://websocket.org/echo.html) and [pusherapp.com](http://pusherapp.com).

## Installation instructions

Clone this repo into your Arduino Sketchbook directory under libraries, then restart the Arduino IDE so that it notices the new library. 

## How To Use This Library

```c
/**
 * Includes
 */
#include <Adafruit_CC3000.h>
#include <SocketIOClient.h>
#include <SPI.h>
#include "utility/debug.h"

/**
 * Network config
 */
#define WLAN_SSID "ssid"
#define WLAN_PASS "password"
#define WLAN_SECURITY WLAN_SEC_WPA2

/**
 * Pins
 */
#define ADAFRUIT_CC3000_IRQ   3
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10

Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT, SPI_CLOCK_DIV2); // you can change this clock speed

SocketIOClient client;

void ondata(SocketIOClient client, char *data) {
  Serial.println(F("Incoming data!"));
  Serial.println(data);
  Serial.println(F("----------"));
}

void setup() {
  InitializeCC30000();

  client.setDataArrivedDelegate(ondata);
  if (!client.connect(cc3000, "example.com", 80)) {
    Serial.println(F("Not connected."));
  }
}

void loop() {
  client.monitor(cc3000);
  client.sendEvent("info", "foobar");
  delay(2000);
}

void InitializeCC30000(void){
  // initialise the module
  Serial.println(F("Initializing..."));

  // initialize CC3000 chip
  if (!cc3000.begin()) {
    Serial.println(F("Couldn't begin()! Check your wiring?"));
    while(1);
  }

  // optional SSID scan
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Failed!"));
    while(1);
  }

  Serial.println(F("Connected!"));

  // wait for DHCP to complete
  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP()) {
    delay(100);
  }  
}
```

It should be fairly easy from then to communicate with the Node backend. Just run:

```
npm install socket.io@9.0 && npm install express@3.0
```

Though those might be the old versions of the libraries, they are the currently supported ones because of a Socket.IO protocol change in 1.0. You can try getting Express to work at version 4.0, but I have not tried to test it yet.

An example Node server might look like:

```js
var io = require('socket.io');
var express = require('express');

var app = express(),
    server = require('http').createServer(app),
    io = io.listen(server);

server.listen(80);

io.sockets.on('connection', function (socket) {
  socket.emit('yo', { hello: 'world' });
  socket.on('info', function (data) {
    console.log(data);
  });
});

// serve HTML files in the `public` directory.
app.use(express.static(__dirname + '/public'));
```
