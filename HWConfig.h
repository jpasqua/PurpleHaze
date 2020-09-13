#ifndef HWConfig_h
#define HWConfig_h

/*------------------------------------------------------------------------------
 *
 * Sensor Configuration
 *
 *----------------------------------------------------------------------------*/

const uint8_t SS_RX_PIN = D6;
const uint8_t SS_TX_PIN = D8;


/*------------------------------------------------------------------------------
 *
 * Indicator LED Configuration
 *
 *----------------------------------------------------------------------------*/

// If there are NeoPixels connected to use as status indicators, set the pin
// that drives the data line. If no NeoPixels are connected, set the pin to -1
const int8_t NEOPIXEL_PIN = D2;


#endif  // HWConfig_h
