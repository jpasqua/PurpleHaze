/*
 * Settings.h
 *    Defines the values that can be set through the web UI and sets their initial values
 *
 */

#ifndef AQMSettings_h
#define AQMSettings_h

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <Arduino.h>
//                                  Third Party Libraries
#include <BaseSettings.h>
//                                  Local Includes
//--------------- End:    Includes ---------------------------------------------


class AQMSettings: public BaseSettings {
public:
  // ----- Constructors and methods
  AQMSettings();
  void fromJSON(JsonDocument &doc) override;
  void toJSON(JsonDocument &doc);
  void logSettings();

  // ----- Settings
  String  description = "Air Quality Sensor"; // User's description of their AQ Sensor
  String  blynkAPIKey = "";                   // Your API Key from Blynk
  struct {
    String pm10 = "#e32400";
    String pm25 = "#4e7a27";
    String pm100 = "#0042aa";
  } chartColors;
  bool showDevMenu = false;

private:
  // ----- Constants -----
  static const uint32_t CurrentVersion;
};
#endif // AQMSettings_h