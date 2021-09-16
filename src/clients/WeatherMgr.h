
/* WeatherMgr
 *     Provide a uniform interface to one or more weather sensing devices
 *     such as a BME280, a DHT22, and/or a DS18B20
 *
 * NOTES:
 * o At the moment we only manage a BME280. The itnerface will need to be generalized
 *   to support other devices.
 * o There may be times when multiple devices can take the same reading. For example,
 *   you may have a BME280 and a DS18B20, and use the Humidity and pressure values
 *   from the BME280, but prefer the temp value from the DS18B20.
 *
 */

#ifndef WeatherMgr_h
#define WeatherMgr_h

#include <WebThing.h>
#include <WTBasics.h>
#include "WeatherReadings.h"
#include "BMESensor.h"

class WeatherMgr {
public:
  // ----- Types

  // ----- Constants -----
  static constexpr uint32_t TenMinutesInMillis = 10 * 60 * 60 * 1000L;
  static constexpr const char* WeatherHistoryFilePath = "/wthrHist.json";

  // ----- Constructors & Destructor -----
  WeatherMgr() = default;

  // ----- Methods -----
  void begin(
      int addr=0x76,
      uint32_t readingInterval = TenMinutesInMillis,
      uint32_t historyInterval = TenMinutesInMillis)
  {
    _bme.begin(addr);
    _readingInterval = readingInterval;
    _historyInterval = historyInterval;
    if (_historyInterval > _historyWriteInterval) {
      _historyWriteInterval = historyInterval;
    }

    samples.load(WeatherHistoryFilePath);
  }

  // --- Getting and using sensor readings ---
  // Returns the timestamp of the last reading
  inline uint32_t lastReadingTime() { return lastReadings.timestamp; }

  // Returns the last set of values read from the sensor. This may be the same
  // as the answer as given on a previous call if the sesnor has not read new
  // values  in the interval.
  const WeatherReadings& getLastReadings() { return lastReadings; }


  void takeReadings(bool force = false) {
    static uint32_t _nextReading = 0;
    static uint32_t _nextHistorySample = 0;
    static uint32_t _nextHistoryWrite = 0;

    uint32_t curMillis = millis();
    if (force || curMillis > _nextReading) {
      _bme.takeReadings(lastReadings);
      lastReadings.timestamp = curMillis;
      _nextReading = curMillis + _readingInterval;

      if (curMillis > _nextHistorySample) {
        time_t gmtTimestamp = WTBasics::wallClockFromMillis(lastReadings.timestamp) + WebThing::getGMTOffset();
        samples.push(TempReading(lastReadings.temp, gmtTimestamp));
        _nextHistorySample = curMillis + _historyInterval;
        if (curMillis > _nextHistoryWrite) {
          samples.store(WeatherHistoryFilePath);
          _nextHistoryWrite = curMillis + _historyWriteInterval;
        }
      }
    }
  }

  void setAttributes(float tempCorrection, float humidityCorrection, int32_t elevation) {
    _bme.setAttributes(tempCorrection, humidityCorrection, elevation);
  }


private:
  // ----- Types -----
  class TempReading : public Serializable {
  public:
    TempReading() = default;
    TempReading(float t, uint32_t ts) : temp(t), timestamp(ts) { }

    virtual void externalize(Stream& writeStream) const {
      StaticJsonDocument<32> doc; // Size provided by https://arduinojson.org/v6/assistant
      doc["timestamp"] = timestamp;
      doc["temp"] = temp;
      serializeJson(doc, writeStream);
    }

    void internalize(const JsonObjectConst &obj) {
      timestamp = obj["timestamp"];
      temp = obj["temp"];
    }

    float temp;
    time_t timestamp; // GMT
  };


  // ----- Data Members -----
  BMESensor _bme;
  WeatherReadings lastReadings;

  uint32_t _readingInterval = TenMinutesInMillis;       // How often do we take a reading
  uint32_t _historyInterval = TenMinutesInMillis;       // How often do we store a sample
  uint32_t _historyWriteInterval = TenMinutesInMillis;  // How often do we write the samples to a file

  static constexpr size_t MaxSamples = 128;
  HistoryBuffer<TempReading, MaxSamples> samples;
};
#endif // WeatherMgr_h