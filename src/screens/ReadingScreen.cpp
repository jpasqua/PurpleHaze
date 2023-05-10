//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <Arduino.h>
//                                  Third Party Libraries
#include <Output.h>
//                                  WebThing Includes
#include <gui/Display.h>
//                                  Local Includes
#include "../../PurpleHazeApp.h"
#include "ReadingScreen.h"
//--------------- End:    Includes ---------------------------------------------

void ReadingScreen::display(
    bool, const char* heading,
    const char* fmt, float val, const char* units) {
  auto oled = Display.oled;
  
  oled->clear();
  oled->setTextAlignment(TEXT_ALIGN_CENTER);
  Display.setFont(Display.FontID::S10);
  oled->drawString(Display.XCenter, 0, heading);

  constexpr auto FmtBufSize = 32;
  char fmtBuf[FmtBufSize];

  snprintf(fmtBuf, FmtBufSize, fmt, val, units);
  Display.setFont(Display.FontID::D32);
  oled->setTextAlignment(TEXT_ALIGN_CENTER);
  oled->drawString(Display.XCenter, 20, fmtBuf);
  uint16_t w = oled->getStringWidth(fmtBuf);
  Display.setFont(Display.FontID::S16);
  oled->drawString(Display.XCenter+w/2+6, 40, units);
  oled->display();

  timeOfLastDisplay = millis();
}

void ReadingScreen::processPeriodicActivity() {
  // Every minute check to see if there are new weather readings. If so, display them.
  static uint32_t lastMinute = 0;
  uint32_t thisMinute = minute();
  if (thisMinute != lastMinute) {
    lastMinute = thisMinute;
    if (phApp->weatherMgr.getLastReadings().timestamp > timeOfLastDisplay) display(true);
  }
}


void HumidityScreen::display(bool force) {
  float humidity = phApp->weatherMgr.hasHumi() ?
      phApp->weatherMgr.getLastReadings().humidity :
      phApp->owmClient->weather.readings.humidity;
  ReadingScreen::display(force, "Humidity", "%.0f", humidity, "%");
}

void TempScreen::display(bool force) {
  float temp = phApp->weatherMgr.hasTemp() ?
      Output::temp(phApp->weatherMgr.getLastReadings().temp) :
      phApp->owmClient->weather.readings.temp;
  ReadingScreen::display(force, "Temperature", "%.0f", temp, Output::tempUnits());
}

void BaroScreen::display(bool force) {
  constexpr auto FmtBufSize = 32;
  char fmtBuf[FmtBufSize];
  snprintf(fmtBuf, FmtBufSize, "Barometer (%s)", Output::baroUnits());

  float baro = phApp->weatherMgr.hasBaro() ?
      Output::baro(phApp->weatherMgr.getLastReadings().pressure) :
      phApp->owmClient->weather.readings.pressure;
  ReadingScreen::display(force, fmtBuf, "%.1f", baro, "");
}
