#ifndef HWConfig_h
#define HWConfig_h

#include <Arduino.h>

// ----- BEGIN: Constants used in the rest of the file. Do not change these
// An enumeration of allowable values for HWConfig
#define Config_Custom        1
#define Config_D1Mini        2
#define Config_ESP32Mini     3
// ----- END: Constants used in the rest of the file

// Pick a configuration. Use Config_Custom to define your own
#define HWConfig Config_D1Mini


#if (HWConfig == Config_D1Mini)
  /*------------------------------------------------------------------------------
   *
   * Config Info for D1 Mini
   *
   *----------------------------------------------------------------------------*/

  // ----- Sensors
  #define USE_SW_SERIAL 1
  const uint8_t SENSOR_RX_PIN = D6;
  const uint8_t SENSOR_TX_PIN = D8;

  const int8_t NEOPIXEL_PIN = D2;


#elif (HWConfig == Config_ESP32Mini)
  /*------------------------------------------------------------------------------
   *
   * Config Info for ESP32D1Mini
   *
   *----------------------------------------------------------------------------*/

  // ----- Sensors
  const uint8_t SENSOR_RX_PIN = 16;
  const uint8_t SENSOR_TX_PIN = 17;

  const int8_t NEOPIXEL_PIN = 21;


#elif (HWConfig ==  Config_Custom)
  /*------------------------------------------------------------------------------
   *
   * Custom Config - put your settings here
   *
   *----------------------------------------------------------------------------*/

  // ----- Sensors
  #define USE_SW_SERIAL 1
  const uint8_t SENSOR_RX_PIN = D6;
  const uint8_t SENSOR_TX_PIN = D8;

  // If there are NeoPixels connected to use as status indicators, set the pin
  // that drives the data line. If no NeoPixels are connected, set the pin to -1
  const int8_t NEOPIXEL_PIN = D2;

#else
    #error "Please set HWConfig"
#endif


#endif  // HWConfig_h
