#include <Wire.h>

#define I2C_SERVER_ADDR 18

#include <WiFiNINA.h>
#include <MQTT.h>

//wifi settings
const char ssid[] = "TUvisitor";
const char pass[] = "";

//mqtt settings
const char mqtt_clientID[] = "Arduino Nano IOT";
const char mqtt_username[] = "d1e4f26e";
const char mqtt_password[] = "7b506b13dfb5a6fe";
WiFiClient wifi;
MQTTClient mqtt;

String data = "";

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
  if (data.length() + payload.length() >= 128) {
    Serial.println("Dropped data");
  } else {
    data = data + payload + "\n";
  }
}

void connectWiFi() {
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("+");
    delay(1000);
  }
}

void connectMQTT() {
  while (!mqtt.connect(mqtt_clientID, mqtt_username, mqtt_password)) {
    Serial.print("x");
    delay(1000);
  }
}

void requestEvent() {
  if (data.length() > 0) {
    char c = data.charAt(0);
    data = data.substring(1, data.length());
    Wire.write(c);
  } else {
    Wire.write((char) 0);
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  for (int i = 0; i < 10 && !Serial; i++) {
    delay(100);
  }
  
  Serial.println("Wire Init");
  Wire.begin(I2C_SERVER_ADDR);
  Wire.onRequest(requestEvent);
  Serial.println();
  
  Serial.print("WiFi Init ");
  WiFi.begin(ssid);
  connectWiFi();
  Serial.println();
  
  Serial.println("MQTT Init ");
  mqtt.begin("broker.shiftr.io", wifi);
  mqtt.onMessage(messageReceived);
  connectMQTT();
  Serial.println();

  Serial.println("Connected!");
  mqtt.subscribe("/cmd");
}

void loop() {
  mqtt.loop();
  if (!mqtt.connected()) {
    Serial.print("Reconnecting ");
    connectWiFi();
    connectMQTT();
    Serial.println(" Done");
  }
  
  Serial.println("Me is nanoboi");
  mqtt.publish("/ping", "nano");

  delay(250);
}
