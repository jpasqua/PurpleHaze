/*
 * PHBlynk
 *    Write PurpleHaze data to the Blynk service
 *
 */

#ifndef PHBlynk_h
#define PHBlynk_h

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//                                  Third Party Libraries
//                                  Local Includes
#include "PurpleHaze.h"
#include "PMS5003.h"
//--------------- End:    Includes ---------------------------------------------


namespace PHBlynk {
  // ----- Constants
  static const int BlynkEnv010        = 0;
  static const int BlynkEnv025        = 1;
  static const int BlynkEnv100        = 2;
  static const int BlynkTimestampPin  = 3;
  static const int BlynkMA30          = 4;

  // ----- State
  bool connectedToBlynk = false;

  // ----- Methods
  void init() {
    Log.trace("Connecting to Blynk");
    if (PH::settings.blynkAPIKey.length() != 0) {
      Blynk.config(PH::settings.blynkAPIKey.c_str());
      connectedToBlynk = Blynk.connect(30*1000L);
      if (connectedToBlynk) { Log.trace("Successfully connected to Blynk"); }
      else { Log.warning("Unable to connect to Blynk"); }
    } else {
      Log.notice("No Blynk API key was specified.");
    }
  }

  void run() {
    if (connectedToBlynk) Blynk.run();
  }

  // Because this function unconditionally writes data to Blynk, it is up to the caller
  // to ensure that there is new data to write since the last update.
  void update() {
    if (!connectedToBlynk) return;
    Log.trace("About to update Blynk...");
    Blynk.virtualWrite(BlynkEnv010, PH::latestData.pm10_env);
    Blynk.virtualWrite(BlynkEnv025, PH::latestData.pm25_env);
    Blynk.virtualWrite(BlynkEnv100, PH::latestData.pm100_env);
    Blynk.virtualWrite(BlynkMA30, (uint16_t)PH::aqiReader.pm25_env_30min.getAverage());
    Blynk.virtualWrite(BlynkTimestampPin, PH::formattedTime(PH::latestData.timestamp));
    Log.trace("Blynk update complete");
  }

  void disconnect() {
    if (connectedToBlynk) {
      Blynk.run();
      Blynk.disconnect();
    }
  }

}

#endif // PHBlynk_h
