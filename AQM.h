#ifndef AQM_h
#define AQM_h

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//                                  Third Party Libraries
#include <WebThing.h>
//                                  Local Includes
#include "AQIReader.h"
#include "PMS5003.h"
#include "AQMSettings.h"
//--------------- End:    Includes ---------------------------------------------


namespace AQM {
  static const String VersionString = "0.0.1";

  extern AQIReadings latestData;
  extern AQMSettings settings;
  extern AQIReader aqiReader;
  
  char* formattedTime(time_t theTime);
}

#endif  // AQM_h