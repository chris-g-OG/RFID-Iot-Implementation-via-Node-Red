#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h> 
#include <TimeLib.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include "../lib/NTPClient_T/NTPClient.h"
//#include <NTPClient.h>

// Network credentials
const char* ssid = "WIFI_Name";
const char* password = "WIFI_Password";

// MQTT broker IP address
const char* host = "Host_IP"; // MQTT broker IP address

WiFiClient espClient; // Create WiFi client
PubSubClient client(espClient); // Create MQTT client

// NTP Server
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7200;
const int   daylightOffset_sec = 3600;

WiFiUDP ntpUDP; // Create UDP instance
NTPClient timeClient(ntpUDP, ntpServer, gmtOffset_sec, daylightOffset_sec); // Create NTP client


#define SS_PIN D8 // SDA pin of the MFRC522
#define RST_PIN D1 // RST pin of the MFRC522
MFRC522 mfrc522(SS_PIN, RST_PIN);   

String content = ""; // Card ID content
//bool auth_value = false;
String payload = "";

StaticJsonDocument<200> doc;

time_t getNtpTime();
void setup_wifi();
void setup_MQTT();
void callback(char* topic, byte* message, unsigned int length);

void setup() {

    Serial.begin(115200);
    
    setup_wifi();
    Serial.println(WiFi.localIP());
    SPI.begin();
    mfrc522.PCD_Init();             
    Serial.println("RFID reader ready");

    timeClient.begin();
    // Set the time provider    
    setSyncProvider(getNtpTime);

    client.setServer(host, 1883);

    setup_MQTT();
    Serial.println("Connected to MQTT broker");
    
    client.setCallback(callback); 
    client.subscribe("reader_event"); 

}
void loop() {

    client.loop();

    if (WiFi.status() != WL_CONNECTED) setup_wifi();

    if (!client.connected()) setup_MQTT();

    //  if (!client.connected()) {
    //      if (client.connect("ESP8266Client")) {
    //     //client.subscribe("topic/test");
    //     //Serial.println("Subscribed to topic/test");
    //     }
    //  }

    // Update the time
    timeClient.update();
    setTime(timeClient.getEpochTime()); // Set the time

    // Serial.println("Approximate your card to the reader...");

    content = "";
    payload = "";
    //state = "";

    doc.clear(); // Clear the JSON document
          
    if (!mfrc522.PICC_IsNewCardPresent()) return ;          
    
    if (!mfrc522.PICC_ReadCardSerial()) return ;
    //mfrc522.PICC_DumpToSerial(&(mfrc522.uid));

    Serial.print("U-ID tag :");   
    for (byte i = 0; i < mfrc522.uid.size; i++) 
    {
        content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ")); // Add space
        content.concat(String(mfrc522.uid.uidByte[i], HEX));  // Add HEX value
    }
    //Get the time stamp of the card read
    ///String time_stamp = timeClient.getFormattedDate();  
    //String time_stamp = timeClient.getFormattedTime() + " " + timeClient.getFormattedDate();
    
    unsigned long time_stamp = timeClient.getEpochTime();

    Serial.println(content + "| At : " + time_stamp);

    content.toUpperCase();
    //String payload = time_stamp + content;
    
    String state = "in";
    
    doc["State"] = state; // in or out
    doc["content"] = content;
    doc["time_stamp"] = time_stamp;

    
    serializeJson(doc, payload);
    Serial.println(payload);
    client.publish("card_event", payload.c_str()); 
    Serial.println("topic sent");
    Serial.println();

    delay(2000);
    
}
void setup_wifi() {
    delay(10);
    // Connect to WiFi network
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi : ");
}
void setup_MQTT() {
    int connectionAttempt = 0;
    while (!client.connected()) {
        if (client.connect("ESP32Client")) {
        } else {
        Serial.println("Failed to connect to MQTT broker. Trying again in 5 seconds...");
        connectionAttempt++;
        delay(5000);
        if (connectionAttempt >= 5) {
            Serial.println("Failed to connect to MQTT broker. Exiting program...");
            return;
        }
        }
    }
}
time_t getNtpTime()
{
      while (timeClient.update() == 0) {
            timeClient.forceUpdate();
        }
        return timeClient.getEpochTime();
}   
void callback(char* topic, byte* message, unsigned int length) {
    
    if (String(topic) == "reader_event") {
        Serial.print("Message arrived on topic: ");
        Serial.print(topic);
        Serial.print(". Message: ");
        String messageTemp;
        
        for (int i = 0; i < length; i++) {
            Serial.print((char)message[i]);
            messageTemp += (char)message[i];
        }
        //Serial.println();
        
    }
}