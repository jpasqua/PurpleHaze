//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <Arduino.h>
//                                  Third Party Libraries
#include <Output.h>
#include <TimeLib.h>
//                                  WebThingApp
#include <gui/Display.h>
#include <gui/Theme.h>
//                                  Local Includes
#include "../../PurpleHazeApp.h"
//--------------- End:    Includes ---------------------------------------------


static inline uint16_t compose(int h, int m) { return(h * 100 + m); }

static constexpr auto Reading_Font = Display.FontID::S10;
static constexpr uint16_t Reading_FontHeight = 13;
static constexpr uint16_t Reading_XOrigin = 0;
static constexpr uint16_t Reading_YOrigin = 51;
static constexpr uint16_t Reading_Width = 43;
static constexpr uint16_t Reading_Height = Reading_FontHeight;
static constexpr uint16_t Reading_BorderSize = 1;

static constexpr auto Label_Font = Display.FontID::S10;
static constexpr uint16_t Label_FontHeight = 13;


HomeScreen::HomeScreen() { }

void HomeScreen::display(bool) {
  auto oled = Display.oled;
  
  oled->clear();

  bool use24Hour = phSettings->uiOptions.use24Hour;
  time_t curTime = now();
  int  m = minute(curTime);
  int  h = hour(curTime);
  compositeTime = compose(h, m);

  if (!use24Hour) {
    if (h > 12) { h -= 12; }
    else if (h == 0) { h = 12;}
  }

  constexpr uint8_t bufSize = 32;
  char buf[bufSize];
  snprintf(buf, bufSize, "%d:%02d", h, m);
  Display.setFont(Display.FontID::D32);
  if (use24Hour) {
    oled->setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
    oled->drawString(Display.XCenter, Display.getFontHeight(Display.FontID::D32)/2 + 2, buf);
  } else {
    oled->setTextAlignment(TEXT_ALIGN_RIGHT);
    oled->drawString(Display.Width-24, 2, buf);
    oled->setTextAlignment(TEXT_ALIGN_LEFT);
    Display.setFont(Display.FontID::S10);
    oled->drawString(Display.Width-20, 4, isAM(curTime) ? "AM" : "PM");
  }

  drawLabels();
  drawReadings();

  oled->display();
}

void HomeScreen::processPeriodicActivity() {
  if (compositeTime != compose(hour(), minute())) display(true);
  uint32_t mostRecentReadingTime;
  #if defined(HAS_AQI_SENSOR) && defined(HAS_WEATHER_SENSOR)
    mostRecentReadingTime = max(phApp->weatherMgr.getLastReadings().timestamp,
                                phApp->aqiMgr.getLastReadings().timestamp);
  #elif defined(HAS_AQI_SENSOR)
    mostRecentReadingTime = phApp->aqiMgr.getLastReadings().timestamp;
  #else
    mostRecentReadingTime = phApp->weatherMgr.getLastReadings().timestamp;
  #endif
  if (mostRecentReadingTime > lastReadingTime) display(true);
}

// ----- Private Functions

void HomeScreen::drawLabels() {
  Display.oled->setTextAlignment(TEXT_ALIGN_CENTER);
  Display.oled->setColor(Theme::Color_NormalText);
  Display.setFont(Label_Font);

  uint16_t xPos = 1;
  uint16_t yPos = Reading_YOrigin - Label_FontHeight + 1; // Add an extra pixel for padding
  #if defined(HAS_AQI_SENSOR)
    // AQI    TEMP        HUMI
    Display.oled->drawString(xPos + Reading_Width/2, yPos, "aqi");
    xPos += Reading_Width;
    Display.oled->drawString(xPos + Reading_Width/2, yPos, "temp");
    xPos += Reading_Width;
    Display.oled->drawString(xPos + Reading_Width/2, yPos, "humi");
  #else
    // TEMP    HUMI       BARO
    Display.oled->drawString(xPos + Reading_Width/2, yPos, "temp");
    xPos += Reading_Width;
    Display.oled->drawString(xPos + Reading_Width/2, yPos, "humi");
    xPos += Reading_Width;
    Display.oled->drawString(xPos + Reading_Width/2, yPos, "baro");
  #endif
}

void HomeScreen::drawReadings() {

  String readings[3];
  #if defined(HAS_AQI_SENSOR) && defined(HAS_WEATHER_SENSOR)
    // AQI    TEMP        HUMI
    readings[0] = String(phApp->aqiMgr.derivedAQI(phApp->aqiMgr.getLastReadings().env.pm25));
    readings[1] = String(Output::temp(phApp->weatherMgr.getLastReadings().temp), 0);
    readings[2] = String(phApp->weatherMgr.getLastReadings().humidity, 0);
    lastReadingTime = max(
      phApp->weatherMgr.getLastReadings().timestamp,
      phApp->aqiMgr.getLastReadings().timestamp);
  #elif defined(HAS_AQI_SENSOR)
    // AQI    OWM_TEMP    OWM_HUMI
    readings[0] = String(phApp->aqiMgr.derivedAQI(phApp->aqiMgr.getLastReadings().env.pm25));
    readings[1] = String(phApp->owmClient->weather.readings.temp, 0);
    readings[2] = String(phApp->owmClient->weather.readings.humidity, 0);
    lastReadingTime = phApp->aqiMgr.getLastReadings().timestamp;
  #elif defined(HAS_WEATHER_SENSOR)
    // TEMP    HUMI       BARO
    auto wReadings = phApp->weatherMgr.getLastReadings();
    readings[0] = String(Output::temp(wReadings.temp), 0);
    readings[1] = String(wReadings.humidity, 0);
    readings[2] = String(Output::baro(wReadings.pressure), 1);
    lastReadingTime = phApp->weatherMgr.getLastReadings().timestamp;
  #else
    readings[0] = String("N/A");
    readings[1] = String(phApp->owmClient->weather.readings.temp, 0);
    readings[2] = String(phApp->owmClient->weather.readings.humidity, 0);
  #endif


  Label l;
  l.init(1, Reading_YOrigin, Reading_Width, Reading_Height, 0);
  for (int i = 0; i < 3; i++) {
    l.drawSimple(readings[i], Reading_Font, Reading_BorderSize, WHITE, WHITE, BLACK);
    l.region.x += Reading_Width-1;
  }
}
