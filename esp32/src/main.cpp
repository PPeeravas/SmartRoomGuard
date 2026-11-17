#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "secrets.h" // TODO: add WiFi password at `include/secrets.h`

WiFiClient wifi;
PubSubClient mqtt(wifi);
void onMessage(char* topic, byte* payload, unsigned int length);
void onCommand(const String& cmd);
void setState(const char* s);
void connectMqtt();

// ---- Fake STM32 (until the Nucleo is ready) ----
String state = "DISARMED";
int door = 0;
bool changed = true;
unsigned long armingStartMs = 0, lastPublishMs = 0;

void readMonitor() ;

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


void loop() {
  connectMqtt();
  mqtt.loop();
  readMonitor();

  if (state == "ARMING" && millis() - armingStartMs >= 10000) setState("ARMED");

  if (changed || millis() - lastPublishMs >= 2000) {  // on change + every 2 s
    changed = false;
    lastPublishMs = millis();
    char json[96];
    snprintf(json, sizeof json, "{\"data\":{\"state\":\"%s\",\"door\":%d,\"dist\":%d}}",
             state.c_str(), door, 100);
    mqtt.publish("@shadow/data/update", json);
  }
}



void setState(const char* s) { state = s; changed = true; }

void onCommand(const String& cmd) {
  if (cmd == "ARM" && state == "DISARMED") { setState("ARMING"); armingStartMs = millis(); }
  else if (cmd == "DISARM" && (state == "ARMED" || state == "ALARM" || state == "SILENCED")) setState("DISARMED");
  else if (cmd == "SILENCE" && state == "ALARM") setState("SILENCED");
}

void readMonitor() {  // type o = door open, c = door closed
  while (Serial.available()) {
    char c = Serial.read();
    if (c == 'o') { door = 1; changed = true; if (state == "ARMED") setState("ALARM"); }
    if (c == 'c') { door = 0; changed = true; }
  }
}

void onMessage(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
  Serial.printf("%s -> %s\n", topic, msg.c_str());
  onCommand(msg);
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