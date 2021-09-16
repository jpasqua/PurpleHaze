#ifndef WeatherBlynkPublisher_h
#define WeatherBlynkPublisher_h

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <ArduinoLog.h>
//                                  Third Party Libraries
//                                  Local Includes
#include "../utils/Output.h"
#include "BlynkMgr.h"
//--------------- End:    Includes ---------------------------------------------


class WeatherBlynkPublisher : public BlynkPublisher {
public:

  WeatherBlynkPublisher(WeatherMgr* weatherMgr)
      : _weatherMgr(weatherMgr) { }

  bool publish() override {
    const WeatherReadings& readings = _weatherMgr->getLastReadings();
    if (readings.timestamp == _timestampOfLastData) return false;

    if (!isnan(readings.temp)) {
      BlynkMgr::writeFloat(TempPin, Output::temp(readings.temp));
      BlynkMgr::writeFloat(HeatIndexPin, Output::temp(readings.heatIndex));
      BlynkMgr::writeFloat(DewPointPin, Output::temp(readings.dewPointTemp));
      BlynkMgr::writeFloat(DewSpreadPin, Output::tempSpread(readings.dewPointSpread));
    }

    if (!isnan(readings.humidity)) {
      BlynkMgr::writeFloat(HumidityPin, readings.humidity);
    }

    if (!isnan(readings.pressure)) {
      BlynkMgr::writeFloat(PressurePin,    Output::baro(readings.pressure));
      BlynkMgr::writeSigned(RelPressurePin, Output::baro(readings.relPressure));
    }

    if (timeStatus() == timeSet) {
      String dateTime = Output::dateTime(WTBasics::wallClockFromMillis(readings.timestamp));
      Log.verbose("Timestamp sent to Blynk: %s", dateTime.c_str());
      BlynkMgr::writeString(TimestampPin, dateTime);
    }

    _timestampOfLastData = readings.timestamp;
    return true;
  }

  BlynkPinRange getPinRange() override {
    return {TempPin, TimestampPin};
  }


private:
  static constexpr uint8_t TempPin         = 0;
  static constexpr uint8_t HumidityPin     = 1;
  static constexpr uint8_t PressurePin     = 2;
  static constexpr uint8_t RelPressurePin  = 3;
  static constexpr uint8_t DewPointPin     = 4;
  static constexpr uint8_t DewSpreadPin    = 5;
  static constexpr uint8_t HeatIndexPin    = 6;
  static constexpr uint8_t TimestampPin    = 8;

  WeatherMgr* _weatherMgr;
  uint32_t _timestampOfLastData = 0;
};



#endif	// WeatherBlynkPublisher_h