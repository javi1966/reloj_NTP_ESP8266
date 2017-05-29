


//http://myshop.biz.ua/index.php?route=information/news/info&news_id=21

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "TM1637.h"
//#include <TM1637Display.h>
#include <Ticker.h>

Ticker flipper;
TM1637 tm1637(0, 2);              // CLK, DIO (D6, D5)
//TM1637Display display(0, 2);
#define GMT 2                       // INVIERNO GMT=1 ,VERANO GMT=2
#define brightness 4

char ssid[] = "Wireless-N";             //  your network SSID (name)
char pass[] = "z123456z";         // your network password
//char ssid[] = "WLAN_BF";             //  your network SSID (name)
//char pass[] = "Z404A03CF9CBF";         // your network password


byte hour, minute, second;
boolean point;

unsigned int localPort = 2390;      // local port to listen for UDP packets

/* Don't hardwire the IP address or we won't get the benefits of the pool. *  Lookup the IP address for the host name instead */
//IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
IPAddress timeServerIP; // time.nist.gov NTP server address
const char* ntpServerName = "time.nist.gov";
//const char* ntpServerName="ntp.i2t.ehu.es";
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

void flip() {
  point = !point;
  second++;
  if (second > 59) {
    second = 0;
    minute++;
  }
  if (minute > 59) {
    minute = 0;
    hour++;
  }
  if (hour > 23) {
    hour = 0;
  }

}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}


void oldloop()
{
  //get a random server from the pool
  WiFi.hostByName(ntpServerName, timeServerIP);

  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1500);

  int cb = udp.parsePacket();
  if (!cb) {
    Serial.println("no packet yet");
     __asm__("nop\n\t");  
  }
  else {
    Serial.print("packet received, length=");
    Serial.println(cb);
    // We've received a packet, read the data from it
    udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    Serial.print("Seconds since Jan 1 1900 = " );
    Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    // print Unix time:
    Serial.println(epoch);

   
    epoch = epoch + GMT * 3600 ;

    hour = (epoch  % 86400L) / 3600;
    minute = (epoch  % 3600) / 60;
    second = epoch % 60;
 
   
    // print the hour, minute and second:
    Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
    Serial.print((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
    Serial.print(':');
    if ( ((epoch % 3600) / 60) < 10 ) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
    Serial.print(':');
    if ( (epoch % 60) < 10 ) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.println(epoch % 60); // print the second
  }
  // wait ten seconds before asking for the time again
 

}


void setup()
{
  Serial.begin(115200);

  tm1637.set(5);
  tm1637.init(D4036B);               ///tm1637
  
  //display.setBrightness(0x0f);
 // display.showNumberDec(8888);  //debug

  flipper.attach(1, flip);          /// Ð¿Ñ€ÐµÑ€Ñ‹Ð²Ð°Ð½Ð¸Ðµ 1 Ñ�ÐµÐº

  Serial.println();
  Serial.println();

  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  WiFi.mode(WIFI_STA);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1500);
    //WiFi.begin(ssid, pass);
    Serial.print("*");
  }
  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());

  oldloop();
}

void loop() {
  if (second == 30) {
    flipper.detach();
    delay(1500);
    oldloop();
    flipper.attach(1, flip);
  }

  int8_t TimeDisp[4];
  tm1637.point(point);
  

  TimeDisp[0] = hour / 10;
  TimeDisp[1] = hour % 10;
  TimeDisp[2] = minute  / 10;
  TimeDisp[3] = minute  % 10;
  //display.showNumberDecEx(hour,0x20,true, 2, 0);
  //display.showNumberDec(minute, true, 2, 2);
 
  tm1637.display(TimeDisp);
}

