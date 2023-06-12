/*
 * PurpleHazeApp:
 *    A simple example of a WebThingApp that display times and currency
 *    exchange rates.
 *
 * CUSTOMIZATION:
 * o To create your own app based on this sample, search for 'CUSTOM'
 *   to find areas that should be changed/customized
 * o Areas labeled 'BOILERPLATE' may require mechanical changes such as updating
 *   the names you have used for classes and variables
 *
 */


//--------------- Begin:  Includes ---------------------------------------------
//                                  Hardware Definitions
#include "src/hardware/HWConfig.h"
//                                  Core Libraries
//                                  Third Party Libraries
#include <ArduinoLog.h>
#include <TimeLib.h>
//                                  WebThing Includes
#include <DataBroker.h>
#include <WebUI.h>
#include <clients/AIOMgr.h>
#include <sensors/WeatherUtils.h>
#include <clients/AIO_DevReadingsPublisher.h>
#include <clients/AIO_WeatherPublisher.h>
#include <clients/AIO_AQIPublisher.h>
//                                  WebThingApp Includes
#include <gui/Display.h>
#include <gui/ScreenMgr.h>
#include <plugins/PluginMgr.h>
#include <plugins/common/GenericPlugin.h>
#include <plugins/common/AIOPlugin.h>
//                                  Local Includes
#include "PurpleHazeApp.h"
#include "PHSettings.h"
#include "PHWebUI.h"
#include "PHDataSupplier.h"
#include "src/screens/AppTheme.h"
//--------------- End:    Includes ---------------------------------------------


//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//                                  Third Party Libraries
//                                  WebThing Includes
//                                  Local Includes
//--------------- End:    Includes ---------------------------------------------


/*------------------------------------------------------------------------------
 *
 * Constants
 *
 *----------------------------------------------------------------------------*/

// CUSTOM: Update these strings for your app:
static const char* VersionString = "0.1.0";
static const char* AppName = "PurpleHaze";
static const char* AppPrefix = "PH-";


/*------------------------------------------------------------------------------
 *
 * Private Utility Functions
 *
 *----------------------------------------------------------------------------*/

Plugin* pluginFactory(const String& type) {
  Plugin *p = NULL;

  // CUSTOM: Choose which plugins you'd like to load
  if      (type.equalsIgnoreCase("generic")) { p = new GenericPlugin(); }
  else if (type.equalsIgnoreCase("aio"))   { p = new AIOPlugin();   }  

  if (p == NULL) {
    Log.warning("Unrecognized plugin type: %s", type.c_str());
  }
  return p;
}

/*------------------------------------------------------------------------------
 *
 * Class function to create and start the PurpleHazeApp singleton
 *
 *----------------------------------------------------------------------------*/

static PHSettings theSettings;  // Allocate storage for the app settings

void PurpleHazeApp::create() {
  // BOILERPLATE
  PluginMgr::setFactory(pluginFactory);
  PurpleHazeApp* app = new PurpleHazeApp(&theSettings);

  app->begin(true, hwConfig.corePins.sda, hwConfig.corePins.scl);
}


/*------------------------------------------------------------------------------
 *
 * PurpleHazeApp Public Functions
 *
 *----------------------------------------------------------------------------*/


PurpleHazeApp::PurpleHazeApp(PHSettings* settings) :
    WTAppImpl(AppName, AppPrefix, VersionString, settings)
{
  // CUSTOM: Perform any object initialization here
  std::function<void()> sleepCB = [this]() { this->aboutToSleep(); };
  WebThing::notifyBeforeDeepSleep(sleepCB);
}


/*------------------------------------------------------------------------------
 *
 * Mandatory WTAppImpl virtual functions
 *
 *----------------------------------------------------------------------------*/

void PurpleHazeApp::app_registerDataSuppliers() {
  // BOILERPLATE
  DataBroker::registerMapper(PHDataSupplier::dataSupplier, PHDataSupplier::PHPrefix);
}

void PurpleHazeApp::app_initWebUI() {
  // BOILERPLATE
  PHWebUI::init();
}


void PurpleHazeApp::app_loop() {
  // CUSTOM: Perform any app-specific periodic activity
  // Note that app_conditionalUpdate() is called for you automatically on a
  // periodic basis, so no need to do that here.

#if defined(HAS_AQI_SENSOR)
  aqiMgr.loop();
#endif

}

void PurpleHazeApp::app_initClients() {
  // CUSTOM: If your app has any app-specific clients, initilize them now

  ScreenMgr.showActivityIcon(AppTheme::Color_Updating);
  prepAIO();
  ScreenMgr.hideActivityIcon();
}

void PurpleHazeApp::app_conditionalUpdate(bool force) {
  // CUSTOM: Update any app-specific clients
  static bool startingUp = true;

  #if defined(HAS_AQI_SENSOR)
    static uint32_t lastTimestamp = 0;

    const AQIReadings& aqiReadings = aqiMgr.getLastReadings();
    if (aqiReadings.timestamp != lastTimestamp) {
      busyIndicator->setColor(0, 255, 0);
      uint16_t quality = aqiMgr.derivedAQI(aqiReadings.env.pm25);
      qualityIndicator->setColor(aqiMgr.colorForQuality(quality));
      lastTimestamp = aqiReadings.timestamp;
      busyIndicator->off();
    }
  #endif

  #if defined(HAS_WEATHER_SENSOR)
    weatherMgr.takeReadings(force);
  #endif

  devReadingsMgr.takeReadings(force);

  if (!startingUp) AIOMgr::publish();
  startingUp = false;
}

Screen* PurpleHazeApp::app_registerScreens() {
  // CUSTOM: Associate a confirm/cancel buttons with the reboot screen
  screens.rebootScreen->setButtons(hwConfig.advanceButton, hwConfig.previousButton);

  // CUSTOM: Register any app-specific Screen objects
  return appScreens.registerScreens(&theSettings, pluginMgr, aqiMgr, weatherMgr);
}

