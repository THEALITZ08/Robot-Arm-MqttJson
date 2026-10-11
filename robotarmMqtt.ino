#include<ESP32Servo.h>
#include<PubSubClient.h>
#include<ArduinoJson.h>
#include<WiFi.h>
Servo kanan;
Servo kiri;
Servo capit;
Servo bawah;
int SudutSekarang[4] = {90,90,90,90};
int SudutTarget[4] = {90,90,90,90};
const char* ssid = "cihuyy";
const char*password = "123456789";
const char* mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;
const char* topic_kanan = "robot/servo/kanan";
const char* topic_kiri = "robot/servo/kiri";
const char* topic_capit = "robot/servo/capit";
const char* topic_bawah= "robot/servo/bawah";
const char* status_kanan = "robot/kanan/status";
const char* status_kiri = "robot/kiri/status";
const char* status_capit = "robot/capit/status";
const char* status_bawah = "robot/bawah/status";
WiFiClient espclient;
PubSubClient client(espclient);
const long intervalWiFi = 3000;
const long intervalMqtt = 2000;
const long intervalServo = 30;
unsigned long Servo = 0;
unsigned long Mqtt = 0;
unsigned long wifi = 0;

void connectWiFi() {
if(WiFi.status() == WL_CONNECTED) return;
unsigned long now = millis();
if(now - wifi >= intervalWiFi) {
Serial.println("terputus");
WiFi.mode(WIFI_STA);
WiFi.begin(ssid,password);
wifi = now;
}
}
void connectMqtt() {
if(client.connected()) return;
String clientID = "ESP" + WiFi.macAddress();
unsigned long sekarang= millis();
if(sekarang - Mqtt >= intervalMqtt) {
if(client.connect(clientID.c_str())) {
Serial.println("BERHASIL!! : ");
Serial.print(WiFi.macAddress());
client.subscribe(topic_kanan);
client.subscribe(topic_kiri);
client.subscribe(topic_capit);
client.subscribe(topic_bawah);
client.publish(status_kanan, "lengan kanan siap");
client.publish(status_kiri, "lengan kiri siap");
client.publish(status_capit, "capit siap");
client.publish(status_bawah, "bawah siap");


} else {
Serial.println("Failed RC : ");
Serial.print(client.state());
}
Mqtt = sekarang;
}
}
void mqttcallback(char*topic, byte*payload, unsigned int length) {
char message[length + 1];
memcpy(message,payload,length);
message[length] = '\0';
StaticJsonDocument<450> doc;
DeserializationError error = deserializeJson(doc, message);
if(error) return;
if(strcmp(topic, topic_kanan) ==0) {
if(doc.containsKey("state")) {
int sudutKanan = doc["state"];
if(sudutKanan >=0 && sudutKanan <=180) {
SudutTarget[0] = sudutKanan;
}
}
}
else if(strcmp(topic, topic_kiri)==0) {
if(doc.containsKey("state")) {
int SudutKiri = doc["state"];
if(SudutKiri >=0 && SudutKiri <=180) {
SudutTarget[1] = SudutKiri;
}
}
}
else if(strcmp(topic, topic_capit)==0) {
if(doc.containsKey("state")) {
int SudutCapit = doc["state"];
if(SudutCapit >=0 && SudutCapit <=180) {
SudutTarget[2] = SudutCapit;
}
}
}
else if(strcmp(topic, topic_bawah)==0) {
if(doc.containsKey("state")) {
int SudutBawah = doc["state"];
if(SudutBawah >=0 && SudutBawah <=180) {
SudutTarget[3] = SudutBawah;
}
}
}

}
void updateServo() {
unsigned long update = millis();
if(update - Servo >= intervalServo) {
for(int i=0; i < 4; i++) { 
if(SudutTarget[i] > SudutSekarang[i]) 
SudutSekarang[i]++;
else if(SudutTarget[i] < SudutSekarang[i]) 
SudutSekarang[i]--;

}
kanan.write(SudutSekarang[0]);
kiri.write (SudutSekarang[1]);
capit.write(SudutSekarang[2]);
bawah.write(SudutSekarang[3]);
Servo = update;
}
}
void setup() {
Serial.begin(115200);
kanan.attach(13);
kiri.attach(14);
capit.attach(25);
bawah.attach(26);
kanan.write(90);
kiri.write(90);
capit.write(90);
bawah.write(90);
connectWiFi();
client.setServer(mqtt_server, mqtt_port);
client.setCallback(mqttcallback);
connectMqtt();
}
void loop() {
if(WiFi.status() != WL_CONNECTED) {
  connectWiFi();
}
if(!client.connected()) {
connectMqtt();
} else{
client.loop();
}
updateServo();
}
