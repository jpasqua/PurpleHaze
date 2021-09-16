#ifndef Output_h
#define Output_h

#include <WTBasics.h>
#include <Time.h>

namespace Output {
  // ----- Units
  extern void setUnits(bool* metric);
  extern float temp(float temp);
  extern float tempSpread(float spread);
  extern float baro(float baro);
  extern const char* tempUnits();
  extern const char* baroUnits();

  // ----- Time
  extern String dateTime(time_t theTime);
};

#endif  // Output_h