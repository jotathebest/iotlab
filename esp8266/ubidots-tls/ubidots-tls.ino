/*************************************************************************************************
 * This Example sends harcoded data to Ubidots and serves as example for users that has devices
 * based on ESP8266 chips that we don't support with a library.
 * 
 * This example is given AS IT IS without any warranty
 * 
 * Made by Jose García.
 * 
 * Based on the original HTTP example by Ivan Grokhotkov, 2017.
 * This example is in public domain.
 *************************************************************************************************/

/********************************
 * Libraries included
 *******************************/
 
#define USING_AXTLS
#include <time.h>
#include <ESP8266WiFi.h>

#include <WiFiClientSecureAxTLS.h>
using namespace axTLS;

/********************************
 * Constants and objects
 *******************************/

#ifndef STASSID
#define STASSID "..." // Put here your SSID name
#define STAPSK "..."  // Put here your WiFi password
#endif

const char *HTTPSERVER = "industrial.api.ubidots.com";
const int HTTPPORT = 443;

char* TOKEN = "BBFF-PddZ3rN6XW3si0udL5JOzkWTMoisin"; // Put here your TOKEN

char* DEVICE_LABEL = "http"; // Your Device label

char const * VARIABLE_LABEL_1 = "http-tls";  // Your variable label
char const * USER_AGENT = "ESP8266";
char const * VERSION = "1.0";

const char *ssid = STASSID;
const char *password = STAPSK;

// Root certificate used by api.github.com.
// Defined in "CACert" tab.
extern const unsigned char caCert[] PROGMEM;
extern const unsigned int caCertLen;
extern const unsigned char caCert2[] PROGMEM;
extern const unsigned int caCertLen2;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
WiFiClientSecure clientUbi;
#pragma GCC diagnostic pop

/********************************
 * Auxiliar Functions
 *******************************/

void SendToUbidots(char *payload)
{

  int contentLength = strlen(payload);

  if (clientUbi.connected())
  {
    /* Builds the request POST - Please reference this link to know all the request's structures https://ubidots.com/docs/api/ */
    clientUbi.print(F("POST /api/v1.6/devices/"));
    clientUbi.print(DEVICE_LABEL);
    clientUbi.print(F(" HTTP/1.1\r\n"));
    clientUbi.print(F("Host: "));
    clientUbi.print(HTTPSERVER);
    clientUbi.print(F("\r\n"));
    clientUbi.print(F("User-Agent: "));
    clientUbi.print(USER_AGENT);
    clientUbi.print(F("/"));
    clientUbi.print(VERSION);
    clientUbi.print(F("\r\n"));
    clientUbi.print(F("X-Auth-Token: "));
    clientUbi.print(TOKEN);
    clientUbi.print(F("\r\n"));
    clientUbi.print(F("Connection: close\r\n"));
    clientUbi.print(F("Content-Type: application/json\r\n"));
    clientUbi.print(F("Content-Length: "));
    clientUbi.print(contentLength);
    clientUbi.print(F("\r\n\r\n"));
    clientUbi.print(payload);
    clientUbi.print(F("\r\n"));

    Serial.println(F("Making request to Ubidots:\n"));
    Serial.print("POST /api/v1.6/devices/");
    Serial.print(DEVICE_LABEL);
    Serial.print(" HTTP/1.1\r\n");
    Serial.print("Host: ");
    Serial.print(HTTPSERVER);
    Serial.print("\r\n");
    Serial.print("User-Agent: ");
    Serial.print(USER_AGENT);
    Serial.print("/");
    Serial.print(VERSION);
    Serial.print("\r\n");
    Serial.print("X-Auth-Token: ");
    Serial.print(TOKEN);
    Serial.print("\r\n");
    Serial.print("Connection: close\r\n");
    Serial.print("Content-Type: application/json\r\n");
    Serial.print("Content-Length: ");
    Serial.print(contentLength);
    Serial.print("\r\n\r\n");
    Serial.print(payload);
    Serial.print("\r\n");
  }
  else
  {
    Serial.println("Connection Failed ubidots - Try Again");
  }

  /* Reach timeout when the server is unavailable */
  int timeout = 0;
  while (!clientUbi.available() && timeout < 5000)
  {
    timeout++;
    delay(1);
    if (timeout >= 5000)
    {
      Serial.println(F("Error, max timeout reached"));
      break;
    }
  }

  /* Reads the response from the server */
  Serial.println(F("\nUbidots' Server response:\n"));
  while (clientUbi.available())
  {
    char c = clientUbi.read();
    Serial.print(c); // Uncomment this line to visualize the response on the Serial Monitor
  }

  /* Disconnecting the clientUbi */
  clientUbi.stop();
}


/********************************
 * Main Functions
 *******************************/

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Synchronize time useing SNTP. This is necessary to verify that
  // the TLS certificates offered by the server are currently valid.
  Serial.print("Setting time using SNTP");
  configTime(8 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2)
  {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));

  // Load root certificate in DER format into WiFiclientUbiSecure object
  bool cert1IsValid = clientUbi.setCACert_P(caCert2, caCertLen2);
  bool cert2IsValid = clientUbi.setCACert_P(caCert, caCertLen);
  if (!cert1IsValid && !cert2IsValid)
  {
    Serial.println("Failed to load root CA certificate!");
    while (true)
    {
      yield();
    }
  }
}

void loop()
{

  /*---- Connecting the clientUbi ----*/
  Serial.print("connecting to ");
  Serial.println(HTTPSERVER);
  clientUbi.connect(HTTPSERVER, HTTPPORT);

  if (!clientUbi.connect(HTTPSERVER, HTTPPORT))
  {
    Serial.println("connection failed");
    return;
  }

  /*---- Verify validity of server's certificate ----*/
  if (clientUbi.verifyCertChain(HTTPSERVER))
  {
    Serial.println("Server certificate verified");
    // Space to store values to send
    char payload[200];
    char str_val_1[30];

    /*---- Simulates the values of the sensors -----*/
    float sensor_value_1 = random(0, 1000)*1.0;

    /* 4 is mininum width, 2 is precision; float value is copied onto str_val*/
    dtostrf(sensor_value_1, 4, 2, str_val_1);

    // Important: Avoid to send a very long char as it is very memory space costly, send small char arrays
    sprintf(payload, "{\"");
    sprintf(payload, "%s%s\":{\"value\":%s}", payload, VARIABLE_LABEL_1, str_val_1);
    sprintf(payload, "%s}", payload);

    /* Calls the Ubidots Function POST */
    SendToUbidots(payload);
  }
  else
  {
    Serial.println("ERROR: certificate verification failed!");
    return;
  }

  delay(10000);
}
