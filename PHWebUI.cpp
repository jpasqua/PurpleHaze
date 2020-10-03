/*
 * PHWebUI:
 *    Implements a simple WebUI for PurpleHaze
 *                    
 * NOTES:
 *
 */


//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//                                  Third Party Libraries
#include <ArduinoLog.h>
//                                  Local Includes
#include "PurpleHaze.h"
#include "PHWebUI.h"
//--------------- End:    Includes ---------------------------------------------



// ----- BEGIN: WebUI namespacea
namespace PHWebUI {
  static const String   checkedOrNot[2] = {"", "checked='checked'"};
  ESPTemplateProcessor  *templateHandler;

  // ----- BEGIN: PHWebUI::Internal
  namespace Internal {
    String Actions =
      "<a class='w3-bar-item w3-button' href='/ChartPage.html'>"
      "<i class='fa fa-bar-chart'></i> Charts</a>"
      "<a class='w3-bar-item w3-button' href='/displayPHConfig'>"
      "<i class='fa fa-cog'></i> Configure PH</a>";
    String DEV_ACTION =
      "<a class='w3-bar-item w3-button' href='/dev'>"
      "<i class='fa fa-gears'></i> Dev Settings</a>";
  }
  // ----- END: PHWebUI::Internal


  // ----- BEGIN: PHWebUI::Endpoints
  namespace Pages {

    // Displays the home page which shows the last set of weather readings.
    //
    // Form:
    //    GET  /displayHomePage
    //
    void displayHomePage() {
      Log.trace("Web Request: Display Home Page");
      if (!WebUI::authenticationOK()) { return; }

      auto mapper =[](String &key) -> String {
        if (key == "LAT")  return WebThing::settings.latAsString();
        if (key == "LNG")  return WebThing::settings.lngAsString();
        if (key == "AQI") return (String(PH::aqiReader.derivedAQI(PH::latestData.pm25_env)));
        if (key == "PM10STD") return (String(PH::latestData.pm10_standard));
        if (key == "PM25STD") return (String(PH::latestData.pm25_standard));
        if (key == "PM100STD") return (String(PH::latestData.pm100_standard));
        if (key == "PM10ENV") return (String(PH::latestData.pm10_env));
        if (key == "PM25ENV") return (String(PH::latestData.pm25_env));
        if (key == "PM100ENV") return (String(PH::latestData.pm100_env));
        if (key == "P03") return (String(PH::latestData.particles_03um));
        if (key == "P05") return (String(PH::latestData.particles_05um));
        if (key == "P10") return (String(PH::latestData.particles_10um));
        if (key == "P25") return (String(PH::latestData.particles_25um));
        if (key == "P50") return (String(PH::latestData.particles_50um));
        if (key == "P100") return (String(PH::latestData.particles_100um));
        if (key == "TMST") return String(PH::formattedTime(PH::latestData.timestamp));
        if (key == "MA10") return (String(PH::aqiReader.pm25_env_10min.getAverage(), 0));
        if (key == "MA30") return (String(PH::aqiReader.pm25_env_30min.getAverage(), 0));
        if (key == "MA1H") return (String(PH::aqiReader.pm25_env_1hr.getAverage(), 0));
        if (key == "MA6H") return (String(PH::aqiReader.pm25_env_6hr.getAverage(), 0));
        return "";
      };

      WebUI::startPage();
      templateHandler->send("/HomePage.html", mapper);
      WebUI::finishPage();
    }

    void displayChartPage() {
      Log.trace("Web Request: Display Chart Page");
      if (!WebUI::authenticationOK()) { return; }

      auto mapper =[](String &key) -> String {
        if (key == "PM10_CLR")  return PH::settings.chartColors.pm10;
        if (key == "PM25_CLR")  return PH::settings.chartColors.pm25;
        if (key == "PM100_CLR")  return PH::settings.chartColors.pm100;
        if (key == "AQI_CLR")  return PH::settings.chartColors.aqi;
        return "";
      };

      WebUI::startPage();
      templateHandler->send("/ChartPage.html", mapper);
      WebUI::finishPage();
    }

    // Displays a form allowing the user to update the PurpleHaze settings.
    //
    // Form:
    //    GET  /displayPHConfig
    //
    void displayPHConfig() {
      Log.trace("Web Request: Display Config");
      if (!WebUI::authenticationOK()) { return; }

      auto mapper =[](String &key) -> String {
        if (key == "DESC") return WebThing::encodeAttr(PH::settings.description);
        if (key == "BLYNK_KEY")  return PH::settings.blynkAPIKey;
        if (key == "I_BRIGHT")  return String(PH::settings.iBright);
        if (key == "PM10_CLR")  return PH::settings.chartColors.pm10;
        if (key == "PM25_CLR")  return PH::settings.chartColors.pm25;
        if (key == "PM100_CLR")  return PH::settings.chartColors.pm100;
        if (key == "AQI_CLR")  return PH::settings.chartColors.aqi;
        return "";
      };

      WebUI::startPage();
      templateHandler->send("/ConfigForm.html", mapper);
      WebUI::finishPage();
    }
  }   // ----- END: PHWebUI::Pages


