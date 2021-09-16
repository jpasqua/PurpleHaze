#ifndef AQIReadings_h
#define AQIReadings_h

struct ParticleReadings {
  uint16_t pm10;
  uint16_t pm25;
  uint16_t pm100;
};

typedef struct {
  uint32_t timestamp;
  // The following field sizes and order are defined to match the raw
  // output from the sensor. Do not reorganize them!
  ParticleReadings standard;
  ParticleReadings env;
  uint16_t particles_03um;  // 0.3um Particle Count
  uint16_t particles_05um;  // 0.5um Particle Count
  uint16_t particles_10um;  // 1.0um Particle Count
  uint16_t particles_25um;  // 2.5um Particle Count
  uint16_t particles_50um;  // 5.0um Particle Count
  uint16_t particles_100um; // 10.0um Particle Count
} AQIReadings;

#endif  // AQIReadings_h
