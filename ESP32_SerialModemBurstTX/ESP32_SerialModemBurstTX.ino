// ESP32 WiFi library
#include <WiFi.h>

// ESP32 software serial impementation
#include <SoftwareSerial.h>

// Configuration parameters
#include "inputs.h"

// Software serial pins for communicating with GNSS receiver
#define TX 26
#define RX 25
SoftwareSerial swSerial;

// Communications port to other ESP32
#define COM_PORT 4081

// Server for communicating RTCM data
WiFiServer TCP_Server(COM_PORT);

#define MAX_SERIAL_LENGTH 2500

// Remote client handle
WiFiClient RemoteClient;

void setup() 
{
    Serial.begin(115200);
    //while(!Serial){}; // TODO: Remove when running headless
    delay(500);

    // Software serial for RTCM data
    swSerial.begin(57600, SWSERIAL_8N1, RX, TX, false);
    delay(500);
    
    // Connect to WiFi
    Serial.print("Connecting to ");
    Serial.println(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    // Start TCP server
    TCP_Server.begin(); 
}

void loop()
{

    // Check for client connections
    checkForConnections();

    // Read RTCM data and send to client
    if (swSerial.available ())
    {
        readSerialBufferAndSend();
    }
}

unsigned int data_counter = 0;
char rtcm_data[MAX_SERIAL_LENGTH];
bool data_available = false;

// Read serial buffer and pack data into an array for sending
void readSerialBufferAndSend()
{

    // Position through current packet
    unsigned long lastReadTime = 0;

    // Read a burst of GNSS data, the PX11XX receivers send RTCM
    // data in bursts of messages with hundreds of milliseconds
    // between each burst
    lastReadTime = millis();
    while (swSerial.available() || (millis() - lastReadTime) < 50)
    {
        if (swSerial.available())
        {
            // Read data from GNSS via serial
            rtcm_data[data_counter] = swSerial.read();
            data_counter++;
            data_available = true;
            lastReadTime = millis();
        }
    }
    
    // Send data to TCP server
    if (data_available)
    {
        RemoteClient.write((uint8_t*)rtcm_data,data_counter);
        Serial.print(millis());Serial.print(" Sent RTCM data length ");Serial.println(data_counter);
        data_available = false;
        data_counter = 0;
    }
}

// Check for client connection requests
void checkForConnections()
{
  if (TCP_Server.hasClient())
  {
    // Reject connections after the first. 
    if (RemoteClient.connected())
    {
        Serial.println("Connection rejected");
        TCP_Server.available().stop();
    }
    // Accept first connection
    else
    {
        Serial.println("Connection accepted");
        RemoteClient = TCP_Server.available();
    }
  }
}
