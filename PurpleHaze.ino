/*
 * PurpleHaze - Monitor an air quality sensor and upload data to the Blynk Service
 *                    
 * NOTES:
 *
 */


//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//                                  Third Party Libraries
#include <ArduinoLog.h>
#include <TimeLib.h>
#include <WebUI.h>
//                                  Local Includes
#include "HWConfig.h"
#include "PurpleHaze.h"
#include "SecondarySerial.h"
#include "PHWebUI.h"
#include "src/clients/WeatherMgr.h"
#include "src/utils/Output.h"
#include "src/clients/AQIMgr.h"
#include "src/clients/BlynkMgr.h"
#include "src/clients/WeatherBlynkPublisher.h"
#include "src/clients/AQIBlynkPublisher.h"
//--------------- End:    Includes ---------------------------------------------


namespace PH {
  /*------------------------------------------------------------------------------
   *
   * Global State
   *
   *----------------------------------------------------------------------------*/
  AQIMgr aqiMgr;
  #if defined(HAS_THP_SENSOR)
    WeatherMgr weatherMgr;
  #endif

  Indicator* sensorIndicator;
  Indicator* qualityIndicator;
  Indicator* busyIndicator;
  NeoPixelIndicators* indicators;

  PHSettings settings;


  namespace Internal {
    static const String   SettingsFileName = "/settings.json";
    SecondarySerial streamToSensor;

    void ensureWebThingSettings() {
      // PH has a fixed hardware configuration so some of the settings need to have
      // specific values. Rather than make the user configure them, set them to the
      // right values here.
      WebThing::displayPowerOptions(false);
      WebThing::settings.sleepOverridePin = -1;
      WebThing::settings.hasVoltageSensing = false;
      WebThing::replaceEmptyHostname("ph-");
    }

    void flushBeforeSleep() { BlynkMgr::disconnect(); }

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
      qualityIndicator->setColor(0x969697);  // No data available yet
      delay(1000);
    }

    void prepSensors() {
      // Start with the Air Quality Sensor
      streamToSensor.begin();

      if (!aqiMgr.init(streamToSensor.s, sensorIndicator)) {
        Log.error("Unable to connect to Air Quality Sensor!");
        qualityIndicator->setColor(255, 0, 0);
        sensorIndicator->setColor(255, 0, 0);
        busyIndicator->setColor(255, 0, 0);
      }

      #if defined(HAS_THP_SENSOR)
        if (SDA_PIN >= 0 && SCL_PIN >= 0) {
          // Override the deault I2C Pins
          Wire.begin(SDA_PIN, SCL_PIN);
        }
        weatherMgr.setAttributes(
          settings.bmeSettings.tempCorrection,
          settings.bmeSettings.humiCorrection,
          WebThing::settings.elevation);
        weatherMgr.begin(BME_I2C_ADDR);
      #endif  // HAS_THP_SENSOR
    }

    void prepWebUI() {
      WebThing::setup();
      WebThing::notifyBeforeDeepSleep(flushBeforeSleep);
      PHWebUI::init();
    }

    void prepBlynk() {
      BlynkMgr::init(settings.blynkAPIKey);

      // ----- Register the BME Publisher
      #if defined(HAS_THP_SENSOR)
        WeatherBlynkPublisher* bp = new WeatherBlynkPublisher(&weatherMgr);
        BlynkMgr::registerPublisher(bp);
      #endif  // HAS_THP_SENSOR

      // ----- Register the AQI Publisher
      AQIBlynkPublisher* ap = new AQIBlynkPublisher(&aqiMgr);
      BlynkMgr::registerPublisher(ap);
    }  

    void baseConfigChange() { WebUI::setTitle(settings.description+" ("+WebThing::settings.hostname+")"); }

    void configModeCallback(const String &ssid, const String &ip) {
      (void)ssid; (void)ip;
      busyIndicator->setColor(0, 0, 255);
    }

    void processReadings() {
      static uint32_t lastTimestamp = 0;

      const AQIReadings& aqiReadings = aqiMgr.getLastReadings();
      if (aqiReadings.timestamp != lastTimestamp) {
        busyIndicator->setColor(0, 255, 0);
        uint16_t quality = aqiMgr.derivedAQI(aqiReadings.env.pm25);
        qualityIndicator->setColor(aqiMgr.colorForQuality(quality));
        lastTimestamp = aqiReadings.timestamp;
        busyIndicator->off();
      }

      #if defined(HAS_THP_SENSOR)
        weatherMgr.takeReadings();
      #endif  // HAS_THP_SENSOR

      BlynkMgr::publish();
    }
  } // ----- END: PH::Internal namespace


  /*------------------------------------------------------------------------------
   *
   * Exported Functions
   *
   *----------------------------------------------------------------------------*/


  void setIndicatorBrightness(uint8_t b) {
    if (indicators) indicators->setBrightness((b*255L)/100);
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
  Output::setUnits(&settings.useMetric);

  Internal::ensureWebThingSettings();         // Override any pertinent settings in WebThing
  WebThing::notifyOnConfigMode(Internal::configModeCallback);

  WebThing::notifyConfigChange(Internal::baseConfigChange);
                                    // If the WebThing config changes, let us know

  Internal::prepWebUI();            // Setup the WebUI, network, etc.
  busyIndicator->off();
  Internal::prepSensors();          // Prep the sensor after we're connected to the web
                                    // so we have time info for timestamps, etc.

  Internal::prepBlynk();

  Internal::processReadings();      // Get our first set of readings!

  WebThing::postSetup();            // Finalize setup of the WebThing - Must be last
}

void loop() {
  WebThing::loop();
  BlynkMgr::loop();
  aqiMgr.loop();
  Internal::processReadings();
}
