#include "DHT.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "secrets.h"

#define LIQUID_LEVEL_SENSOR_PIN 14
/*--------------------------- DHT SENSOR ---------------------------------------*/
#define DHTPIN 2     // Digital pin connected to the DHT sensor
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
DHT dht(DHTPIN, DHTTYPE);

//Constants
const char* topic = "water_leak/detection"; // mqtt topic to publish messages
const char* ntpServer = "pool.ntp.org";
const char* ssid = SECRET_SSID;   // network SSID (name) 
const char* password = SECRET_WIFI_PASS;   // network password
const char* mqttServer = SECRET_MQTT_HOST;   // mqtt server host
const int mqttPort = SECRET_MQTT_PORT; // mqtt server port


WiFiClient espClient;
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer);
//MQTT client
PubSubClient client(espClient);


void initWiFi() {
  Serial.print("Connecting to WiFi ..");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println("Connected");
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
}

//reconnect to mqtt client
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

String serializeJsonAsString(int temperature, int humidity, int heatIndex, int waterLevel, unsigned long timestamp) {
   String output;
   StaticJsonDocument<96> doc;
   
   doc["waterLevel"] = waterLevel;
   doc["ts"] = timestamp;
   doc["humidity"] = humidity;
   doc["temperature"] = temperature;
   doc["heatIndex"] = heatIndex;
   
   serializeJson(doc, output);
   
   return output;
}

void setup() {
  Serial.begin(115200);
  
  dht.begin();
  initWiFi();
  
  // Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(0);
  
  client.setServer(mqttServer, mqttPort);
  
  delay(2000);
}

void loop() {
   if (!client.connected()) {
     reconnect();
   }
   client.loop();
  
   timeClient.update();
   // Reading temperature or humidity takes about 250 milliseconds!
   // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
   int humidity = round(dht.readHumidity());
   int temperature = round(dht.readTemperature());

   // Check if any reads failed and exit early (to try again).
   if (isnan(humidity) || isnan(temperature)) {
     Serial.println(F("Failed to read from DHT sensor!"));
     return;
   }
   
    // Compute heat index in Celsius (isFahreheit = false)
    int heatIndex = round(dht.computeHeatIndex(temperature, humidity, false));
    unsigned long timestamp = timeClient.getEpochTime();
    
    // Read water level from analog sensor
    int waterLevel = analogRead(LIQUID_LEVEL_SENSOR_PIN);

    // Serialize values as stringified json and publish to mqtt
    const String output = serializeJsonAsString(temperature,humidity,heatIndex,waterLevel,timestamp);

    client.publish(topic,output.c_str());
    
    delay(5000);
}
