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


void WeatherSettings::fromJSON(const JsonDocument &doc) {
  tempCorrection = doc["wthr"]["tempCorrection"];
  humiCorrection = doc["wthr"]["humiCorrection"];
  chartColors.temp = String(doc["wthr"]["chartColors"]["temp"]|"#4e7a27");
  chartColors.humi = String(doc["wthr"]["chartColors"]["humi"]|"#ff00ff");
  graphRange = doc["wthr"]["graphRange"];
  graphRange = (graphRange > 2) ? 2 : graphRange;
}

void WeatherSettings::toJSON(JsonDocument &doc) {
  doc["wthr"]["tempCorrection"] = tempCorrection;
  doc["wthr"]["humiCorrection"] = humiCorrection;
  doc["wthr"]["chartColors"]["temp"] = chartColors.temp;
  doc["wthr"]["chartColors"]["humi"] = chartColors.humi;
  doc["wthr"]["graphRange"] = graphRange;
}

void WeatherSettings::logSettings() {
  Log.verbose(F("Weather Settings"));
  Log.verbose(F("  tempCorrection = %F"), tempCorrection);
  Log.verbose(F("  humiCorrection = %F"), humiCorrection);
  Log.verbose(F("  chartColors.temp = %s"), chartColors.temp.c_str());
  Log.verbose(F("  chartColors.humi = %s"), chartColors.humi.c_str());
  Log.verbose(F("  Graph Range = %d"), graphRange);
}

void AQISettings::fromJSON(const JsonDocument &doc) {
  chartColors.aqi = String(doc["aqi"]["chartColors"]["aqi"]|"#f00f88");
  graphRange = doc["aqi"]["graphRange"];
  graphRange = (graphRange > 2) ? 2 : graphRange;
}

void AQISettings::toJSON(JsonDocument &doc) {
  doc["aqi"]["chartColors"]["aqi"] = chartColors.aqi;
  doc["aqi"]["graphRange"] = graphRange;
}

void AQISettings::logSettings() {
  Log.verbose(F("AQI Settings"));
  Log.verbose(F("  chartColors.aqi = %s"), chartColors.aqi.c_str());
  Log.verbose(F("  Graph Range = %d"), graphRange);
}



PHSettings::PHSettings() {
  version = PHSettings::CurrentVersion;
  maxFileSize = 2048;
}

void PHSettings::fromJSON(const JsonDocument &doc) {
  description = (doc["description"]|"Environment Sensor");
  // if (description.isEmpty() || description == "null") description = String("Environment Sensor");
  aio.username = String(doc["aioUsername"]|"");
  aio.key = String(doc["aioKey"]|"");
  aio.groupName = String(doc["aioGroup"]|"");
  iBright = doc[F("iBright")] | 80;

  aqiSettings.fromJSON(doc);
  weatherSettings.fromJSON(doc);
  WTAppSettings::fromJSON(doc);
}

void PHSettings::toJSON(JsonDocument &doc) {
  doc["description"] = description;
  doc["aioUsername"] = aio.username;
  doc["aioKey"] = aio.key;
  doc["aioGroup"] = aio.groupName;
  doc[F("iBright")] = iBright;

  aqiSettings.toJSON(doc);
  weatherSettings.toJSON(doc);
  WTAppSettings::toJSON(doc);
}

void PHSettings::logSettings() {
  Log.verbose(F("PurpleHaze Settings"));
  Log.verbose(F("  description = %s"), description.c_str());
  Log.verbose(F("  aio.username = %s"), aio.username.c_str());
  Log.verbose(F("  aio.key = %s"), aio.key.c_str());
  Log.verbose(F("  aio.groupName = %s"), aio.groupName.c_str());
  Log.verbose(F("  indicator brightness: %d"), iBright);
  aqiSettings.logSettings();
  weatherSettings.logSettings();
  WTAppSettings::logSettings();
}

