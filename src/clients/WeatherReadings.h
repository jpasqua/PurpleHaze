#ifndef WeatherReadings_h
#define WeatherReadings_h

class WeatherReadings {
public:
  uint32_t  timestamp;        // When was the reading taken (millis())

  float     temp;             // If isnan(), then no value is available
  float     humidity;         // If isnan(), then no value is available
  float     pressure;         // If isnan(), then no value is available

  // The following values are derived from the sensed values rather than
  // being sensed independently.
  float     heatIndex;
  int       relPressure;
  float     dewPointSpread;
  double    dewPointTemp;
};

#endif // WeatherReadings_h