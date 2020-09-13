#ifndef Indicators_h
#define Indicators_h

#include <Adafruit_NeoPixel.h>



class NeoPixelIndicators {
public:
  Adafruit_NeoPixel *leds;

  NeoPixelIndicators() { }

  NeoPixelIndicators(uint8_t pin, uint8_t count) {
    _pin = pin;
    _count = count;
  }

  void begin() {
    leds = new Adafruit_NeoPixel(_count, _pin, NEO_RGB + NEO_KHZ800);
    leds->begin();
    leds->clear();
    leds->show();
  }

  void cycle() {
    for (int i = 0; i < _count; i++) {
      leds->setPixelColor(i, i==0?255:0, i==1?255:0, i==2?255:0);
      leds->show();
      delay(266);
    }
    for (int i = _count; i >= 0; i--) {
      leds->setPixelColor(i, 0, 0, 0);
      leds->show();
      delay(266);
    }
  }

  void setBrightness(uint8_t b) { leds->setBrightness(b); }

private:
  uint8_t _pin = 0;
  uint8_t _count = 0;;
};

class Indicator {
public:
  virtual void setColor(uint32_t c) { (void)c;  }
  virtual void setColor(uint8_t r, uint8_t g, uint8_t b) { (void)r; (void)g; (void)b;  }
  virtual void on() {  }
  virtual void off() {  }
};

class NeoPixelIndicator : public Indicator {
public:
  void begin(NeoPixelIndicators* parent, uint8_t which) {
    _parent = parent;
    _which = which;
  }

  virtual void setColor(uint32_t c) {
    _parent->leds->setPixelColor(_which, c);
    _parent->leds->show();
  }
  
  void setColor(uint8_t r, uint8_t g, uint8_t b) {
    _parent->leds->setPixelColor(_which, r, g, b);
    _parent->leds->show();
  }

  void on() { setColor(255, 255, 255); }
  void off() { setColor(0, 0, 0); }

private:
  NeoPixelIndicators* _parent;
  uint8_t _which;
};


#endif  // Indicators_h