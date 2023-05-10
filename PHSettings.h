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
#include <WTAppSettings.h>
//                                  Local Includes
//--------------- End:    Includes ---------------------------------------------


class WeatherSettings {
public:
  float   tempCorrection = 0.0; // Correction to the temp reading - always in Celsius
  float   humiCorrection = 0.0; // Correction to the humidty sensor reading
  struct {
    String temp = "#ff00ff";
    String humi = "#4e7a27";
  } chartColors;
  uint8_t graphRange = 0;
  void fromJSON(const JsonDocument &doc);
  void toJSON(JsonDocument &doc);
  void logSettings();
};

class AQISettings {
public:
  struct {
    String aqi = "#f00f88";
  } chartColors;
  uint8_t graphRange = 0;
  void fromJSON(const JsonDocument &doc);
  void toJSON(JsonDocument &doc);
  void logSettings();
};

class PHSettings: public WTAppSettings {
public:
  // ----- Constructors and methods
  PHSettings();
  void fromJSON(const JsonDocument &doc) override;
  void toJSON(JsonDocument &doc) override;
  void logSettings() override;

  // ----- Settings
  String  description = "Air Quality Sensor"; // User's description of their AQ Sensor
  struct {
    String username;
    String key;
    String groupName;
  } aio;
  uint8_t iBright = 50;                       // Brightness of indicators (0-100%)
  
  AQISettings aqiSettings;
  WeatherSettings weatherSettings;

private:
  // ----- Constants -----
  static constexpr uint32_t CurrentVersion = 0x0002;
};
#endif // PHSettings_h