#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "secrets.h" // TODO: add WiFi password at `include/secrets.h`

WiFiClient wifi;
PubSubClient mqtt(wifi);

void connectMqtt() {
  while (!mqtt.connected()) {
    Serial.print("MQTT connecting...");
    if (mqtt.connect(NETPIE_CLIENT_ID, NETPIE_TOKEN, NETPIE_SECRET)) {
      Serial.println("connected");
    } else {
      Serial.printf("failed, rc=%d\n", mqtt.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);  // station mode: join a network, don't create one
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nIP: ");
  Serial.println(WiFi.localIP());
  mqtt.setServer("mqtt.netpie.io", 1883);
}


void loop() {
  connectMqtt();  // reconnects if dropped
  mqtt.loop();    // keeps the connection alive; must run often
}