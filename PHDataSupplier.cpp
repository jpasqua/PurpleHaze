/*
 * PHDataSupplier:
 *    Supplies app-specific data to the WebThing DataBroker
 *
 */

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//                                  Third Party Libraries
#include <ArduinoLog.h>
#include <Output.h>
//                                  Local Includes
#include "PurpleHazeApp.h"
#include "PHDataSupplier.h"
//--------------- End:    Includes ---------------------------------------------


namespace PHDataSupplier {

  // CUSTOM: If your app has a custom data source, publish that data to
  // plugins by implementing a data supplier that maps keys to values.
  // In this case we publish the data from the sensors
  void dataSupplier(const String& key, String& val) {
    const AQIReadings& aqiReadings = phApp->aqiMgr.getLastReadings();
    if (key == "aqi")           val.concat(phApp->aqiMgr.derivedAQI(aqiReadings.env.pm25));
    else if (key == "pm10std")  val.concat(aqiReadings.standard.pm10);
    else if (key == "pm25std")  val.concat(aqiReadings.standard.pm25);
    else if (key == "pm100std") val.concat(aqiReadings.standard.pm100);
    else if (key == "pm10env")  val.concat(aqiReadings.env.pm10);
    else if (key == "pm25env")  val.concat(aqiReadings.env.pm25);
    else if (key == "pm100env") val.concat(aqiReadings.env.pm100);
    else if (key == "p03")      val.concat(aqiReadings.particles_03um);
    else if (key == "p05")      val.concat(aqiReadings.particles_05um);
    else if (key == "p10")      val.concat(aqiReadings.particles_10um);
    else if (key == "p25")      val.concat(aqiReadings.particles_25um);
    else if (key == "p50")      val.concat(aqiReadings.particles_50um);
    else if (key == "p100")     val.concat(aqiReadings.particles_100um);
    else if (key == "tmst")     val = Output::formattedTime(Basics::wallClockFromMillis(aqiReadings.timestamp));
  }
}

