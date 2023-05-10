#include <gui/devices/DeviceSelect.h>
#if DEVICE_TYPE == DEVICE_TYPE_OLED

#ifndef ReadingScreen_h
#define ReadingScreen_h

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <Arduino.h>
//                                  Third Party Libraries
//                                  WebThing Includes
#include <gui/Screen.h>
//                                  Local Includes
//--------------- End:    Includes ---------------------------------------------

class ReadingScreen : public Screen {
public:
  ReadingScreen() { }

  virtual void display(bool force = false) = 0;

  void display(
      bool /*force*/, const char* heading,
      const char* fmt, float val, const char* units);

  virtual void processPeriodicActivity();

private:
  uint32_t timeOfLastDisplay;
};

class HumidityScreen : public ReadingScreen {
public:
  void display(bool force = false);
};

class TempScreen : public ReadingScreen {
public:
  void display(bool force = false);
};

class BaroScreen : public ReadingScreen {
public:
  void display(bool force = false);
};

#endif  // ReadingScreen_h
#endif