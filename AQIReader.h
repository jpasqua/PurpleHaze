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
  MovingAverage pm25env_10min;
  MovingAverage pm25env_30min;
  MovingAverage pm25env_1hr;
  MovingAverage pm25env_6hr;

  // Historical data
  enum HistoryRange {Range_1Hour, Range_1Day, Range_1Week, Range_Combined};
  void emitHistoryAsJson(HistoryRange r, Stream& s);

  bool init(Stream* streamToSensor, Indicator* indicator);
  void process(time_t wallClock);

  void logData(AQIReadings& data);
  void logAvgs();


private:
  enum State {awake, retrying, waking, asleep};
  static const uint32_t ColorForState[];
  struct RecordedReadings {
    uint32_t timestamp;
    PMReadings env;
  };

  PMS5003* aqi;
  State state;
  uint32_t enteredStateAt;
  Indicator* _indicator;
  AQIReadings data;
  uint32_t last5minTimestamp = 0;
  uint32_t last1hrTimestamp = 0;
  uint32_t last1dayTimestamp = 0;

  CircularBuffer<RecordedReadings, 12> readings_5min; // The last hour's worth of readings at 5 minute intervals
  CircularBuffer<RecordedReadings, 24> readings_1hr;  // The last day's worth of readings at 1 hour intervals
  CircularBuffer<RecordedReadings, 28> readings_6hr;  // The last week's worth of readings at 6 hour intervals

  void enterState(State);
  void updateHistoricalData(AQIReadings& newSample);
  void storeHistoricalData(String historyFilePath);
  void loadHistoricalData(String historyFilePath);
  void updateMovingAvgs(uint16_t newSample);
  void computeAverages(AQIReadings& avg, AQIReadings* dataSet, uint16_t nReadings);
  uint16_t sizeOfRange(HistoryRange r);
  RecordedReadings getFromRange(HistoryRange r, uint16_t index);
};

#endif  // AQIReader_h