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


void BMESettings::fromJSON(const JsonDocument &doc) {
  tempCorrection = doc["bme"]["tempCorrection"];
  humiCorrection = doc["bme"]["humiCorrection"];
  chartColors.temp = String(doc["chartColors"]["temp"]|"#4e7a27");
  chartColors.avg = String(doc["chartColors"]["avg"]|"#ff00ff");
}

void BMESettings::toJSON(JsonDocument &doc) {
  doc["bme"]["tempCorrection"] = tempCorrection;
  doc["bme"]["humiCorrection"] = humiCorrection;
  doc["chartColors"]["temp"] = chartColors.temp;
  doc["chartColors"]["avg"] = chartColors.avg;
}

void BMESettings::logSettings() {
  Log.verbose("BME Settings");
  Log.verbose("  tempCorrection = %F", tempCorrection);
  Log.verbose("  humiCorrection = %F", humiCorrection);
}



PHSettings::PHSettings() {
  version = PHSettings::CurrentVersion;
  maxFileSize = 1024;
}

void PHSettings::fromJSON(const JsonDocument &doc) {
  description = doc["description"].as<String>();
  blynkAPIKey = String(doc["blynkAPIKey"]|"");
  showDevMenu = doc[F("showDevMenu")];
  useMetric = doc[F("useMetric")];
  iBright = doc[F("iBright")];
  chartColors.pm10 = String(doc["chartColors"]["pm10"]|"#e32400");
  chartColors.pm25 = String(doc["chartColors"]["pm25"]|"#4e7a27");
  chartColors.pm100 = String(doc["chartColors"]["pm100"]|"#0042aa");
  chartColors.aqi = String(doc["chartColors"]["aqi"]|"#f00f88");

  bmeSettings.fromJSON(doc);
  logSettings();
}

void PHSettings::toJSON(JsonDocument &doc) {
  doc["description"] = description;
  doc["blynkAPIKey"] = blynkAPIKey;
  doc[F("showDevMenu")] = showDevMenu;
  doc[F("useMetric")] = useMetric;
  doc[F("iBright")] = iBright;
  doc["chartColors"]["pm10"] = chartColors.pm10;
  doc["chartColors"]["pm25"] = chartColors.pm25;
  doc["chartColors"]["pm100"] = chartColors.pm100;
  doc["chartColors"]["aqi"] = chartColors.aqi;
  bmeSettings.toJSON(doc);
}

void PHSettings::logSettings() {
  Log.verbose(F("PurpleHaze Settings"));
  Log.verbose(F("  description = %s"), description.c_str());
  Log.verbose(F("  blynkAPIKey = %s"), blynkAPIKey.c_str());
  Log.verbose(F("  show dev menu: %T"), useMetric);
  Log.verbose(F("  Use metric: %T"), showDevMenu);
  Log.verbose(F("  indicator brightness: %d"), iBright);
  Log.verbose(F("  chartColors.pm10 = %s"), chartColors.pm10.c_str());
  Log.verbose(F("  chartColors.pm25 = %s"), chartColors.pm25.c_str());
  Log.verbose(F("  chartColors.pm100 = %s"), chartColors.pm100.c_str());
  Log.verbose(F("  chartColors.aqi = %s"), chartColors.aqi.c_str());
  bmeSettings.logSettings();
}

