#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <TinyGPS++.h>
#include <TimeLib.h>
#include <SoftwareSerial.h>

#define wifi_ssid "SSID"
#define wifi_password "PASS!"

#define mqtt_server   "m16.cloudmqtt.com"
#define mqtt_user     "user"
#define mqtt_password "pass"
#define mqtt_port     1883

#define gps_topic       "location"
#define latitude_topic  "latitude"
#define longitude_topic "longitude"

#define time_offset   3600

static const int RXPin = 4, TXPin = 5;
static const uint32_t GPSBaud = 9600;

//DynamicJsonDocument loc(100);
StaticJsonDocument<100> loc;
JsonArray data;

float latitude , longitude;

long lastMsg = 0;   
bool debug = true;

TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);
byte last_second, Second, Minute, Hour, Day, Month;
int Year;
char Time[]  = "00:00:00";
char loc_output[128];

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  ss.begin(GPSBaud);

  /* Initialize gps JSON data */
  loc["sensor"] = "gps";
  data = loc.createNestedArray("data");
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

  while (ss.available() > 0) {
    if (gps.encode(ss.read())) {
      /* Get time from GPS Module */
      if (gps.time.isValid())
      {
        Minute = gps.time.minute();
        Second = gps.time.second();
        Hour   = gps.time.hour();
      }

      /* Get date drom GPS module */
      if (gps.date.isValid())
      {
        Day   = gps.date.day();
        Month = gps.date.month();
        Year  = gps.date.year();
      }

      // set current UTC time
      setTime(Hour, Minute, Second, Day, Month, Year);
      // add the offset to get local time
      adjustTime(time_offset);

      // update time array
      Time[6] = second() / 10 + '0';
      Time[7] = second() % 10 + '0';
      Time[3]  = minute() / 10 + '0';
      Time[4] = minute() % 10 + '0';
      Time[0]  = hour()   / 10 + '0';
      Time[1]  = hour()   % 10 + '0';

      /* Get GPS location data */
      if (gps.location.isValid()) {
        latitude = gps.location.lat();
        longitude = gps.location.lng();
      }
    }    
  }
    
  long now = millis();
  // Send a message every 10 second
  if(now -  lastMsg > 30000 * 1){
    lastMsg = now;

    if (debug) {
      Serial.print("Latitude : ");
      Serial.print(latitude);
      Serial.print(" | Longitude : ");
      Serial.println(longitude);     
    }

    if (latitude != 0 && longitude != 0) {
      /* Generate JSON packet from gps data */
      loc["timestamp"] = Time;  
      data.add(latitude);
      data.add(longitude);

      /* Produce JSON Document*/
      serializeJson(loc, loc_output);

      /* Publish location data to mqtt broker */
      client.publish(gps_topic, loc_output, true);    
    }  
  }
}
