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
  logSettings();
}

void AQMSettings::toJSON(JsonDocument &doc) {
  doc["description"] = description;
  doc["blynkAPIKey"] = blynkAPIKey;
}

void AQMSettings::logSettings() {
  Log.verbose("AQM Settings");
  Log.verbose("  description = %s", description.c_str());
  Log.verbose("  blynkAPIKey = %s", blynkAPIKey.c_str());
}

