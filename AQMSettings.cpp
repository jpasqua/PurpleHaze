/*
 * AQMSettings
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
#include "AQMSettings.h"
//--------------- End:    Includes ---------------------------------------------

const uint32_t  AQMSettings::CurrentVersion = 0x0001;

AQMSettings::AQMSettings() {
  version = AQMSettings::CurrentVersion;
  maxFileSize = 1024;
}

void AQMSettings::fromJSON(JsonDocument &doc) {
  description = doc["description"].as<String>();
  blynkAPIKey = doc["blynkAPIKey"].as<String>();
  showDevMenu = doc[F("showDevMenu")];
  chartColors.pm10 = doc["pm10Color"].as<String>();
  chartColors.pm25 = doc["pm25Color"].as<String>();
  chartColors.pm100 = doc["pm100Color"].as<String>();
  logSettings();
}

void AQMSettings::toJSON(JsonDocument &doc) {
  doc["description"] = description;
  doc["blynkAPIKey"] = blynkAPIKey;
  doc[F("showDevMenu")] = showDevMenu;
  doc["pm10Color"] = chartColors.pm10;
  doc["pm25Color"] = chartColors.pm25;
  doc["pm100Color"] = chartColors.pm100;

}

void AQMSettings::logSettings() {
  Log.verbose(F("AQM Settings"));
  Log.verbose(F("  description = %s"), description.c_str());
  Log.verbose(F("  blynkAPIKey = %s"), blynkAPIKey.c_str());
  Log.verbose(F("  show dev menu: %T"), showDevMenu);
  Log.verbose(F("  chartColors.pm10 = %s"), chartColors.pm10.c_str());
  Log.verbose(F("  chartColors.pm25 = %s"), chartColors.pm25.c_str());
  Log.verbose(F("  chartColors.pm100 = %s"), chartColors.pm100.c_str());
}

