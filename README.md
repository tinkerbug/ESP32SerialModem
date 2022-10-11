# ESP32 Serial Modem
These programs are for ESP32 micro-processors to connect to a WiFi network and send RTCM correction data for an RTK GPS system between a base station and a rover. These are tested with a SkyTraq PX1125R GNSS receiver, which sends bursts of RTCM data every 1 second. Each burst contains multiple RTCM messages in quick succession with at least 800 micro seconds between bursts. The logic in the files supports the PX1125R receiver, but should work with othe receivers that send RTCM data as well.

# Base Station
The base station's GNSS receiver calculates RTCM correction data, which is a binary format, and sends it to another GNSS receiver on a rover. The base station sketch is in the the ESP32_SerialModemBurstTX directory. This code uses the SW Serial library for ESP32 to read data output from the GNSS receiver and act as a server that serves the correction data when a client connects.

# Rover
The rover receives GNSS correction data to compute an Real Time Kinetic (RTK) solution. To accompish this the ESP32_SerialModemBurstRX sketch reads data from the TCP server hosted on the base station. Once a burst of data is received, it then sends that data to the correction input port on the GNSS receiver using the software serial for ESP32 library.
