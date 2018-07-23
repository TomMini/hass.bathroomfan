// Get ESP8266 going with Arduino IDE
// - https://github.com/esp8266/Arduino#installing-with-boards-manager
// Required libraries (sketch -> include library -> manage libraries)
// - PubSubClient by Nick ‘O Leary
// - DHT sensor library by Adafruit

#include <stdio.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define wifi_ssid "<WIFI_SSID>"
#define wifi_password "<WIFI_PASSWORD>"

#define mqtt_server "<MQTT_SERVER>"
#define mqtt_user "<MQTT_USERNAME>"
#define mqtt_password "<MQTT_PASSWORD>"
#define mqtt_clientid "<MQTT_CLIENTID>"

#define topic_state "homeassistant/sensor/bathroom/state"
#define topic_set "homeassistant/switch/bathroom_fan/set"
#define topic_temp_config "homeassistant/sensor/bathroomT/config"
#define topic_hum_config "homeassistant/sensor/bathroomH/config"
#define temp_config "{\"name\":\"Temperature\",\"state_topic\":\"homeassistant/sensor/bathroom/state\",\"unit_of_measurement\":\"°C\",\"value_template\":\"{{value_json.temperature}}\"}"
#define hum_config "{\"name\":\"Humidity\",\"state_topic\":\"homeassistant/sensor/bathroom/state\",\"unit_of_measurement\":\"%\",\"value_template\":\"{{value_json.humidity}}\"}"

#define DHT_TYPE DHT22
#define DHT_PIN D3
#define DHT_DEBUG

#define RELAY_PIN D1

#define MOTION_PIN D2

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHT_PIN, DHT_TYPE, 11); // 11 works fine for ESP8266

long lastMsg = 0;
float temp = 0.0;
float temp_diff = 0.1;
float hum = 0.0;
float hum_diff = 1.0;
int motion = 0;
bool reconnecting = false;

void setup()
{
  Serial.begin(115200);
  pinMode(RELAY_PIN, INPUT);
  setup_relay();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(mqtt_callback);
  dht.begin();// initialize temperature sensor  
}

void loop()
{
  if (!client.connected()) {
    if (reconnecting == true) {
      return;
    }

    reconnecting = true;
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000)
  {
    lastMsg = now;

    float newTemp = dht.readTemperature();
    float newHum = dht.readHumidity();
    int newMotion = digitalRead(MOTION_PIN);
    bool changed = false;

    if (checkBound(newTemp, temp, temp_diff))
    {
      temp = newTemp;
      changed = true;
      Serial.print("New temperature:");
      Serial.println(String(temp).c_str());
    }

    if (checkBound(newHum, hum, hum_diff))
    {
      hum = newHum;
      changed = true;
      Serial.print("New humidity:");
      Serial.println(String(hum).c_str());
    }

    if (motion != newMotion) {
      motion = newMotion;
      changed = true;
      Serial.print("Motion changed:");
      Serial.println(String(motion).c_str());
    }

    if (changed == true) 
    {
      String m;
      if (motion == 1) {
        m = "ON";
      } else {
        m = "OFF";
      }
      
      String payload = "{ \"temperature\": " + String(temp) + ",\"humidity\": " + String(hum) + ",\"motion\": \"" + m + "\"}";
      client.publish(topic_state, payload.c_str());
    }
  }
}

void setup_wifi()
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) 
{
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
 
  Serial.print("Message:");
  for (int i = 0; i < length; i++) 
  {
    Serial.print((char)payload[i]);
  }
 
  Serial.println();
  Serial.println("-----------------------"); 

  if (string_equals(payload, length, "ON"))
  {
    digitalWrite(RELAY_PIN, HIGH);
  }
  else if (string_equals(payload, length, "OFF"))
  {
    digitalWrite(RELAY_PIN, LOW);
  }
}

bool string_equals(byte* data, unsigned int length, const char* str)
{
  if (strlen(str) != length) 
  {
    return false;
  }

  for (int i = 0; i < length; i++)
  {
    if (data[i] != str[i])
    {
      return false;
    }
  }

  return true;
}

void setup_relay()
{
  pinMode(RELAY_PIN, OUTPUT);
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if (client.connect(mqtt_clientid, mqtt_user, mqtt_password))
    {
      Serial.println("connected");
      reconnecting = false;
      autoconfigure_hass();
      client.subscribe(topic_set);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void autoconfigure_hass()
{
  client.publish(topic_temp_config, temp_config, true);
  client.publish(topic_hum_config, hum_config, true);
}

bool checkBound(float newValue, float prevValue, float maxDiff)
{
  return !isnan(newValue) &&
         (newValue < prevValue - maxDiff || newValue > prevValue + maxDiff);
}
