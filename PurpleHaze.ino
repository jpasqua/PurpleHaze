/*
 * AQM - Monitor an air quality sensor and upload data to the Blynk Service
 *                    
 * NOTES:
 *
 */


//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <SoftwareSerial.h>
#include <ESP8266mDNS.h>
//                                  Third Party Libraries
#include <ArduinoLog.h>
#include <BlynkSimpleEsp8266.h>
#include <TimeLib.h>
#include <WebUI.h>
//                                  Local Includes
#include "AQM.h"
#include "HWConfig.h"
#include "AQIReader.h"
#include "Indicators.h"
#include "AQMWebUI.h"
#include "AQMBlynk.h"
//--------------- End:    Includes ---------------------------------------------


namespace AQM {
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
  AQMSettings settings;
  AQIReadings latestData;

  namespace Internal {
    static const uint16_t QualityBrackets[] = {50, 100, 150, 200, 150, 300, 350};
    static const uint16_t nBrackets = sizeof(QualityBrackets)/sizeof(QualityBrackets[0]);
    static const uint32_t QualityColors[] = {
      0x6DDF4E,   //   0- 49 GOOD
      0xFFFF00,   //  50- 99
      0xEB6036,   // 100-149
      0xE7352E,   // 150-199
      0x8A1E4B,   // 200-249
      0x831B40,   // 250-299
      0x721527,   // 300-349
      0x721627    // 350+
    };
    static const String   SettingsFileName = "/settings.json";

    void ensureWebThingSettings() {
      // AQM has a fixed hardware configuration so some of the settings need to have
      // specific values. Rather than make the user configure them, set them to the
      // right values here.
      WebThing::displayPowerOptions(false);
      WebThing::settings.sleepOverridePin = -1;
      WebThing::settings.hasVoltageSensing = false;
      if (WebThing::settings.hostname.isEmpty()) WebThing::settings.hostname = ("aqm" + String(ESP.getChipId(), HEX));
    }

    void flushBeforeSleep() { AQMBlynk::disconnect(); }

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
      AQMWebUI::init();
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
      uint16_t quality = latestData.pm25_env;
      int bracketIndex;
      for (bracketIndex = 0; bracketIndex < nBrackets; bracketIndex++) {
        if (quality < QualityBrackets[bracketIndex]) break;
      }
      qualityIndicator->setColor(QualityColors[bracketIndex]);

      AQMBlynk::update();
      lastTimestamp = latestData.timestamp;
      busyIndicator->off();
    }
  } // ----- END: AQM::Internal namespace


  /*------------------------------------------------------------------------------
   *
   * Exported Functions
   *
   *----------------------------------------------------------------------------*/

  void setIndicatorBrightness(uint8_t b) {
    if (indicators) indicators->setBrightness((b*255L)/100);
  }

  char *formattedTime(time_t theTime) {
    static char dateTime[19];
    sprintf(dateTime, "%d/%02d/%02d %02d:%02d:%02d", 
      year(theTime), month(theTime), day(theTime), hour(theTime), minute(theTime), second(theTime));
    return dateTime;
  }

} // ----- END: AQM namespace


/*------------------------------------------------------------------------------
 *
 * GLOBAL SCOPE: setup() and loop()
 *
 * The setup() and loop() functions need to be in the global scope, but are logically
 * part of the AQM namespace. Use a "using" directive for JUST THESE 2 functions
 * to treat them as if they were in that namespace.
 *
 *----------------------------------------------------------------------------*/

using namespace AQM;

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

  AQMBlynk::init();                 // Setup the Blynk Client

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
  AQMBlynk::run();
  aqiReader.process(now());
  Internal::processReadings();
}