/*------------------------------------------------------------------------------
 *
 * Functions for use by other parts of the app
 *
 *----------------------------------------------------------------------------*/

  void PurpleHazeApp::setIndicatorBrightness(uint8_t b) {
    if (indicators) indicators->setBrightness((b*255L)/100);
  }

/*------------------------------------------------------------------------------
 *
 * Optional WTAppImpl virtual functions
 *
 *----------------------------------------------------------------------------*/

void PurpleHazeApp::app_configureHW() {
  // CUSTOM: Configure/Initialize any app-specific hardware here
  // At this point, the settings have been read, but almost nothing
  // else has been done.

  configureDisplay(); // Sets display parameters
  configurePins();
  configureIndicators();
  prepSensors();
  
  // Technically this should be done later, since ScreenMgr.init() hasn't been
  // called yet.
  ScreenMgr.setSequenceButtons(hwConfig.advanceButton, hwConfig.previousButton);
}

/*------------------------------------------------------------------------------
 *
 * Protected functions
 *
 *----------------------------------------------------------------------------*/

void PurpleHazeApp::configModeCallback(const String &ssid, const String &ip) {
  busyIndicator->setColor(0, 0, 255);
  WTAppImpl::configModeCallback(ssid, ip);
}


/*------------------------------------------------------------------------------
 *
 * Private functions
 *
 *----------------------------------------------------------------------------*/

void PurpleHazeApp::aboutToSleep() {
  AIOMgr::publish();
  Display.setBrightness(0);
}

void PurpleHazeApp::prepAIO() {
  if (phSettings->aio.username.isEmpty() || phSettings->aio.key.isEmpty()) {
    Log.trace("PurpleHazeApp::prepAIO: AIO username or key is empty");
    return;
  }

    auto aioBusyCallBack = [this](bool busy) {
      if (busy) ScreenMgr.showActivityIcon(AppTheme::Color_Updating);
      else ScreenMgr.hideActivityIcon();
    };

  AIOMgr::init(phSettings->aio.username, phSettings->aio.key);
  AIOMgr::setBusyCB(aioBusyCallBack);
  AIOMgr::aio->setDefaultGroup(phSettings->aio.groupName.c_str());

  // ----- Register the BME Publisher
  #if defined(HAS_WEATHER_SENSOR)
    AIOMgr::registerPublisher(new AIO_WeatherPublisher(&weatherMgr));
  #endif

  // ----- Register the AQI Publisher
  #if defined(HAS_AQI_SENSOR)
    AIOMgr::registerPublisher(new AIO_AQIPublisher(&aqiMgr));
  #endif

  // ----- Register the DevReadings Publisher
  AIOMgr::registerPublisher(new AIO_DevReadingsPublisher(&devReadingsMgr));
}  

//
// ----- Hardware Configuration
//

void PurpleHazeApp::prepSensors() {
  #if defined(HAS_AQI_SENSOR)
    streamToSensor.begin();
    if (!aqiMgr.init(streamToSensor.s, sensorIndicator)) {
      Log.error("Unable to connect to Air Quality Sensor!");
      qualityIndicator->setColor(255, 0, 0);
      sensorIndicator->setColor(255, 0, 0);
      busyIndicator->setColor(255, 0, 0);
    }
  #endif

  #if defined(HAS_WEATHER_SENSOR)
    auto weatherBusyCallBack = [this](bool busy) {
      if (busy) busyIndicator->setColor(0, 255, 0);
      else busyIndicator->off();
    };

    WeatherUtils::configureAvailableSensors(weatherMgr);
    weatherMgr.init(
      phSettings->weatherSettings.tempCorrection,
      phSettings->weatherSettings.humiCorrection,
      WebThing::settings.elevation,
      weatherBusyCallBack);
  #endif
}

void PurpleHazeApp::configureDisplay() {
  // Set the display options before we fire up the display!
  Display.setDeviceOptions(&hwConfig.displayDeviceOptions);
}

void PurpleHazeApp::configurePins() {
  // Initialize the synthetic grounds
  for (int i = 0; i < hwConfig.nSyntheticGrounds; i++) {
    Basics::Pin pin = hwConfig.syntheticGrounds[i];
    if (pin != Basics::UnusedPin) {
      pinMode(pin, OUTPUT);
      digitalWrite(pin, LOW);      
    }
  }

  // Register any physical buttons that are connected
  for (int i = 0; i < hwConfig.nPhysicalButtons; i++) {
    Basics::Pin pin = hwConfig.physicalButtons[i];
    if (pin != Basics::UnusedPin) {
      WebThing::buttonMgr.addButton(pin);
    }
  }
}

void PurpleHazeApp::configureIndicators() {
  if (NEOPIXEL_PIN == Basics::UnusedPin) {
    indicators = NULL;
    qualityIndicator = new Indicator();
    sensorIndicator = new Indicator();
    busyIndicator = new Indicator();
  } else {
    indicators = new NeoPixelIndicators(NEOPIXEL_PIN, 3, NEOPIXEL_TYPE);
    indicators->begin();
    indicators->setBrightness(phSettings->iBright);
    NeoPixelIndicator* npi;
    npi = new NeoPixelIndicator(); npi->begin(indicators, 0); qualityIndicator = npi;
    npi = new NeoPixelIndicator(); npi->begin(indicators, 1); sensorIndicator = npi;
    npi = new NeoPixelIndicator(); npi->begin(indicators, 2); busyIndicator = npi;
  }
  qualityIndicator->setColor(0x969697);  // No data available yet
  sensorIndicator->off();
  busyIndicator->off();
}

