#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>

#define WIFI_SSID "tesalia"
#define WIFI_PASSWORD "admin2022&"

#define MQTT_HOST IPAddress(192, 168, 1, 150)
#define MQTT_PORT 1883

#define MQTT_PUB_MOV "laboratorio/sensor/movimiento"

float movimiento;

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

unsigned long previousMillis = 0; 
const long interval = 10000;

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("Connected to Wi-Fi.");
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from Wi-Fi.");
  mqttReconnectTimer.detach(); 
  wifiReconnectTimer.once(2, connectToWifi);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void onMqttPublish(uint16_t packetId) {
  Serial.print("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

int buttonState = 0;  
int lastButtonState = 0; 

int inputPin = D7;

void setup() {
  Serial.begin(115200);
  Serial.println();

  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);
  
 mqttClient.onConnect(onMqttConnect);
 mqttClient.onDisconnect(onMqttDisconnect);
 mqttClient.onPublish(onMqttPublish);
 mqttClient.setServer(MQTT_HOST, MQTT_PORT);
 connectToWifi();

  pinMode(inputPin, INPUT);
  Serial.begin(9600);

}

void loop() {
 int buttonState = digitalRead(inputPin);
 if (buttonState != lastButtonState) {
   if (buttonState == HIGH) {
    uint16_t packetIdPub1 = mqttClient.publish(MQTT_PUB_MOV, 1, true, String("ON").c_str());
    Serial.printf("Publishing on topic %s at QoS 1, packetId: %i", MQTT_PUB_MOV, packetIdPub1);
    Serial.println("Te movistes");
  }
  else {
    uint16_t packetIdPub1 = mqttClient.publish(MQTT_PUB_MOV, 1, true, String("OFF").c_str());
    Serial.printf("Publishing on topic %s at QoS 1, packetId: %i", MQTT_PUB_MOV, packetIdPub1);
    Serial.println("No te movistes");
  }
 }
  lastButtonState = buttonState;
}
