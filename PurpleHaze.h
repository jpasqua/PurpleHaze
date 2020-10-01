#ifndef PurpleHaze_h
#define PurpleHaze_h

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//                                  Third Party Libraries
#include <WebThing.h>
//                                  Local Includes
#include "AQIReader.h"
#include "PMS5003.h"
#include "PHSettings.h"
//--------------- End:    Includes ---------------------------------------------


namespace PH {
  static const String VersionString = "0.0.1";

  extern AQIReadings latestData;
  extern PHSettings settings;
  extern AQIReader aqiReader;
  
  char* formattedTime(time_t theTime);
  void setIndicatorBrightness(uint8_t b); // 0-100%
}

#endif  // PurpleHaze_h