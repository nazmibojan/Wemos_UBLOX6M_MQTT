#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

#define wifi_ssid "SSID"
#define wifi_password "PASSWD"

#define mqtt_server "IP"
#define mqtt_user "username"
#define mqtt_password "password"

#define latitude_topic "latitude"
#define longitude_topic "longitude"

static const int RXPin = 4, TXPin = 5;
static const uint32_t GPSBaud = 9600;

float latitude , longitude;
byte GPSData;

long lastMsg = 0;   
bool debug = true;

TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  ss.begin(GPSBaud);
}

/*
 * @brief Initialize connection to wifi
*/
void setup_wifi(void)
{
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi OK ");
  Serial.print("=> ESP8266 IP address: ");
  Serial.print(WiFi.localIP());
  Serial.println("");
}

void reconnect() {

  while (!client.connected()) {
    Serial.print("Connecting to MQTT broker ...");
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("OK");
    } else {
      Serial.print("KO, error : ");
      Serial.print(client.state());
      Serial.println(" Wait 5 secondes before to retry");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  if (debug) {
    while (ss.available() > 0) {
//        GPSData = ss.read();
//        Serial.write(GPSData);
        
      if (gps.encode(ss.read())) {
        if (gps.location.isValid()) {
          latitude = gps.location.lat();
          longitude = gps.location.lng();
        }
      }    
    }
      
    Serial.print("Latitude : ");
    Serial.print(latitude);
    Serial.print(" | Longitude : ");
    Serial.println(longitude);     
  }

  long now = millis();
  // Send a message every second
  if(now -  lastMsg > 1000 * 1){
    lastMsg = now;
      client.publish(latitude_topic, String(latitude).c_str(), true);
      client.publish(longitude_topic, String(longitude).c_str(), true);
  }  
}
