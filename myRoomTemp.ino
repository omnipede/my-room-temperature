/*
    Program that check temperature and send data to thingspeak.
*/

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS D4

#ifndef STASSID
#define STASSID "Your_SSID_please"
#define STAPSK  "Your_passwd_please"
#define HOST "api.thingspeak.com"
#define PORT 80
#define WRITE_API_KEY "Your_write_api_key_please"
#endif

const char* ssid     = STASSID;
const char* password = STAPSK;

const char* host = HOST;
const uint16_t port = PORT;

ESP8266WiFiMulti WiFiMulti;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(115200);

  // We start by connecting to a WiFi network
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, password);

  Serial.println();
  Serial.println();
  Serial.print("Wait for WiFi... ");

  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  sensors.begin();
  delay(500);
}


void loop() {
  float celsius = 0.0;
  unsigned long loop_start = millis();
  Serial.print("connecting to ");
  Serial.print(host);
  Serial.print(':');
  Serial.println(port);

  WiFiClient client;

  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    Serial.println("wait 5 sec...");
    delay(5000);
    return;
  }

  /* Get temperature. */
  sensors.requestTemperatures();
  celsius = sensors.getTempCByIndex(0);
  Serial.print("Current temperature: ");
  Serial.println(celsius);
  
  /* Send req to server. */
  String req = "GET https://api.thingspeak.com/update?api_key=" + String(WRITE_API_KEY) + "&field1=" + String(celsius) + "\r\n";
  client.print(req);

  unsigned long timeout = millis();
  while(client.available() == 0){
    if(millis() - timeout > 5000){
      Serial.println("Timeout!");
      client.stop();
      return;
    }
  }

  Serial.println("closing connection");
  client.stop();

  /* Delay time is 1 min = 60,000msec. */
  /* If loop took less than 1 min, then wait until 1 min pass. */
  unsigned long loop_time = millis() - loop_start;
  if(loop_time < 60000){
    unsigned long delay_time = 60000 - loop_time;
    Serial.print("Time taken: ");
    Serial.println(loop_time);
    Serial.print("Wait for: ");
    Serial.println(delay_time);
    delay(delay_time);
  }
}
