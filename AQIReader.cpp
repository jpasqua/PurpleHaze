#include <Arduino.h>
#include <FS.h>
#include <ArduinoLog.h>
#include <WebThing.h>
#include "AQIReader.h"

const uint32_t AQIReader::ColorForState[] = {
  0x00FF00, // awake
  0x8634EB, // awakeButPending
  0xFFA500, // waking
  0x000000  // asleep
};

static const String HistoryFilePath = "/history.json";
static const uint32_t MaxHistoryFileSize = 4096;

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
  loadHistoricalData(HistoryFilePath);

//#define MOCK_HISTORY
#ifdef MOCK_HISTORY
  AQIReadings data;
  data.pm10_standard = 26;
  data.pm25_standard = 42;
  data.pm100_standard = 50;
  data.pm10_env = 23;
  data.pm25_env = 37;
  data.pm100_env = 48;
  data.particles_03um = 4668;
  data.particles_05um = 1335;
  data.particles_10um = 282;
  data.particles_25um = 30;
  data.particles_50um = 8;
  data.particles_100um = 8;

  data.timestamp = 1599755402L - (1 * 7 * 24 * 65 * 60L);  // A week ago (+ 5 minutes)
  for (int i = 0; i < 7*24*6; i++) {
    updateHistoricalData(data);
    data.timestamp += 10 * 60L;
    data.pm10_env = max(data.pm10_env+random(-1, 2), 5);
    data.pm25_env = max(data.pm25_env+random(-1, 2), 5);
    data.pm100_env = max(data.pm100_env+random(-1, 2), 5);
  }
#endif  // MOCK_HISTORY

Serial.println("Waking up the Sensor");
  enterState(waking);
  return true;
}

void AQIReader::emitHistoryAsJson(HistoryRange r, Stream& s) {
  int32_t tzOffset = WebThing::getGMTOffset();
  uint16_t size = sizeOfRange(r);
  bool needsComma = false;
  s.println("{ \"data\": [");

  for (int i = 0; i < size; i++) {
    AQIReadings data = getFromRange(r, i);
    if (needsComma) s.println(",");
    s.print("{ \"ts\":"); s.print(data.timestamp - tzOffset);
    s.print(", \"pm10_env\":"); s.print(data.pm10_env);
    s.print(", \"pm25_env\":"); s.print(data.pm25_env);
    s.print(", \"pm100_env\":"); s.print(data.pm100_env);
    s.print(" }");
    needsComma = true;
  }
  s.println("] }");
  yield();
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

uint16_t AQIReader::sizeOfRange(HistoryRange r) {
  switch (r) {
    case Range_1Hour: return readings_10min.size();
    case Range_1Day: return readings_1hr.size();
    case Range_1Week: return readings_1day.size();
    case Range_Combined: return readings_10min.size() + readings_1hr.size() + readings_1day.size();
  }
  return 0; // Assert(CantHappen)
}

AQIReadings AQIReader::getFromRange(HistoryRange r, uint16_t index) {
  switch (r) {
    case Range_1Hour: return readings_10min[index];
    case Range_1Day: return readings_1hr[index];
    case Range_1Week: return readings_1day[index];
    case Range_Combined:
      if (index < readings_10min.size()) return readings_10min[index];
      index -= readings_10min.size();
      if (index < readings_1hr.size()) return readings_1hr[index];
      index -= readings_1hr.size();
      // Fall out...
  }
  return readings_1day[index];
}

void AQIReader::enterState(State newState) {
  state = newState;
  enteredStateAt = millis();
  _indicator->setColor(ColorForState[state]);
}

void AQIReader::updateHistoricalData(AQIReadings& newSample) {
  static int lastM = -1, lastH = -1, lastD = -1;
  bool needToWrite = false;

  int m = minute(newSample.timestamp)/10;   // 10 Minute intervals
  int h = hour(newSample.timestamp);
  int d = day(newSample.timestamp);

  if (m != lastM) { readings_10min.push(newSample); lastM = m; needToWrite = true; }
  if (h != lastH) { readings_1hr.push(newSample); lastH = h; needToWrite = true; }
  if ((d != lastD && h == 14) || (readings_1day.isEmpty() && h >= 14)) {
    // Take readings at 2PM. If we don't have any data yet and we're already
    // past 2PM, use the current reading to represent this day/
    readings_1day.push(newSample); lastD = d;
    needToWrite = true; 
  }
  if (needToWrite) storeHistoricalData(HistoryFilePath);
}

void AQIReader::loadHistoricalData(String historyFilePath) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations" 
  File historyFile = SPIFFS.open(historyFilePath, "r");
#pragma GCC diagnostic pop

  if (!historyFile) {
    Log.error(F("Failed to open history file for read: %s"), historyFilePath.c_str());
  } else {
    size_t size = historyFile.size();
    if (size > MaxHistoryFileSize) {
      Log.warning(F("History file is too big: %d"), size);
      return;
    }    
  }

  DynamicJsonDocument doc(MaxHistoryFileSize);
  auto error = deserializeJson(doc, historyFile);
  historyFile.close();
  if (error) {
    Log.warning(F("Failed to parse history file: %s"), error.c_str());
    return;
  }

// Log.verbose("Historical data:");
// serializeJsonPretty(doc, Serial);

  AQIReadings data;
  int32_t tzOffset = WebThing::getGMTOffset();
  // Note that timestamps are externalized as GMT time. When we
  // internalize them, we need to adjust back to local time
  JsonArrayConst hourData = doc[F("hour")]["data"];
  for (JsonObjectConst reading : hourData) {
    data.timestamp = reading["ts"];
Serial.print("tz: "); Serial.print(tzOffset); Serial.print(", dt: "); Serial.println(data.timestamp);
    data.timestamp += tzOffset;
 Serial.print("updated dt: "); Serial.println(data.timestamp);
    data.pm10_env = reading["pm10_env"];
    data.pm25_env = reading["pm25_env"];
    data.pm100_env = reading["pm100_env"];
    readings_10min.push(data);
  }
  JsonArrayConst dayData = doc[F("day")]["data"];
  for (JsonObjectConst reading : dayData) {
    data.timestamp = reading["ts"];
    data.timestamp += tzOffset;
    data.pm10_env = reading["pm10_env"];
    data.pm25_env = reading["pm25_env"];
    data.pm100_env = reading["pm100_env"];
    readings_1hr.push(data);
  }
  JsonArrayConst weekData = doc[F("week")]["data"];
  for (JsonObjectConst reading : weekData) {
    data.timestamp = reading["ts"];
    data.timestamp += tzOffset;
    data.pm10_env = reading["pm10_env"];
    data.pm25_env = reading["pm25_env"];
    data.pm100_env = reading["pm100_env"];
    readings_1day.push(data);
  }

Log.verbose("History as read from file:");
emitHistoryAsJson(Range_1Hour, Serial);
emitHistoryAsJson(Range_1Day, Serial);
emitHistoryAsJson(Range_1Week, Serial);
}

