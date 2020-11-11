/*
 * PurpleHaze - Monitor an air quality sensor and upload data to the Blynk Service
 *                    
 * NOTES:
 *
 */


//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <SoftwareSerial.h>
//                                  Third Party Libraries
#include <ArduinoLog.h>
#include <TimeLib.h>
#include <WebUI.h>
//                                  Local Includes
#include "PurpleHaze.h"
#include "HWConfig.h"
#include "AQIReader.h"
#include "PHWebUI.h"
#include "PHBlynk.h"
//--------------- End:    Includes ---------------------------------------------


namespace PH {
  /*------------------------------------------------------------------------------
   *
   * Global State
   *
   *----------------------------------------------------------------------------*/
  AQIReader aqiReader;
  Indicator* sensorIndicator;
  Indicator* qualityIndicator;
  Indicator* busyIndicator;
  NeoPixelIndicators* indicators;
  SoftwareSerial* streamToSensor;
  PHSettings settings;
  AQIReadings latestData;

  namespace Internal {
    static const struct {
      uint16_t max;
      uint32_t color;
      const char* shortDesc;
      const char* longDesc;
    } QualityBrackets[] = {
      { 50, 0x00ff00, "Good",
          "Air quality is satisfactory, and air pollution poses little or no risk."},
      {100, 0xffff00, "Moderate",
          "Air quality is acceptable. However, there may be a risk for some people, "
          "particularly those who are unusually sensitive to air pollution."},
      {150, 0xffa500, "Unhealthy for Sensitive Groups",
          "Members of sensitive groups may experience health effects. The general "
          "public is less likely to be affected."},
      {200, 0xff0000, "Unhealthy",
          "Some members of the general public may experience health effects; "
          "members of sensitive groups may experience more serious health effects."},
      {300, 0x9400D3, "Very Unhealthy",
          "Health alert: The risk of health effects is increased for everyone."},
      {999, 0x9400D3, "Hazardous",
          "Health warning of emergency conditions: "
          "everyone is more likely to be affected."},
    };
    static const uint16_t nBrackets = sizeof(QualityBrackets)/sizeof(QualityBrackets[0]);

    static const String   SettingsFileName = "/settings.json";

    void ensureWebThingSettings() {
      // PH has a fixed hardware configuration so some of the settings need to have
      // specific values. Rather than make the user configure them, set them to the
      // right values here.
      WebThing::displayPowerOptions(false);
      WebThing::settings.sleepOverridePin = -1;
      WebThing::settings.hasVoltageSensing = false;
      if (WebThing::settings.hostname.isEmpty()) WebThing::settings.hostname = ("ph_" + String(ESP.getChipId(), HEX));
    }

    void flushBeforeSleep() { PHBlynk::disconnect(); }

    void prepIO() {
      if (NEOPIXEL_PIN == -1) {
        indicators = NULL;
        qualityIndicator = new Indicator();
        sensorIndicator = new Indicator();
        busyIndicator = new Indicator();
      } else {
        indicators = new NeoPixelIndicators(NEOPIXEL_PIN, 3);
        indicators->begin();
        indicators->setBrightness(settings.iBright);
        NeoPixelIndicator* npi;
        npi = new NeoPixelIndicator(); npi->begin(indicators, 0); qualityIndicator = npi;
        npi = new NeoPixelIndicator(); npi->begin(indicators, 1); sensorIndicator = npi;
        npi = new NeoPixelIndicator(); npi->begin(indicators, 2); busyIndicator = npi;

      }
      qualityIndicator->setColor(0x969696);  // No data available yet
    }

    void prepSensor() {
      streamToSensor = new SoftwareSerial(SS_RX_PIN, SS_TX_PIN);
      streamToSensor->begin(9600);

      if (!aqiReader.init(streamToSensor, sensorIndicator)) {
        Log.error("Unable to connect to Air Quality Sensor!");
        qualityIndicator->setColor(255, 0, 0);
        sensorIndicator->setColor(255, 0, 0);
        busyIndicator->setColor(255, 0, 0);
      }
    }

    void prepWebUI() {
      WebThing::setup();
      WebThing::notifyBeforeDeepSleep(flushBeforeSleep);
      PHWebUI::init();
    }

