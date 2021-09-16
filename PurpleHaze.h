#ifndef PurpleHaze_h
#define PurpleHaze_h

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//                                  Third Party Libraries
#include <WebThing.h>
//                                  Local Includes
#include "PHSettings.h"
#include "src/clients/AQIMgr.h"
#include "src/clients/WeatherMgr.h"
#include "src/clients/WeatherReadings.h"
#include "src/utils/Indicators.h"
//--------------- End:    Includes ---------------------------------------------


namespace PH {
  static const String VersionString = "0.0.4";

  extern AQIMgr aqiMgr;

  #if defined(HAS_THP_SENSOR)
    extern WeatherMgr weatherMgr;
  #endif  // HAS_THP_SENSOR

  extern PHSettings settings;
  extern Indicator* busyIndicator;

  /**
   * Set the brightness of all indicators to the supplied
   * value from 0-100%
   */
  void setIndicatorBrightness(uint8_t b);

  void updateWeatherSensorAttributes();

}

#endif  // PurpleHaze_h