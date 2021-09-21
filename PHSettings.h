/*
 * PHSettings.h
 *    Defines the values that can be set through the web UI and sets their initial values
 *
 */

#ifndef PHSettings_h
#define PHSettings_h

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <Arduino.h>
//                                  Third Party Libraries
#include <BaseSettings.h>
//                                  Local Includes
//--------------- End:    Includes ---------------------------------------------


class WeatherSettings {
public:
  float   tempCorrection = 0.0; // Correction to the temp reading - always in Celsius
  float   humiCorrection = 0.0; // Correction to the humidty sensor reading
  struct {
    String temp = "#4e7a27";
    String avg = "#ff00ff";
  } chartColors;
  void fromJSON(const JsonDocument &doc);
  void toJSON(JsonDocument &doc);
  void logSettings();
};

class PHSettings: public BaseSettings {
public:
  // ----- Constructors and methods
  PHSettings();
  void fromJSON(const JsonDocument &doc) override;
  void toJSON(JsonDocument &doc) override;
  void logSettings() override;

  // ----- Settings
  String  description = "Air Quality Sensor"; // User's description of their AQ Sensor
  String  blynkAPIKey = "";                   // Your API Key from Blynk
  struct {
    String pm10 = "#e32400";
    String pm25 = "#4e7a27";
    String pm100 = "#0042aa";
    String aqi = "#f00f88";
  } chartColors;
  uint8_t iBright = 50;                       // Brightness of indicators (0-100%)
  bool showDevMenu = false;
  bool useMetric = false;
  bool use24Hour = false;
  
  WeatherSettings weatherSettings;

private:
  // ----- Constants -----
  static constexpr uint32_t CurrentVersion = 0x0002;
};
#endif // PHSettings_h