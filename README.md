# SocketIO Arduino Client for Adafruit cc3000 instead of Ethernet Shield.

This was based off Bill Roy’s Ethernet shield socket.io version which I then edited to support the Adafruit cc3000 board.


Kevin's documentation is reproduced hereinafter, with changes as needed.


## Caveats

This library doesn't support every inch of the Websocket spec, most notably the use of a Sec-Websocket-Key.  Also, because Arduino doesn't support SSL, this library also doesn't support the use of Websockets over https.  If you're interested in learning more about the Websocket spec I recommend checking out the [Wikipedia Page](http://en.wikipedia.org/wiki/WebSocket).  Now that I've got that out of the way, I've been able to successfully use this to connect to several hosted Websocket services, including: [echo.websocket.org](http://websocket.org/echo.html) and [pusherapp.com](http://pusherapp.com).

## Installation instructions

Clone this repo into your Arduino Sketchbook directory under libraries, then restart the Arduino IDE so that it notices the new library.  Now, under File\Examples you should see SocketIOClient.  

## How To Use This Library
  
  #define WLAN_SSID       “ssid”           // cannot be longer than 32 characters!
  #define WLAN_PASS       “password”
  #define WLAN_SECURITY   WLAN_SEC_WPA2

 Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT, SPI_CLOCK_DIV2); // you can change this clock speed
 
 SocketIOClient client;

void setup() {
  InitializeCC30000();
  client.setDataArrivedDelegate(ondata);
  if (!client.connect(cc3000,"smartgaragenode.herokuapp.com",80)) Serial.println(F("Not connected."));
}

void loop() {
  client.monitor();
  client.send(“ping”);
  delay(2000);
}

 void InitializeCC30000(void)
  {
    // Initialise the module
    LogLine(F("\nInitializing..."));
    
    if (!cc3000.begin())
    {
      LogLine(F("Couldn't begin()! Check your wiring?"));
      while(1);
    }
    
    // Optional SSID scan
    if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
      LogLine(F("Failed!"));
      while(1);
    }
    
    LogLine(F("Connected!"));
    /* Wait for DHCP to complete */
    LogLine(F("Request DHCP"));
    while (!cc3000.checkDHCP())
    {
      delay(100); // ToDo: Insert a DHCP timeout!
    }  
}

void dataArrived(WebSocketClient client, char *data) {
  Serial.println("Data Arrived: " + data);
}
