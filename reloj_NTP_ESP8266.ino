#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUDP.h>
#include <ArduinoJson.h>

#include "TM1637.h"
//#include <TM1637Display.h>



TM1637 tm1637(0, 2);              // CLK, DIO (D6, D5)
//TM1637Display display(0, 2);


#define NTP_OFFSET   0 * 60 * 60 // In seconds
#define NTP_INTERVAL 60 * 1000 // In miliseconds
#define NTP_ADDRESS  "es.pool.ntp.org"

#define brightness 4

#define JSON_BUFF_DIMENSION 2500

const int LED = 0;
char ssid[] = "Wireless-N";             //  your network SSID (name)
char pass[] = "z123456z";         // your network password
//char ssid[] = "WLAN_BF";             //  your network SSID (name)
//char pass[] = "Z404A03CF9CBF";         // your network password


byte hour, minute, second;
boolean point;
String hora;
String minuto;
const long timerUpdate = 60L;  //medio minuto
const long timer5seg = 20 ;    //5 seg
bool  flagUpdate = false;
bool flagUpdate5seg = false;
bool flag1hora = false;
bool tarea = false;
int valorH;
int valorM;
int temperatura;
String text;
const char server[] = "api.openweathermap.org";
int jsonend = 0;
boolean startJson = false;
int timezone;


// A UDP instance to let us send and receive packets over UDP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

WiFiClient client;


//************************************************************************************************
void TimingISR()
{
  static long cntTemp = 0L;
  static long cntT5seg = 0L;
  static long cntT1hora = 0L;


  //digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

  point = !point;

  if (++cntT1hora > 7200 )  //7200
  {
    flag1hora = true;

    cntT1hora = 0;
  }

  if (++cntTemp > timerUpdate)
  {
    flagUpdate = true;

    cntTemp = 0L;
  }

  if (++cntT5seg > timer5seg)
  {
    flagUpdate5seg = true;

    cntT5seg = 0L;
  }

  timer0_write(ESP.getCycleCount() + 40000000L); // 80MHz == 1sec

}
//*********************************************************************************************************

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);

  tm1637.set(5);
  tm1637.init(D4036B);               ///tm1637

  //display.setBrightness(0x0f);
  // display.showNumberDec(8888);  //debug

  Serial.println();

  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  WiFi.mode(WIFI_STA);

  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print("*");

    if (++timeout > 100)
    {
      Serial.println("Sin Conexion WIFI");
      ESP.reset();
    }
  }


Serial.println("");

Serial.println("WiFi connected");
Serial.println("IP address: ");
Serial.println(WiFi.localIP());


reqOpenweather();


timeClient.begin();
timeClient.setTimeOffset(timezone);
//timeClient.setTimeOffset(3600);
dameHora();


noInterrupts();
timer0_isr_init();
timer0_attachInterrupt(TimingISR);
timer0_write(ESP.getCycleCount() + 40000000L); // 80MHz == 1sec
interrupts();
}

//*******************************************************************************************************
void reqOpenweather() {


  HTTPClient http;  //Object of class HTTPClient
  http.begin("http://api.openweathermap.org/data/2.5/weather?q=sevilla,ES&APPID=c2ecf0a83c555c2054704fd94ff29f9e&units=metric");
  int httpCode = http.GET();
  //Check the returning code
  if (httpCode > 0) {

    Serial.println(http.getString());
    const size_t capacity = JSON_ARRAY_SIZE(1) + 2 * JSON_OBJECT_SIZE(1) + 2 * JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(6) + JSON_OBJECT_SIZE(14) + 290;
    DynamicJsonDocument doc(capacity);

    DeserializationError error = deserializeJson(doc, http.getString());

    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
    }

    JsonObject main = doc["main"];
    temperatura = main["temp"];
    timezone = doc["timezone"];

    Serial.println(temperatura + "," + timezone);


    http.end();
  }

  else {
    // if no connction was made:
    Serial.println("connection failed");
    return;
  }
}

//*********************************************************************************************************
void dameHora() {

  timeClient.update();
  String formattedTime = timeClient.getFormattedTime();
  // Serial.println(formattedTime);

  int split = formattedTime.indexOf(":");
  hora = formattedTime.substring(0, split);
  minuto = formattedTime.substring(split + 1, split + 3);
  // Serial.println("Hora Actual: " + hora + ":" + minuto);

  valorH = String(hora).toInt();
  valorM =  String(minuto).toInt();

}



//*******************************************************************************************************

void loop() {

  int8_t TimeDisp[4];

  if (flagUpdate ) {  //cada minuto

    dameHora();


    flagUpdate = false;

  }

  if (flag1hora) {

    reqOpenweather();

    flag1hora = false;
  }

  if (flagUpdate5seg) {

    //  Serial.println("5 seg");
    tarea = !tarea;
    flagUpdate5seg = false;
  }

  switch (tarea) {

    case 0:

      tm1637.point(point);
      TimeDisp[0] = valorH / 10;
      TimeDisp[1] = valorH % 10;
      TimeDisp[2] = valorM  / 10;
      TimeDisp[3] = valorM  % 10;

      tm1637.display(TimeDisp);
      break;
    case 1:
      tm1637.point(0);
      TimeDisp[0] = temperatura / 10;
      TimeDisp[1] = temperatura % 10;
      TimeDisp[2] = 16;
      TimeDisp[3] = 12;  //es el indice en el array static int8_t TubeTab[] en TM1637.cpp
      tm1637.display(TimeDisp);
      break ;

    default: break;
  }

}
