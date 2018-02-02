/****************************************
 * Include Libraries
 ****************************************/
 
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <stdio.h>  


/****************************************
 * Define Constants
 ****************************************/

#define WIFISSID "...." // Put your WifiSSID here
#define PASSWORD "...." // Put your wifi password here
#define TOKEN "...." // Put your Ubidots' TOKEN
#define VARIABLE_LABEL "...." // Assing the variable label
#define DEVICE_LABEL "...." // Assig the device label
#define MQTT_CLIENT_NAME "...." // MQTT client Name

char mqttBroker[] = "things.ubidots.com";
char payload[700];
char topic[150];

// Space to store values to send
char str_temp[6];
char str_lat[6];
char str_lng[6];


/****************************************
 * Initializate constructors for objects
 ****************************************/

ESP8266WiFiMulti WiFiMulti;
WiFiClient ubidots;
PubSubClient client(ubidots);


/****************************************
 * Auxiliar Functions
 ****************************************/
 
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
} 

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    
    // Attempt to connect
    if (client.connect(MQTT_CLIENT_NAME, TOKEN,"")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);
    }
  }
}


/****************************************
 * Main Functions
 ****************************************/
 
void setup() {
    Serial.begin(115200);
    pinMode(A0, INPUT);
    WiFiMulti.addAP(WIFISSID, PASSWORD);
    Serial.println();
    Serial.println();
    Serial.print("Wait for WiFi... ");

    while(WiFiMulti.run() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    client.setServer(mqttBroker, 1883);
    client.setCallback(callback);
}

void loop() {

    if (!client.connected()) {
      reconnect();

      // Subscribes for getting the value of the control variable in the temperature-box device
      char topicToSubscribe[200];
      sprintf(topicToSubscribe, "%s", ""); // Cleans the content of the char
      sprintf(topicToSubscribe, "%s%s", "/v1.6/devices/", "temperature-box");
      sprintf(topicToSubscribe, "%s/%s/lv", topicToSubscribe, "control");
      client.subscribe(topicToSubscribe);
    }
    
    // Values to send
    float temperature = random(0, 9);
    float lat = 6.101;
    float lng= -1.293;
    
    /* 4 is mininum width, 2 is precision; float value is copied onto str_temp*/
    dtostrf(temperature, 4, 2, str_temp);
    dtostrf(lat, 4, 2, str_lat);
    dtostrf(lng, 4, 2, str_lng);

    sprintf(topic, "%s", ""); // Cleans the topic content
    sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);

    sprintf(payload, "%s", ""); // Cleans the payload content
    sprintf(payload, "{\"%s\":", VARIABLE_LABEL); // Adds the variable label   
    sprintf(payload, "%s {\"value\": %s", payload, str_temp); // Adds the value
    sprintf(payload, "%s, \"context\":{\"lat\": %s, \"lng\": %s}", payload, str_lat, str_lng); // Adds coordinates
    sprintf(payload, "%s } }", payload); // Closes the dictionary brackets

    client.publish(topic, payload);
    client.loop();
    delay(1000);
}