#ifndef SecondarySerial_h
#define SecondarySerial_h

#include "HWConfig.h"

class SecondarySerial {
public:
  Stream* s;

  void begin() {
    #if defined(USE_SW_SERIAL)
      #include <SoftwareSerial.h>
      SoftwareSerial ss = new SoftwareSerial(SENSOR_RX_PIN, SENSOR_TX_PIN);
      s->begin(SensorBaudRate);
      s = ss;
    #else
      HardwareSerial* hs = new HardwareSerial(1);
      hs->begin(SensorBaudRate,SERIAL_8N1, SENSOR_RX_PIN, SENSOR_TX_PIN);
      s = hs;
    #endif
  }

private:
  const uint16_t SensorBaudRate = 9600;
};


#endif  // SecondarySerial_h