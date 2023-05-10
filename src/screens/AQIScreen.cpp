#include <gui/devices/DeviceSelect.h>
#if DEVICE_TYPE == DEVICE_TYPE_OLED

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <Arduino.h>
//                                  Third Party Libraries
#include <TimeLib.h>
//                                  WebThingApp
#include <gui/Display.h>
#include <gui/Theme.h>
//                                  Local Includes
#include "../../PurpleHazeApp.h"
#include "AQIScreen.h"
#include "AQIIcons.h"
//--------------- End:    Includes ---------------------------------------------


void AQIScreen::display(bool) {
  const AQIReadings& aqiReadings = phApp->aqiMgr.getLastReadings();
  auto aqi = phApp->aqiMgr.derivedAQI(aqiReadings.env.pm25);
  Display.oled->clear();


  auto font = Display.FontID::D32;
  int offset = 0;
  int yOffset = 0;

  if (aqi > 99 && aqi < 200) {
    // We can fit 3 digits of the D32 font if the leading digit is a 1.
    // The image will logically overlap it, but it won't be visible.
    offset = 5;
  } else if (aqi > 199) {
    font = Display.FontID::D16;
    yOffset = 12;
  }

  uint16_t rightColumnCenter = Display.XCenter + Display.XCenter/2 - offset;
  Display.setFont(font);
  Display.oled->setTextAlignment(TEXT_ALIGN_CENTER);
  Display.oled->drawString(rightColumnCenter, 5 + yOffset, String(aqi));

  Display.setFont(Display.FontID::SB12);
  Display.oled->setTextAlignment(TEXT_ALIGN_CENTER);
  Display.oled->drawString(rightColumnCenter, 49, "AQI");

  const uint8_t* aqiIcon = AQILevels[phApp->aqiMgr.aqiBracket(aqi)];
  Display.oled->drawXbm(0, 0, AQI_ICON_WIDTH, AQI_ICON_HEIGHT, aqiIcon);

  Display.oled->display();
  timeOfLastDisplay = millis();
}

void AQIScreen::processPeriodicActivity() {
  // Every minute check to see if there is a new AQI reading. If so, display it.
  static uint32_t lastMinute = 0;
  uint32_t thisMinute = minute();
  if (thisMinute != lastMinute) {
    lastMinute = thisMinute;
    uint32_t mostRecentReadingTime;
    mostRecentReadingTime = phApp->aqiMgr.getLastReadings().timestamp;
    if (mostRecentReadingTime > timeOfLastDisplay) display(true);
  }
}


#endif