/*
    Program that check temperature and send data to thingspeak.
*/

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS D4

#ifndef STASSID
#define STASSID "YOUR_SSID"
#define STAPSK  "YOUR_PASSWORD"
#define HOST "api.thingspeak.com"
#define PORT 80
#define AWS_HOST "YOUR_AWS_INSTANCE_PUBLIC_DOMAIN"
#define AWS_PORT 3000
#define WRITE_API_KEY "YOUR_THINGSPEAK_WRITE_API_KEY"
#endif

const char* ssid     = STASSID;
const char* password = STAPSK;

const char* host = HOST;
const uint16_t port = PORT;

const char* aws_host = AWS_HOST;
const uint16_t aws_port = AWS_PORT;

ESP8266WiFiMulti WiFiMulti;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(115200);

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
  unsigned long loop_start = millis();
  
   /* Get temperature. */
  float celsius = 0.0;
  sensors.requestTemperatures();
  celsius = sensors.getTempCByIndex(0);
  Serial.print("Current temperature: ");
  Serial.println(celsius);

  /* Send data to Thingspeak. */
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
  
  String req = "GET /update?api_key=" + String(WRITE_API_KEY) + "&field1=" + String(celsius) + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n";
  client.print(req);

  Serial.println("closing connection");
  client.stop();

  delay(1000);

  /* Send data to AWS. */
  Serial.println("Connecting to aws ... ");
  if (!client.connect(aws_host, aws_port)){
    Serial.println("Can not connect to AWS!");
    Serial.println("Wait 5 sec ... ");
    delay(5000);
    return;
  }

  req = "GET /data?temp=" + String(celsius) + " HTTP/1.1\r\n" + 
        "Host: " + aws_host + "\r\n" +
        "Connection: close\r\n\r\n";
  client.print(req);
  Serial.print(req);

  Serial.println("closing aws connection");
  client.stop();

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
