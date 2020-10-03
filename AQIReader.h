/*
 * AQIReader
 *    Read data realited to the Air Quality Index from an underlying device and
 *    store historical information and averages.
 *
 */

#ifndef AQIReader_h
#define AQIReader_h

//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
//                                  Third Party Libraries
#include <CircularBuffer.h>
#include <TimeLib.h>
//                                  Local Includes
#include "PMS5003.h"
#include "MovingAverage.h"
#include "Indicators.h"
//--------------- End:    Includes ---------------------------------------------


class AQIReader {
public:
  AQIReader();
  AQIReadings getLastReadings() { return data; }
  uint16_t derivedAQI(uint16_t reading);

  // Moving averages
  MovingAverage pm25_env_10min;
  MovingAverage pm25_env_30min;
  MovingAverage pm25_env_1hr;
  MovingAverage pm25_env_6hr;
  MovingAverage pm25_env_1day;
  MovingAverage pm25_env_1week;

  // Historical data
  enum HistoryRange {Range_1Hour, Range_1Day, Range_1Week, Range_Combined};
  void emitHistoryAsJson(HistoryRange r, Stream& s);
  uint16_t sizeOfRange(HistoryRange r);
  AQIReadings getFromRange(HistoryRange r, uint16_t index);

  bool init(Stream* streamToSensor, Indicator* indicator);
  void process(time_t wallClock);

  void logData(AQIReadings& data);
  void logAvgs();


private:
  enum State {awake, retrying, waking, asleep};
  static const uint32_t ColorForState[];

  PMS5003* aqi;
  State state;
  uint32_t enteredStateAt;
  Indicator* _indicator;
  AQIReadings data;

  CircularBuffer<AQIReadings, 60/10> readings_10min; // The last hour's worth of readings at 10 minute intervals
  CircularBuffer<AQIReadings, 24> readings_1hr; // The last day's worth of readings at 1 hour intervals
  CircularBuffer<AQIReadings, 7> readings_1day; // The last week's worth of readings at 1 day intervals

  void enterState(State);
  void updateHistoricalData(AQIReadings& newSample);
  void storeHistoricalData(String historyFilePath);
  void loadHistoricalData(String historyFilePath);
  void updateMovingAvgs(uint16_t newSample);
  void computeAverages(AQIReadings& avg, AQIReadings* dataSet, uint16_t nReadings);
};

#endif  // AQIReader_h