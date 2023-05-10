/*
 * PurpleHazeApp:
 *    This is the core of the MultiMon functionality. It is invoked
 *    by the primary sketch ".ino" file.
 *
 * NOTES:
 * o PurpleHazeApp is derived from WTAppImpl, which is itself derived
 *   from WTApp.
 * o This is a singleton and is made available to the whole app via
 *   an extern declaration in WTApp.h:
 *       WTApp* wtApp;
 * o This must be cast back to PurpleHazeApp* in order to get to app-specific
 *   data and functions. Technically this should be a dynamic_cast, but for
 *   efficiency, we perform a "C-style" cast.
 * o Macros are provided to easily get the app and settings in their
 *   specialized forms.
 *
 */


#ifndef PurpleHazeApp_h
#define PurpleHazeApp_h

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//                                  Third Party Libraries
#include <Indicators.h>
//                                  WebThing Includes
#include <WTAppImpl.h>
#include <sensors/WeatherMgr.h>
#include <sensors/AQIMgr.h>
#include <sensors/DevReadingsMgr.h>
//                                  Local Includes
#include "src/hardware/HWConfig.h"
#include "PHSettings.h"
#include "PHScreenConfig.h"
#include "src/hardware/SecondarySerial.h"
//--------------- End:    Includes ---------------------------------------------


// A few convenience macros:
// phApp simply yields the wtApp singleton cast as a PurpleHazeApp*
// phSettings yields the settings object from wtApp, cast as a PHSettings*
#define phApp ((PurpleHazeApp*)wtApp)
#define phSettings ((PHSettings*)phApp->settings)


class PurpleHazeApp : public WTAppImpl {
public:
  static void create();

  // CUSTOM: Data defined by this app which is available to the whole app
  AQIMgr aqiMgr;
  WeatherMgr weatherMgr;
  DevReadingsMgr devReadingsMgr;

  Indicator* sensorIndicator;
  Indicator* qualityIndicator;
  Indicator* busyIndicator;
  NeoPixelIndicators* indicators;

  // CUSTOM: Screens implemented by this app
  PHScreens appScreens;


  // ----- Functions that *must* be provided by subclasses
  virtual void app_registerDataSuppliers() override;
  virtual Screen* app_registerScreens() override;
  virtual void app_initWebUI() override;
  virtual void app_initClients() override;
  virtual void app_conditionalUpdate(bool force = false) override;
  virtual void app_loop() override;

  // ----- Functions that *may* be provided by subclasses
  virtual void app_configureHW() override;

  // ----- Public functions
  PurpleHazeApp(PHSettings* settings);
  void setIndicatorBrightness(uint8_t b);

protected:
  virtual void configModeCallback(const String &ssid, const String &ip) override;

private:

  void prepAIO();
  void prepSensors();
  void configureDisplay();
  void configurePins();
  void configureIndicators();
  void aboutToSleep();


  SecondarySerial streamToSensor;
};

#endif	// PurpleHazeApp_h
