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

#define MAX_SERIAL_LENGTH 25000
#define MAX_MSG_LENGTH 1000

// IP address of server to connect to
IPAddress serverAddress(192, 168, 86, 45);
WiFiClient TCP_Client;

void setup()
{

    // USB serial 
    Serial.begin(115200);
    //while (!Serial){}; // TODO: Remove when running headless

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

    // Connect to TCP server
    while (!TCP_Client.connected()) 
    {
        connectToServer();
    }
}

char rtcm_data[MAX_SERIAL_LENGTH];

// Read serial from TCP server and send to GNSS
void loop ()
{
    // Monitor server connection and attempt to reconnect if connection is lost
    while (!TCP_Client.connected()) 
        connectToServer();

    // Read data from TCP server
    if (TCP_Client.connected())
        readAndSendTCPData();
}

void readAndSendTCPData()
{
    unsigned long i = 0;
    bool data_read = false;
    
    // If serial data is still available on TCP server
    while (TCP_Client.available())
    {
        // Read data from serial
        rtcm_data[i] = TCP_Client.read();
        i++;
        data_read = true;     

        // Ensure the char array is not overfilled
        if (i >= MAX_SERIAL_LENGTH)
        {
            Serial.println("!!!More TCP data than space in char array");
            break;
        }
    }
    if (data_read)
    {
        Serial.print(millis()/1000.0);Serial.print(" !!! Number of chars read: ");Serial.println(i);
        
        // Send message to GPS receiver
        swSerial.write(rtcm_data, i);
    }
}

void connectToServer()
{
    Serial.print("Connecting to TCP Server ...");
    TCP_Client.stop();
    
    // Reconnect to TCP server
    if (TCP_Client.connect(serverAddress, COM_PORT))
    {
        Serial.println("connected to TCP server");
    }
    else
    {
        Serial.print(".");
        delay(500);
    }
}
