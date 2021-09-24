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
#include <Output.h>
//                                  WebThing Includes
#include <sensors/WeatherMgr.h>
//                                  Local Includes
#include "src/hardware/HWConfig.h"
#include "PurpleHaze.h"
#include "PHWebUI.h"
#include "src/clients/AQIMgr.h"
//--------------- End:    Includes ---------------------------------------------



// ----- BEGIN: WebUI namespace
namespace PHWebUI {
  static const String   checkedOrNot[2] = {"", "checked='checked'"};

  // ----- BEGIN: PHWebUI::Internal
  namespace Internal {
    const __FlashStringHelper* APP_MENU_ITEMS = FPSTR(
      "<a class='w3-bar-item w3-button' href='/ChartPage.html'>"
      "<i class='fa fa-bar-chart'></i> Charts</a>"
      "<a class='w3-bar-item w3-button' href='/displayPHConfig'>"
      "<i class='fa fa-cog'></i> Configure PH</a>");
    const __FlashStringHelper* DEV_MENU_ITEMS = FPSTR(
      "<a class='w3-bar-item w3-button' href='/dev'>"
      "<i class='fa fa-gears'></i> Dev Settings</a>");

    constexpr uint32_t BusyColor = 0xff88ff;
    void showBusyStatus(bool busy) {
      if (busy) PH::busyIndicator->setColor(BusyColor);
      else PH::busyIndicator->off();
    }
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
      auto mapper =[](const String &key, String& val) -> void {
        const AQIReadings& aqiReadings = PH::aqiMgr.getLastReadings();
        if      (key == "LAT")      val = WebThing::settings.latAsString();
        else if (key == "LNG")      val = WebThing::settings.lngAsString();
        else if (key == "AQI")      val.concat(PH::aqiMgr.derivedAQI(aqiReadings.env.pm25));
        else if (key == "PM10STD")  val.concat(aqiReadings.standard.pm10);
        else if (key == "PM25STD")  val.concat(aqiReadings.standard.pm25);
        else if (key == "PM100STD") val.concat(aqiReadings.standard.pm100);
        else if (key == "PM10ENV")  val.concat(aqiReadings.env.pm10);
        else if (key == "PM25ENV")  val.concat(aqiReadings.env.pm25);
        else if (key == "PM100ENV") val.concat(aqiReadings.env.pm100);
        else if (key == "P03")      val.concat(aqiReadings.particles_03um);
        else if (key == "P05")      val.concat(aqiReadings.particles_05um);
        else if (key == "P10")      val.concat(aqiReadings.particles_10um);
        else if (key == "P25")      val.concat(aqiReadings.particles_25um);
        else if (key == "P50")      val.concat(aqiReadings.particles_50um);
        else if (key == "P100")     val.concat(aqiReadings.particles_100um);
        else if (key == "TMST")     val = Output::formattedTime(Basics::wallClockFromMillis(aqiReadings.timestamp));
        else if (key == "MA10")     val.concat(round(PH::aqiMgr.pm25env_10min.getAverage()));
        else if (key == "MA30")     val.concat(round(PH::aqiMgr.pm25env_30min.getAverage()));
        else if (key == "MA1H")     val.concat(round(PH::aqiMgr.pm25env_1hr.getAverage()));
        else if (key == "MA6H")     val.concat(round(PH::aqiMgr.pm25env_6hr.getAverage()));
      };

      WebUI::wrapWebPage("/", "/HomePage.html", mapper);
    }

    void displayChartPage() {
      auto mapper =[](const String &key, String& val) -> void {
        if      (key == "PM10_CLR")  val = PH::settings.chartColors.pm10;
        else if (key == "PM25_CLR")  val = PH::settings.chartColors.pm25;
        else if (key == "PM100_CLR") val = PH::settings.chartColors.pm100;
        else if (key == "AQI_CLR")   val = PH::settings.chartColors.aqi;
        else if (key == "TMP_CLR")   val = PH::settings.weatherSettings.chartColors.temp;
        else if (key == "USE_METRIC")val = PH::settings.useMetric;
      };

      WebUI::wrapWebPage("/displayChartPage", "/ChartPage.html", mapper);
    }