  namespace Endpoints {
    void getHistory() {
      String rangeArg = WebUI::arg("range");
      AQIReader::HistoryRange range;

      if (rangeArg.equalsIgnoreCase("hour")) range = AQIReader::HistoryRange::Range_1Hour;
      else if (rangeArg.equalsIgnoreCase("day")) range = AQIReader::HistoryRange::Range_1Day;
      else if (rangeArg.equalsIgnoreCase("week")) range = AQIReader::HistoryRange::Range_1Week;
      else range = AQIReader::HistoryRange::Range_Combined;

      auto provider = [range](Stream& s) -> void {
        PH::aqiReader.emitHistoryAsJson(range, s);
      };

      WebUI::sendArbitraryContent("application/json", -1, provider);
    }


    // Handler for the "/updatePHConfig" endpoint. This is invoked as the target
    // of the form presented by "/displayPHConfig". It updates the values of the
    // corresponding settings and writes the settings to EEPROM.
    //
    // TO DO: Call a function to let the main app know that settings may have changed
    //        so that it can take any appropriate actions
    //
    // Form:
    //    GET /updatePHConfig?description=DESC&iBright=INT&...
    //
    void updatePHConfig() {
      if (!WebUI::authenticationOK()) { return; }
      Log.trace("PHWebUI: Handle Update Config");

      PH::settings.description = WebUI::arg("description");
      PH::settings.blynkAPIKey = WebUI::arg("blynkAPIKey");
      PH::settings.iBright = (constrain(WebUI::arg("iBright").toInt(), 0, 100));
      PH::settings.chartColors.pm10 = WebUI::arg("pm10Color");
      PH::settings.chartColors.pm25 = WebUI::arg("pm25Color");
      PH::settings.chartColors.pm100 = WebUI::arg("pm100Color");
      PH::settings.write();

      // The description MAY have changed. Update the title just in case
      WebUI::setTitle(PH::settings.description+" ("+WebThing::settings.hostname+")");
      PH::setIndicatorBrightness(PH::settings.iBright);

      WebUI::redirectHome();
    }
  }   // ----- END: PHWebUI::Endpoints

  namespace Dev {
    void updateSettings() {
      if (!WebUI::authenticationOK()) { return; }
      Log.trace("Web Request: /dev/updateSettings");

      PH::settings.showDevMenu = WebUI::hasArg("showDevMenu");
      PH::settings.write();

      WebUI::redirectHome();
    }

    void displayDevPage() {
      Log.trace(F("Web Request: /dev/displayDevPage"));
      if (!WebUI::authenticationOK()) { return; }

      auto mapper =[](String &key) -> String {
        if (key == "SHOW_DEV_MENU") return checkedOrNot[PH::settings.showDevMenu];
        return "";
      };

      WebUI::startPage();
      templateHandler->send("/DevPage.html", mapper);
      WebUI::finishPage();
    }

    void reboot() {
      if (!WebUI::authenticationOK()) { return; }
      ESP.restart();
    }

    void yieldSettings() {
      Log.trace(F("Web Request: /dev/settings"));
      if (!WebUI::authenticationOK()) { return; }

      DynamicJsonDocument *doc = (WebUI::hasArg("wt")) ? WebThing::settings.asJSON() :
                                                         PH::settings.asJSON();
      WebUI::sendJSONContent(doc);
      doc->clear();
      delete doc;
    }

  }   // ----- END: PHWebUI::Dev


  void init() {
    WebUI::setTitle(PH::settings.description+" ("+WebThing::settings.hostname+")");
    String actions = Internal::Actions;
    if (PH::settings.showDevMenu) {
      actions += Internal::DEV_ACTION;
    }
    WebUI::addMenuItems(actions);

    WebUI::registerHandler("/", Pages::displayHomePage);
    WebUI::registerHandler("/ChartPage.html",     Pages::displayChartPage);
    WebUI::registerHandler("/displayPHConfig",    Pages::displayPHConfig);

    WebUI::registerHandler("/updatePHConfig",     Endpoints::updatePHConfig);
    WebUI::registerHandler("/getHistory",         Endpoints::getHistory);

    WebUI::registerHandler("/dev",                Dev::displayDevPage);
    WebUI::registerHandler("/dev/reboot",         Dev::reboot);
    WebUI::registerHandler("/dev/settings",       Dev::yieldSettings);
    WebUI::registerHandler("/dev/updateSettings", Dev::updateSettings);

    templateHandler = WebUI::getTemplateHandler();
  }

}
// ----- END: PHWebUI
