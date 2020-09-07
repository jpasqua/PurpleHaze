/*
 * WebUI:
 *    Implements a simple WebUI for a WebThing. It contains the basic
 *    structure that can be extended to incorporate "thing-specific" UI
 *                    
 * NOTES:
 *
 */


//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <ESP8266WebServer.h>
//                                  Third Party Libraries
#include <ArduinoLog.h>
//                                  Local Includes
#include "AQM.h"
#include "AQMWebUI.h"
//--------------- End:    Includes ---------------------------------------------



// ----- BEGIN: WebUI namespacea
namespace AQMWebUI {
  static const String   checkedOrNot[2] = {"", "checked='checked'"};
  ESPTemplateProcessor  *templateHandler;

  // ----- BEGIN: AQMWebUI::Internal
  namespace Internal {
    String Actions =
      "<a class='w3-bar-item w3-button' href='/displayAQMConfig'>"
      "<i class='fa fa-cog'></i> Configure AQM</a>";
  }
  // ----- END: AQMWebUI::Internal


  // ----- BEGIN: AQMWebUI::Endpoints
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
        if (key == "PM10STD") return (String(AQM::latestData.pm10_standard));
        if (key == "PM25STD") return (String(AQM::latestData.pm25_standard));
        if (key == "PM100STD") return (String(AQM::latestData.pm100_standard));
        if (key == "PM10ENV") return (String(AQM::latestData.pm10_env));
        if (key == "PM25ENV") return (String(AQM::latestData.pm25_env));
        if (key == "PM100ENV") return (String(AQM::latestData.pm100_env));
        if (key == "P03") return (String(AQM::latestData.particles_03um));
        if (key == "P05") return (String(AQM::latestData.particles_05um));
        if (key == "P10") return (String(AQM::latestData.particles_10um));
        if (key == "P25") return (String(AQM::latestData.particles_25um));
        if (key == "P50") return (String(AQM::latestData.particles_50um));
        if (key == "P100") return (String(AQM::latestData.particles_100um));
        if (key == "TMST") return String(AQM::formattedTime(AQM::latestData.timestamp));
        if (key == "MA10") return (String(AQM::aqiReader.pm25_env_10min.getAverage(), 0));
        if (key == "MA30") return (String(AQM::aqiReader.pm25_env_30min.getAverage(), 0));
        if (key == "MA1H") return (String(AQM::aqiReader.pm25_env_1hr.getAverage(), 0));
        if (key == "MA6H") return (String(AQM::aqiReader.pm25_env_6hr.getAverage(), 0));
        return "";
      };

      WebUI::startPage();
      templateHandler->send("/HomePage.html", mapper);
      WebUI::finishPage();
    }

    // Displays a form allowing the user to update the AQM settings.
    //
    // Form:
    //    GET  /displayAQMConfig
    //
    void displayAQMConfig() {
      Log.trace("Web Request: Display Config");
      if (!WebUI::authenticationOK()) { return; }

      auto mapper =[](String &key) -> String {
        if (key == "DESC") return WebThing::encodeAttr(AQM::settings.description);
        if (key == "BLYNK_KEY")  return AQM::settings.blynkAPIKey;
        return "";
      };

      WebUI::startPage();
      templateHandler->send("/ConfigForm.html", mapper);
      WebUI::finishPage();
    }
  }   // ----- END: AQMWebUI::Pages


  namespace Endpoints {
    // Handler for the "/updateAQMConfig" endpoint. This is invoked as the target
    // of the form presented by "/displayAQMConfig". It updates the values of the
    // corresponding settings and writes the settings to EEPROM.
    //
    // TO DO: Call a function to let the main app know that settings may have changed
    //        so that it can take any appropriate actions
    //
    // Form:
    //    GET /updateAQMConfig?description=DESC&useMetric=BOOL&...
    //
    void updateAQMConfig() {
      if (!WebUI::authenticationOK()) { return; }
      Log.trace("AQMWebUI: Handle Update Config");

      AQM::settings.description = WebUI::arg("description");
      AQM::settings.blynkAPIKey = WebUI::arg("blynkAPIKey");
      AQM::settings.write();

      // The description MAY have changed. Update the title just in case
      WebUI::setTitle(AQM::settings.description+" ("+WebThing::settings.hostname+")");

      WebUI::redirectHome();
    }
  }   // ----- END: AQMWebUI::Endpoints


  void init() {
    WebUI::setTitle(AQM::settings.description+" ("+WebThing::settings.hostname+")");
    WebUI::addMenuItems(Internal::Actions);

    WebUI::registerHandler("/", Pages::displayHomePage);
    WebUI::registerHandler("/displayAQMConfig", Pages::displayAQMConfig);

    WebUI::registerHandler("/updateAQMConfig", Endpoints::updateAQMConfig);

    templateHandler = WebUI::getTemplateHandler();
  }

}
// ----- END: AQMWebUI
