/*
 * PHSettings
 *    Handle reading and writing settings information to the file system
 *    in JSON format.
 *
 * NOTES:
 *
 */

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <Arduino.h>
#include <FS.h>
//                                  Third Party Libraries
#include <ArduinoLog.h>
#include <ArduinoJson.h>
//                                  Local Includes
#include "PHSettings.h"
//--------------- End:    Includes ---------------------------------------------

const uint32_t  PHSettings::CurrentVersion = 0x0001;

PHSettings::PHSettings() {
  version = PHSettings::CurrentVersion;
  maxFileSize = 1024;
}

void PHSettings::fromJSON(JsonDocument &doc) {
  description = doc["description"].as<String>();
  blynkAPIKey = String(doc["blynkAPIKey"]|"");
  showDevMenu = doc[F("showDevMenu")];
  iBright = doc[F("iBright")];
  chartColors.pm10 = String(doc["chartColors"]["pm10"]|"#e32400");
  chartColors.pm25 = String(doc["chartColors"]["pm25"]|"#4e7a27");
  chartColors.pm100 = String(doc["chartColors"]["pm100"]|"#0042aa");
  chartColors.aqi = String(doc["chartColors"]["aqi"]|"#f00f88");
  logSettings();
}

void PHSettings::toJSON(JsonDocument &doc) {
  doc["description"] = description;
  doc["blynkAPIKey"] = blynkAPIKey;
  doc[F("showDevMenu")] = showDevMenu;
  doc[F("iBright")] = iBright;
  doc["chartColors"]["pm10"] = chartColors.pm10;
  doc["chartColors"]["pm25"] = chartColors.pm25;
  doc["chartColors"]["pm100"] = chartColors.pm100;
  doc["chartColors"]["aqi"] = chartColors.aqi;

}

void PHSettings::logSettings() {
  Log.verbose(F("PurpleHaze Settings"));
  Log.verbose(F("  description = %s"), description.c_str());
  Log.verbose(F("  blynkAPIKey = %s"), blynkAPIKey.c_str());
  Log.verbose(F("  show dev menu: %T"), showDevMenu);
  Log.verbose(F("  indicator brightness: %d"), iBright);
  Log.verbose(F("  chartColors.pm10 = %s"), chartColors.pm10.c_str());
  Log.verbose(F("  chartColors.pm25 = %s"), chartColors.pm25.c_str());
  Log.verbose(F("  chartColors.pm100 = %s"), chartColors.pm100.c_str());
  Log.verbose(F("  chartColors.aqi = %s"), chartColors.aqi.c_str());
}

