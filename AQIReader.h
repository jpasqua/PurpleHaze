#ifndef AQIReader_h
#define AQIReader_h


#include <CircularBuffer.h>
#include <TimeLib.h>
#include "PMS5003.h"
#include "MovingAverage.h"
#include "Indicators.h"

class AQIReader {
public:
  enum State {awake, awakeButPending, waking, asleep};

  AQIReader();
  State getState() { return state; }
  AQIReadings getLastReadings() { return data; }
  
  // Moving averages
  MovingAverage pm25_env_10min;
  MovingAverage pm25_env_30min;
  MovingAverage pm25_env_1hr;
  MovingAverage pm25_env_6hr;
  MovingAverage pm25_env_1day;
  MovingAverage pm25_env_1week;

  // Historical data
  CircularBuffer<AQIReadings, 60/10> readings_10min;
    // The last hour's worth of readings at 10 minute intervals
  CircularBuffer<AQIReadings, 24> readings_1hr;
    // The last day's worth of readings at 1 hour intervals
  CircularBuffer<AQIReadings, 7> readings_1day;
    // The last week's worth of readings at 1 day intervals

  bool init(Stream* streamToSensor, Indicator* indicator);
  void process(time_t wallClock);
  void logData(AQIReadings& data);
  void logAvgs();
  void logHistory();


private:
  static const uint32_t ColorForState[];

  PMS5003* aqi;
  State state;
  uint32_t enteredStateAt;
  Indicator* _indicator;
  AQIReadings data;

  void enterState(State);
  void updateHistoricalData(AQIReadings& newSample);
  void updateMovingAvgs(uint16_t newSample);
  void computeAverages(AQIReadings& avg, AQIReadings* dataSet, uint16_t nReadings);
};

#endif  // AQIReader_h