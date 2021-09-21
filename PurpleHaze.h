#ifndef PurpleHaze_h
#define PurpleHaze_h

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//                                  Third Party Libraries
#include <Indicators.h>
//                                  WebThing Includes
#include <WebThing.h>
#include <sensors/WeatherMgr.h>
#include <sensors/WeatherReadings.h>
//                                  Local Includes
#include "src/hardware/HWConfig.h"
#include "PHSettings.h"
#include "src/clients/AQIMgr.h"
//--------------- End:    Includes ---------------------------------------------


namespace PH {
  static const String VersionString = "0.0.4";

  extern AQIMgr aqiMgr;

  #if defined(HAS_WEATHER_SENSOR)
    extern WeatherMgr weatherMgr;
  #endif

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