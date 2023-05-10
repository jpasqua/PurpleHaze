#include <gui/devices/DeviceSelect.h>
#if DEVICE_TYPE == DEVICE_TYPE_OLED

#ifndef AQIScreen_h
#define AQIScreen_h

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <Arduino.h>
//                                  Third Party Libraries
//                                  Local Includes
#include <gui/Display.h>
//--------------- End:    Includes ---------------------------------------------


class AQIScreen : public Screen {
public:
  virtual void display(bool) override;
  virtual void processPeriodicActivity() override;

private:
  uint32_t timeOfLastDisplay;
};

#endif  // AQIScreen_h
#endif