void AQIReader::storeHistoricalData(String historyFilePath) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations" 
  File historyFile = SPIFFS.open(historyFilePath, "w");
#pragma GCC diagnostic pop

  if (!historyFile) {
    Log.error(F("Failed to open history file for writing: %s"), historyFilePath.c_str());
    return;
  }

  historyFile.println("{\"hour\":"); emitHistoryAsJson(Range_1Hour, historyFile);
  historyFile.println(", \"day\":"); emitHistoryAsJson(Range_1Day, historyFile);
  historyFile.println(", \"week\":"); emitHistoryAsJson(Range_1Week, historyFile);
  historyFile.println("}");

  Log.verbose("History data written");
  historyFile.close();
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
  Log.verbose("-- %s --", WebThing::formattedInterval(
    hour(data.timestamp), minute(data.timestamp), second(data.timestamp), true, true).c_str());
  Log.verbose(F("---------------------------------------"));
  Log.verbose(F("Concentration Units (standard)"));
  Log.verbose(F("---------------------------------------"));
  Log.verbose(F("PM 1.0: %d\t\tPM 2.5: %d\t\tPM 10: %d"), data.pm10_standard ,data.pm25_standard, data.pm100_standard);
  Log.verbose(F("---------------------------------------"));
  Log.verbose(F("PM 1.0: %d\t\tPM 1.0: %d\t\tPM 10: %d"), data.pm10_env ,data.pm25_env, data.pm100_env);
  Log.verbose(F("---------------------------------------"));
  Log.verbose(F("Particles > 0.3um / 0.1L air:"), data.particles_03um);
  Log.verbose(F("Particles > 0.5um / 0.1L air:"), data.particles_05um);
  Log.verbose(F("Particles > 1.0um / 0.1L air:"), data.particles_10um);
  Log.verbose(F("Particles > 2.5um / 0.1L air:"), data.particles_25um);
  Log.verbose(F("Particles > 5.0um / 0.1L air:"), data.particles_50um);
  Log.verbose(F("Particles > 50 um / 0.1L air:"), data.particles_100um);
  Log.verbose(F("---------------------------------------"));
}

void AQIReader::logAvgs() {
  Log.verbose("-- %s --", WebThing::formattedInterval(
    hour(data.timestamp), minute(data.timestamp), second(data.timestamp), true, true).c_str());
  Log.verbose(F("Last 30 Minutes: %F"), pm25_env_30min.getAverage());
  Log.verbose(F("Last 30 Minutes: %F"), pm25_env_1hr.getAverage());
  Log.verbose(F("Last 30 Minutes: %F"), pm25_env_6hr.getAverage());
  Log.verbose(F("Last 30 Minutes: %F"), pm25_env_1day.getAverage());
  Log.verbose(F("Last 30 Minutes: %F"), pm25_env_1week.getAverage());
}

void AQIReader::logHistory() {
  // TO DO: Implement Me!!
}
