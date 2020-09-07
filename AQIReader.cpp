#include <ArduinoLog.h>
#include "AQIReader.h"

const uint32_t AQIReader::ColorForState[] = {
  0x00FF00, // awake
  0x8634EB, // awakeButPending
  0xFFA500, // waking
  0x000000  // asleep
};

AQIReader::AQIReader() {
  aqi = new PMS5003();
  pm25_env_10min.setSamplesToConsider(2);         // (2) 5-minute periods
  pm25_env_30min.setSamplesToConsider(6);         // (6) 5-minute periods
  pm25_env_1hr.setSamplesToConsider(12);          // (12) 5-minute periods
  pm25_env_6hr.setSamplesToConsider(6 * 12);      // (6) 1-hour periods
  pm25_env_1day.setSamplesToConsider(4 * 72);     // (4) 6-hour periods
  pm25_env_1week.setSamplesToConsider(7 * 288);   // (7) 1-day periods
}

bool AQIReader::init(Stream* streamToSensor, Indicator* indicator) {
  _indicator = indicator;
  if (!aqi->begin(streamToSensor)) {
    _indicator->setColor(255, 0, 0);
    return false;
  }

Serial.println("Waking up the Sensor");
  enterState(waking);
  return true;
}

void AQIReader::process(time_t wallClock) {
  uint32_t elapsed = millis() - enteredStateAt;

  switch (state) {
    case asleep:
      if (elapsed > (4*60+30)*1000L) {
        aqi->wakeUp();
        enterState(waking);
        Log.verbose("Hey, wake up!");
      }
      return;
    case waking:
      if (elapsed > 30*1000L) {
        enterState(awake);
        Log.verbose("Ok, OK, I'm awake");
      }
      return;
    case awakeButPending:
      if (elapsed < 500L) return;
      enterState(awake);
      // Log.verbose("OK, let's try again...");
      // NOTE: No break or return. Fall through!!!
    case awake:
      // Log.verbose("About to read");
      break;
  }
  
  if (aqi->read(&data)) {
    data.timestamp = wallClock;
    logData(data);
  } else {
    enterState(awakeButPending);
Serial.print('.');
    return;
  }

  updateMovingAvgs(data.pm25_env);
  updateHistoricalData(data);
  logAvgs();

  aqi->sleep();
  enterState(asleep);
  Log.verbose("\nGood night...");
}

void AQIReader::enterState(State newState) {
  state = newState;
  enteredStateAt = millis();
  _indicator->setColor(ColorForState[state]);
}

void AQIReader::updateHistoricalData(AQIReadings& newSample) {
  static int lastM = -1, lastH = -1, lastD = -1;
  int m = minute(newSample.timestamp)/10;   // 10 Minute intervals
  int h = hour(newSample.timestamp);
  int d = day(newSample.timestamp);

  if (m != lastM) { readings_10min.push(newSample); lastM = m; }
  if (h != lastH) { readings_1hr.push(newSample); lastH = h; }
  if (d != lastD && h == 14) { readings_1day.push(newSample); lastD = d; } // Take readings at 2PM
}

void AQIReader::updateMovingAvgs(uint16_t newSample) {
  pm25_env_10min.addSample(newSample);
  pm25_env_30min.addSample(newSample);
  pm25_env_1hr.addSample(newSample);
  pm25_env_6hr.addSample(newSample);
  pm25_env_1day.addSample(newSample);
  pm25_env_1week.addSample(newSample);
}


void AQIReader::logData(AQIReadings& data) {
  Serial.println(F("---------------------------------------"));
  Serial.println(F("Concentration Units (standard)"));
  Serial.println(F("---------------------------------------"));
  Serial.print(F("PM 1.0: ")); Serial.print(data.pm10_standard);
  Serial.print(F("\t\tPM 2.5: ")); Serial.print(data.pm25_standard);
  Serial.print(F("\t\tPM 10: ")); Serial.println(data.pm100_standard);
  Serial.println(F("Concentration Units (environmental)"));
  Serial.println(F("---------------------------------------"));
  Serial.print(F("PM 1.0: ")); Serial.print(data.pm10_env);
  Serial.print(F("\t\tPM 2.5: ")); Serial.print(data.pm25_env);
  Serial.print(F("\t\tPM 10: ")); Serial.println(data.pm100_env);
  Serial.println(F("---------------------------------------"));
  Serial.print(F("Particles > 0.3um / 0.1L air:")); Serial.println(data.particles_03um);
  Serial.print(F("Particles > 0.5um / 0.1L air:")); Serial.println(data.particles_05um);
  Serial.print(F("Particles > 1.0um / 0.1L air:")); Serial.println(data.particles_10um);
  Serial.print(F("Particles > 2.5um / 0.1L air:")); Serial.println(data.particles_25um);
  Serial.print(F("Particles > 5.0um / 0.1L air:")); Serial.println(data.particles_50um);
  Serial.print(F("Particles > 50 um / 0.1L air:")); Serial.println(data.particles_100um);
  Serial.println(F("---------------------------------------"));
}

void AQIReader::logAvgs() {
  Serial.print("Last 10 Minutes: "); Serial.println(pm25_env_10min.getAverage());
  Serial.print("Last 30 Minutes: "); Serial.println(pm25_env_30min.getAverage());
  Serial.print("Last 60 Minutes: "); Serial.println(pm25_env_1hr.getAverage());
  Serial.print("Last 6 Hours: "); Serial.println(pm25_env_6hr.getAverage());
  Serial.print("Last 1 Day: "); Serial.println(pm25_env_1day.getAverage());
  Serial.print("Last 7 Days: "); Serial.println(pm25_env_1week.getAverage());
}

void AQIReader::logHistory() {
  // TO DO: Implement Me!!
}
