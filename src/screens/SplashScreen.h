#include <gui/devices/DeviceSelect.h>
#if DEVICE_TYPE == DEVICE_TYPE_OLED

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <Arduino.h>
//                                  Third Party Libraries
//                                  Local Includes
#include <gui/Display.h>
#include "PHSplashBitmap.h"
//--------------- End:    Includes ---------------------------------------------


class SplashScreen : public Screen {
public:
  virtual void display(bool) override {
    Display.oled->clear();
    Display.oled->drawXbm(0, 0, Display.Width, Display.Height, PHSplashBitmap);
    Display.oled->display();
  }
  virtual void processPeriodicActivity() override { }

protected:
};

#endif