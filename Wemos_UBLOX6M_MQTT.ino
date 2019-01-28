#include <TinyGPS++.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

#define wifi_ssid "Mako Brimob"
#define wifi_password "password"

#define mqtt_server "192.168.43.7"
#define mqtt_user "user"
#define mqtt_password "pass"

#define gps_topic "gps"

// Buffer to decode MQTT messages
char message_buff[100];

long lastMsg = 0;   
long lastRecu = 0;
bool debug = true;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(9600);
  pinMode(D2, OUTPUT);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
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

  long now = millis();
  // Send a message every second
  if(now -  lastMsg > 1000 * 1){
    lastMsg = now;
      
    if (debug) {
      Serial.print("Temperature : ");
      Serial.print(t);
      Serial.print(" | Humidity : ");
      Serial.println(h);
    }

    client.publish(gps, String(t).c_str(), true);
  }  
}
