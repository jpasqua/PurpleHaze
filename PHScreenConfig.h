/*
 * PHScreenConfig
 *     Configure the app-specific screens used by PurpleHaze
 *
 */


#ifndef PHScreenConfig_h
#define PHScreenConfig_h

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//                                  Third Party Libraries
//                                  WebThing Includes
#include <plugins/PluginMgr.h>
#include <gui/ScreenMgr.h>
#include <screens/oled/SensorGraphScreen.h>
//                                  Local Includes
#include "src/screens/SplashScreen.h"
#include "src/screens/HomeScreen.h"
#include "src/screens/AQIScreen.h"
#include "src/screens/ReadingScreen.h"
//--------------- End:    Includes ---------------------------------------------


class PHScreens {
public:
  SplashScreen*       splashScreen;
  HomeScreen*         homeScreen;
#if defined(HAS_AQI_SENSOR)
  AQIScreen*          aqiScreen;
  AQIGraphScreen*     aqiGraphScreen;
#endif
#if defined(HAS_WEATHER_SENSOR)
  WeatherGraphScreen*	weatherGraphScreen;
  TempScreen* 				tempScreen;
  HumidityScreen* 		humiScreen;
  BaroScreen* 				baroScreen;
#endif

	Screen* registerScreens(
		PHSettings* settings, PluginMgr& pluginMgr,
		const AQIMgr& aqiMgr, const WeatherMgr& weatherMgr)
{
		uint8_t weatherReadings = weatherMgr.availableReadingTypes();

	  // CUSTOM: Register any app-specific Screen objects
	  splashScreen = new SplashScreen();
	  homeScreen = new HomeScreen();

    ScreenMgr.registerScreen("Home", homeScreen);
    ScreenMgr.setAsHomeScreen(homeScreen);
    ScreenMgr.registerScreen("Splash", splashScreen, true);

#if defined(HAS_AQI_SENSOR)
	  aqiScreen = new AQIScreen();
	  aqiGraphScreen = new AQIGraphScreen(aqiMgr);
	  ScreenMgr.registerScreen("AQI", aqiScreen);
	  ScreenMgr.registerScreen("AQI-Graph", aqiGraphScreen);
	  aqiGraphScreen->selectBuffer(settings->aqiSettings.graphRange);
#else
	  (void)aqiMgr;
#endif
#if defined(HAS_WEATHER_SENSOR)
	  if (weatherReadings && READ_TEMP) {
	  	tempScreen = new TempScreen();
  	  weatherGraphScreen = new WeatherGraphScreen(weatherMgr);
		  weatherGraphScreen->selectBuffer(settings->weatherSettings.graphRange);
		  ScreenMgr.registerScreen("Temp", tempScreen);
		  ScreenMgr.registerScreen("Temp-Graph", weatherGraphScreen);
		}
	  if (weatherReadings && READ_HUMI) {
	  	humiScreen = new HumidityScreen();
		  ScreenMgr.registerScreen("Humidity", humiScreen);
		}
	  if (weatherReadings && READ_PRES) {
	  	baroScreen = new BaroScreen();
		  ScreenMgr.registerScreen("Pressure", baroScreen);
		}
#else
	  (void)weatherMgr;
#endif

	  // CUSTOM: Add a sequence of screens that the user can cycle through
	  auto& sequence = ScreenMgr.sequence;
    sequence.clear();
    sequence.push_back(homeScreen);
#if defined(HAS_AQI_SENSOR)
	  sequence.push_back(aqiScreen);
	  sequence.push_back(aqiGraphScreen);
#endif
#if defined(HAS_WEATHER_SENSOR)
	  if (weatherReadings && READ_TEMP) {
	  	sequence.push_back(weatherGraphScreen);
		  sequence.push_back(tempScreen);
		}
	  if (weatherReadings && READ_HUMI) sequence.push_back(humiScreen);
	  if (weatherReadings && READ_PRES) sequence.push_back(baroScreen);
#endif
	  sequence.push_back(wtAppImpl->screens.weatherScreen);
	  sequence.push_back(wtAppImpl->screens.forecastFirst3);
	  sequence.push_back(wtAppImpl->screens.forecastLast2);
	  // Add any plugins to the sequence
	  uint8_t nPlugins = pluginMgr.getPluginCount();
	  for (int i = 0; i < nPlugins; i++) {
	    Plugin* p = pluginMgr.getPlugin(i);
	    sequence.push_back(p->getFlexScreen());
	  }
	  sequence.push_back(wtAppImpl->screens.infoScreen);

  // Log.verbose("PHScreens::registerScreens, The default screen sequence is:");
  // for (const Screen* s : sequence) {
  // 	Log.verbose("%s", s->name.c_str());
  // }
    
  // Reconcile the screen list from the settings, with the list of
  // screens we're actually using.
  ScreenMgr.reconcileScreenSequence(settings->screenSettings);

	  return splashScreen;
	}
};

#endif	// PHScreenConfig_h