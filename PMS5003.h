/*
 * PMS5003.h
 *
 * This is a mash up of Adafruit's PM25 AQI driver
 * (https://github.com/adafruit/Adafruit_PMS5003) with 
 * the SwapBap PMS driver (https://github.com/SwapBap/PMS)
 *
 * It is jsut the UART part of the Adafruit driver along
 * with the control commands of the SwapBap driver
 *
 * ORIGINAL NOTICES from Adafruit library:
 * Written by Ladyada for Adafruit Industries.
 * BSD license, all text here must be included in any redistribution.
 *
 */

#ifndef PMS5003_h
#define PMS5003_h

#include <Arduino.h>

typedef struct {
  uint32_t timestamp;
  uint16_t pm10_standard;   // Standard PM1.0
  uint16_t pm25_standard;   // Standard PM2.5
  uint16_t pm100_standard;  // Standard PM10.0
  uint16_t pm10_env;        // Environmental PM1.0
  uint16_t pm25_env;        // Environmental PM2.5
  uint16_t pm100_env;       // Environmental PM10.0
  uint16_t particles_03um;  // 0.3um Particle Count
  uint16_t particles_05um;  // 0.5um Particle Count
  uint16_t particles_10um;  // 1.0um Particle Count
  uint16_t particles_25um;  // 2.5um Particle Count
  uint16_t particles_50um;  // 5.0um Particle Count
  uint16_t particles_100um; // 10.0um Particle Count
} AQIReadings;


class PMS5003 {
public:
  PMS5003();

  // Prepares the hardware via the supplied serial interface (HardwareSerial/SoftwareSerial)
  bool begin(Stream* theStream);

  // Put the device to sleep. This reduces power usage and extends the life of the sensor
  void sleep();

  // Wakeup a sleeping device. Wait 30 seconds after waking for stable readings

  void wakeUp();

  // Put the device into Active mode which is the default mode after power up.
  // In this mode sensor will send serial data to the host automatically.
  void activeMode();

  // Put the device into Passive mode. In this mode sensor will send serial data
  // to the host upon request.
  void passiveMode();

  // Request a read while in Passive mode
  void requestRead();

  // Read a packet of data if one is available. Return false if no data was available or
  // if the data was bad in some way.
  bool read(AQIReadings* data);


private:
  enum DeviceMode { MODE_ACTIVE, MODE_PASSIVE };

  Stream*     _serial_dev = NULL;
  uint8_t     _readbuffer[32];
  DeviceMode  _mode = MODE_ACTIVE;
};

#endif  // PMS5003_h