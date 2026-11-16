#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "secrets.h" // TODO: add WiFi password at `include/secrets.h`

WiFiClient wifi;
PubSubClient mqtt(wifi);

void onMessage(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
  Serial.printf("%s -> %s\n", topic, msg.c_str());
}

void connectMqtt() {
  while (!mqtt.connected()) {
    Serial.println("MQTT connecting...");
    if (mqtt.connect(NETPIE_CLIENT_ID, NETPIE_TOKEN, NETPIE_SECRET)) {
      Serial.println("connected");
      mqtt.subscribe("@msg/cmd");
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
  mqtt.setCallback(onMessage);
}

unsigned long lastPublishMs = 0;

void loop() {
  connectMqtt();
  mqtt.loop();

  if (millis() - lastPublishMs >= 2000) {  // every 2 s, without blocking
    lastPublishMs = millis();
    mqtt.publish("@shadow/data/update",
                 "{\"data\":{\"state\":\"ARMED\",\"door\":0,\"dist\":676}}");
  }
}
