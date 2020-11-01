#ifndef PurpleHaze_h
#define PurpleHaze_h

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//                                  Third Party Libraries
#include <WebThing.h>
//                                  Local Includes
#include "Indicators.h"
#include "AQIReader.h"
#include "PMS5003.h"
#include "PHSettings.h"
//--------------- End:    Includes ---------------------------------------------


namespace PH {
  static const String VersionString = "0.0.3";

  extern AQIReadings latestData;
  extern PHSettings settings;
  extern AQIReader aqiReader;
  extern Indicator* busyIndicator;

  /**
   * Return a pointer to a static buffer with the supplied time as a formatted
   * string of the form YYYY/MM/DD HH:MM:SS
   */
  char* formattedTime(time_t theTime);

  /**
   * Set the brightness of all indicators to the supplied
   * value from 0-100%
   */
  void setIndicatorBrightness(uint8_t b);

  /**
   * Appends a JSON description corresponding to the supplied AQI value
   * to the supplied String object.
   * Example result:
   * {
   *   "timestamp": 1604162511,
   *   "aqi": 51,
   *   "shortDesc": "Moderate",
   *   "longDesc": "Air quality is acceptable. <remainder omitted for brevity>",
   *   "color": 16776960
   * }
   */
  void aqiAsJSON(uint16_t quality, String& result);
}

#endif  // PurpleHaze_h