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
  chartColors.avg = String(doc["wthr"]["chartColors"]["avg"]|"#ff00ff");
}

void WeatherSettings::toJSON(JsonDocument &doc) {
  doc["wthr"]["tempCorrection"] = tempCorrection;
  doc["wthr"]["humiCorrection"] = humiCorrection;
  doc["wthr"]["chartColors"]["temp"] = chartColors.temp;
  doc["wthr"]["chartColors"]["avg"] = chartColors.avg;
}

void WeatherSettings::logSettings() {
  Log.verbose(F("Weather Settings"));
  Log.verbose(F("  tempCorrection = %F"), tempCorrection);
  Log.verbose(F("  humiCorrection = %F"), humiCorrection);
  Log.verbose(F("  chartColors.temp = %s"), chartColors.temp.c_str());
  Log.verbose(F("  chartColors.avg = %s"), chartColors.avg.c_str());
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
  use24Hour = doc[F("use24Hour")];
  iBright = doc[F("iBright")];
  chartColors.pm10 = String(doc["chartColors"]["pm10"]|"#e32400");
  chartColors.pm25 = String(doc["chartColors"]["pm25"]|"#4e7a27");
  chartColors.pm100 = String(doc["chartColors"]["pm100"]|"#0042aa");
  chartColors.aqi = String(doc["chartColors"]["aqi"]|"#f00f88");

  weatherSettings.fromJSON(doc);
  logSettings();
}

void PHSettings::toJSON(JsonDocument &doc) {
  doc["description"] = description;
  doc["blynkAPIKey"] = blynkAPIKey;
  doc[F("showDevMenu")] = showDevMenu;
  doc[F("useMetric")] = useMetric;
  doc[F("use24Hour")] = use24Hour;
  doc[F("iBright")] = iBright;
  doc["chartColors"]["pm10"] = chartColors.pm10;
  doc["chartColors"]["pm25"] = chartColors.pm25;
  doc["chartColors"]["pm100"] = chartColors.pm100;
  doc["chartColors"]["aqi"] = chartColors.aqi;
  weatherSettings.toJSON(doc);
}

void PHSettings::logSettings() {
  Log.verbose(F("PurpleHaze Settings"));
  Log.verbose(F("  description = %s"), description.c_str());
  Log.verbose(F("  blynkAPIKey = %s"), blynkAPIKey.c_str());
  Log.verbose(F("  show dev menu: %T"), showDevMenu);
  Log.verbose(F("  Use metric: %T"), useMetric);
  Log.verbose(F("  Use 24 Hour Time: %T"), use24Hour);
  Log.verbose(F("  indicator brightness: %d"), iBright);
  Log.verbose(F("  chartColors.pm10 = %s"), chartColors.pm10.c_str());
  Log.verbose(F("  chartColors.pm25 = %s"), chartColors.pm25.c_str());
  Log.verbose(F("  chartColors.pm100 = %s"), chartColors.pm100.c_str());
  Log.verbose(F("  chartColors.aqi = %s"), chartColors.aqi.c_str());
  weatherSettings.logSettings();
}