    void baseConfigChange() { WebUI::setTitle(settings.description+" ("+WebThing::settings.hostname+")"); }

    void configModeCallback(String &ssid, String &ip) {
      (void)ssid; (void)ip;
      busyIndicator->setColor(0, 0, 255);
    }

    void processReadings() {
      static uint32_t lastTimestamp = 0;

      latestData = aqiReader.getLastReadings();
      if (latestData.timestamp == lastTimestamp) return;

      busyIndicator->setColor(0, 255, 0);
      uint16_t quality = aqiReader.derivedAQI(latestData.env.pm25);
      for (int i = 0; i < nBrackets; i++) {
        if (quality <= QualityBrackets[i].max) {
          qualityIndicator->setColor(QualityBrackets[i].color);
          break;
        }
      }
      
      PHBlynk::update();
      lastTimestamp = latestData.timestamp;
      busyIndicator->off();
    }
  } // ----- END: PH::Internal namespace


  /*------------------------------------------------------------------------------
   *
   * Exported Functions
   *
   *----------------------------------------------------------------------------*/

  void aqiAsJSON(uint16_t quality, String& result) {
    const char* shortDesc = "Unknown";
    const char* longDesc = "Unknown";
    uint32_t color = 0x000000;

    for (int i = 0; i < Internal::nBrackets; i++) {
      if (quality <= Internal::QualityBrackets[i].max) {
        shortDesc = Internal::QualityBrackets[i].shortDesc;
        longDesc = Internal::QualityBrackets[i].longDesc;
        color = Internal::QualityBrackets[i].color;
        break;
      }
    }

    int32_t tzOffset = WebThing::getGMTOffset();
    result += "{\n";
    result += "  \"timestamp\": "; result += (latestData.timestamp - tzOffset); result += ",\n";
    result += "  \"aqi\": "; result += quality; result += ",\n";
    result += "  \"shortDesc\": "; result += "\""; result += shortDesc; result += "\",\n";
    result += "  \"longDesc\": "; result += "\""; result += longDesc; result += "\",\n";
    result += "  \"color\": "; result += color; result += "\n";
    result += "}";
  }

  void setIndicatorBrightness(uint8_t b) {
    if (indicators) indicators->setBrightness((b*255L)/100);
  }

  char *formattedTime(time_t theTime) {
    static char dateTime[19];
    sprintf(dateTime, "%d/%02d/%02d %02d:%02d:%02d", 
      year(theTime), month(theTime), day(theTime), hour(theTime), minute(theTime), second(theTime));
    return dateTime;
  }

} // ----- END: PH namespace


/*------------------------------------------------------------------------------
 *
 * GLOBAL SCOPE: setup() and loop()
 *
 * The setup() and loop() functions need to be in the global scope, but are logically
 * part of the PH namespace. Use a "using" directive for JUST THESE 2 functions
 * to treat them as if they were in that namespace.
 *
 *----------------------------------------------------------------------------*/

using namespace PH;

void setup() {
  WebThing::preSetup();             // Setup the WebThing - Must be first
  WebThing::setDisplayedVersion(VersionString);

  settings.init(Internal::SettingsFileName);  
  settings.read();                  // Read the settings
  Internal::prepIO();               // Prepare any I/O pins used locally

  Internal::ensureWebThingSettings();         // Override any pertinent settings in WebThing
  WebThing::notifyOnConfigMode(Internal::configModeCallback);

  WebThing::notifyConfigChange(Internal::baseConfigChange);
                                    // If the WebThing config changes, let us know

  Internal::prepWebUI();            // Setup the WebUI, network, etc.
  busyIndicator->off();
  Internal::prepSensor();           // Prep the sensor after we're connected to the web
                                    // so we have time info for timestamps, etc.

  PHBlynk::init();                 // Setup the Blynk Client

  Internal::processReadings();      // Get our first set of readings!

  WebThing::postSetup();            // Finalize setup of the WebThing - Must be last
}

void loop() {
  // static uint32_t l = millis();
  // static uint32_t qi = 0;
  // if (millis()-l > 500) {
  //   l = millis();
  //   qualityIndicator->setColor(Internal::QualityColors[qi]);
  //   qi = (qi+1)%8;
  // }
  WebThing::loop();
  PHBlynk::run();
  aqiReader.process(now());
  Internal::processReadings();
}