    // Displays a form allowing the user to update the PurpleHaze settings.
    //
    // Form:
    //    GET  /displayPHConfig
    //
    void displayPHConfig() {
      auto mapper =[](const String &key, String& val) -> void {
        if      (key == "DESC")       val = WebThing::encodeAttr(PH::settings.description);
        else if (key == "BLYNK_KEY")  val = PH::settings.blynkAPIKey;
        else if (key == "I_BRIGHT")   val.concat(PH::settings.iBright);
        else if (key == "PM10_CLR")   val = PH::settings.chartColors.pm10;
        else if (key == "PM25_CLR")   val = PH::settings.chartColors.pm25;
        else if (key == "PM100_CLR")  val = PH::settings.chartColors.pm100;
        else if (key == "AQI_CLR")    val = PH::settings.chartColors.aqi;
        else if (key == "USE_METRIC") val = checkedOrNot[PH::settings.useMetric];
        else if (key == "USE_24HOUR") val = checkedOrNot[PH::settings.use24Hour];
#if defined(HAS_WEATHER_SENSOR)
        else if (key == "WTHR_VIS")   val = "true";
        else if (key == "TEMP_CORRECT") val.concat(PH::settings.weatherSettings.tempCorrection);
        else if (key == "HUMI_CORRECT") val.concat(PH::settings.weatherSettings.humiCorrection);
        else if (key == "TEMP_CLR")   val = PH::settings.weatherSettings.chartColors.temp;
        else if (key == "AVG_CLR")    val = PH::settings.weatherSettings.chartColors.avg;
#else
        else if (key == "WTHR_VIS")   val = "false";
#endif
      };

      WebUI::wrapWebPage("/displayPHConfig", "/ConfigForm.html", mapper);
    }
  }   // ----- END: PHWebUI::Pages


  namespace Endpoints {
    void getHistory() {
      auto action = []() {
        String rangeArg = WebUI::arg("range");
        AQIMgr::HistoryRange range;

        if (rangeArg.equalsIgnoreCase("hour")) range = AQIMgr::HistoryRange::Range_1Hour;
        else if (rangeArg.equalsIgnoreCase("day")) range = AQIMgr::HistoryRange::Range_1Day;
        else if (rangeArg.equalsIgnoreCase("week")) range = AQIMgr::HistoryRange::Range_1Week;
        else range = AQIMgr::HistoryRange::Range_Combined;

        auto provider = [range](Stream& s) -> void {
          PH::aqiMgr.emitHistoryAsJson(range, s);
        };

        WebUI::sendArbitraryContent("application/json", -1, provider);
      };

      WebUI::wrapWebAction("/getHistory", action, false);
    }

#if defined(HAS_WEATHER_SENSOR)
    void getWeatherHistory() {
      auto action = []() {
        String rangeArg = WebUI::arg("range");
        WeatherMgr::HistoryRange range;

        if (rangeArg.equalsIgnoreCase("hour")) range = WeatherMgr::HistoryRange::Range_1Hour;
        else if (rangeArg.equalsIgnoreCase("day")) range = WeatherMgr::HistoryRange::Range_1Day;
        else if (rangeArg.equalsIgnoreCase("week")) range = WeatherMgr::HistoryRange::Range_1Week;
        else range = WeatherMgr::HistoryRange::Range_Combined;

        auto provider = [range](Stream& s) -> void {
          PH::weatherMgr.emitHistoryAsJson(range, s);
        };

        WebUI::sendArbitraryContent("application/json", -1, provider);
      };

      WebUI::wrapWebAction("/getWeatherHistory", action, false);
    }
#else
    void getWeatherHistory() {
      WebUI::redirectHome();
    }
#endif

    void getAQI() {
      auto action = []() {
        String result;
        result.reserve(300);
        const AQIReadings& aqiReadings = PH::aqiMgr.getLastReadings();
        PH::aqiMgr.aqiAsJSON(
          PH::aqiMgr.derivedAQI(aqiReadings.env.pm25),
          aqiReadings.timestamp, result);
        WebUI::sendStringContent("application/json", result);
      };

      WebUI::wrapWebAction("/getHistory", action, false);
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
      auto action = []() {
        PH::settings.description = WebUI::arg("description");
        PH::settings.blynkAPIKey = WebUI::arg("blynkAPIKey");
        PH::settings.iBright = (constrain(WebUI::arg("iBright").toInt(), 0, 100));
        PH::settings.useMetric = WebUI::hasArg(F("metric"));
        PH::settings.use24Hour = WebUI::hasArg(F("is24hour"));
        PH::settings.chartColors.pm10 = WebUI::arg("pm10Color");
        PH::settings.chartColors.pm25 = WebUI::arg("pm25Color");
        PH::settings.chartColors.pm100 = WebUI::arg("pm100Color");
        PH::settings.chartColors.aqi = WebUI::arg("aqiColor");
        PH::settings.weatherSettings.chartColors.temp = WebUI::arg("tempColor");
        PH::settings.weatherSettings.chartColors.avg = WebUI::arg("avgColor");
        PH::settings.write();

        // The description MAY have changed. Update the title just in case
        PH::setIndicatorBrightness(PH::settings.iBright);
        WebUI::setTitle(PH::settings.description+" ("+WebThing::settings.hostname+")");
        WebUI::redirectHome();
      };

      WebUI::wrapWebAction("/updatePHConfig", action, false);
    }
  }   // ----- END: PHWebUI::Endpoints

  void init() {
    WebUI::setTitle(PH::settings.description+" ("+WebThing::settings.hostname+")");

    WebUI::addAppMenuItems(Internal::APP_MENU_ITEMS);
    WebUI::Dev::init(
        &PH::settings.showDevMenu, nullptr, &PH::settings, Internal::DEV_MENU_ITEMS);

    WebUI::registerBusyCallback(Internal::showBusyStatus);

    WebUI::registerHandler("/", Pages::displayHomePage);
    WebUI::registerHandler("/ChartPage.html",     Pages::displayChartPage);
    WebUI::registerHandler("/displayPHConfig",    Pages::displayPHConfig);

    WebUI::registerHandler("/updatePHConfig",     Endpoints::updatePHConfig);
    WebUI::registerHandler("/getHistory",         Endpoints::getHistory);
    WebUI::registerHandler("/getWeatherHistory",  Endpoints::getWeatherHistory);
    WebUI::registerHandler("/getAQI",             Endpoints::getAQI);
  }

}
// ----- END: PHWebUI
