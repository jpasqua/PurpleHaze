/*
 * AQIReader
 *    Read data realited to the Air Quality Index from an underlying device and
 *    store historical information and averages.
 *
 */


//--------------- Begin:  Includes ---------------------------------------------
//                                  Core Libraries
#include <Arduino.h>
#include <FS.h>
//                                  Third Party Libraries
#include <ArduinoLog.h>
#include <WebThing.h>
//                                  Local Includes
#include "AQIReader.h"
//--------------- End:    Includes ---------------------------------------------


const uint32_t AQIReader::ColorForState[] = {
  0x00FF00, // awake
  0x8634EB, // retrying
  0xFFA500, // waking
  0x000000  // asleep
};

const struct {
  float pMin;
  float pRange;
  uint16_t aqMin;
  uint16_t aqRange;
} AQITable[] = {
      {  0.0,  15.4,   0, 50},
      { 15.5,  24.9,  51, 49},
      { 40.5,  24.9, 101, 49},
      { 65.5,  84.9, 151, 49},
      {150.5,  99.9, 201, 99},
      {250.5, 249.9, 301, 199}
};
static const int AQITableLength= sizeof(AQITable)/sizeof(AQITable[0]);

static const String HistoryFilePath = "/history.json";
static const uint32_t MaxHistoryFileSize = 8192;

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

  enterState(waking);
  return true;
}

uint16_t AQIReader::derivedAQI(uint16_t reading) {
  int i;
  for (i = 0; i < AQITableLength; i++) {
    if (reading < AQITable[i].pMin) break;
  }
  i--;
  float aqi = ((reading -  AQITable[i].pMin)*(AQITable[i].aqRange))/AQITable[i].pRange + AQITable[i].aqMin;
  return (uint16_t)aqi;
}


void AQIReader::emitHistoryAsJson(HistoryRange r, Stream& s) {
  if (r == Range_Combined) {
    s.print("{ \"hour\":"); emitHistoryAsJson(Range_1Hour, s); s.println(",");
    s.print("  \"day\":");  emitHistoryAsJson(Range_1Day, s);  s.println(",");
    s.print("  \"week\":"); emitHistoryAsJson(Range_1Week, s); s.println("}");
    return;
  }

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
      if (elapsed > (4*60+30)*1000L) { enterState(waking); }
      return;
    case waking:
      if (elapsed > 30*1000L) {
        enterState(awake);
        Log.verbose("Ok, OK, I'm awake");
      }
      return;
    case retrying:
      if (elapsed < 500L) return;
      enterState(awake);
      // NOTE: No break or return. Fall through!!!
    case awake:
      // Log.verbose("About to read");
      break;
  }
  
  if (aqi->read(&data)) {
    data.timestamp = wallClock;
    logData(data);
  } else {
    enterState(retrying);
Serial.print('.');
    return;
  }

  updateMovingAvgs(data.pm25_env);
  updateHistoricalData(data);
  logAvgs();

  enterState(asleep);
}

uint16_t AQIReader::sizeOfRange(HistoryRange r) {
  switch (r) {
    case Range_1Hour: return readings_5min.size();
    case Range_1Day: return readings_1hr.size();
    case Range_1Week: return readings_1day.size();
    case Range_Combined: return readings_5min.size() + readings_1hr.size() + readings_1day.size();
  }
  return 0; // Assert(CantHappen)
}

AQIReadings AQIReader::getFromRange(HistoryRange r, uint16_t index) {
  switch (r) {
    case Range_1Hour: return readings_5min[index];
    case Range_1Day: return readings_1hr[index];
    case Range_1Week: return readings_1day[index];
    case Range_Combined:
      if (index < readings_5min.size()) return readings_5min[index];
      index -= readings_5min.size();
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
  if (state == waking) {
    Log.verbose("Hey, wake up!");
    aqi->wakeUp();
  } else if (state == asleep) {
    Log.verbose("\nGood night...");
    aqi->sleep();
  }
}

void AQIReader::updateHistoricalData(AQIReadings& newSample) {
  // We don't necessarilly want to write the historical data to Flash on every
  // reading. The following variables control how frequently that occurs
  static const uint32_t WriteThreshold = 10 * 60 * 1000L; // Write every 10 Minutes
  static uint32_t lastWrite = 0;

  bool storedNewValue = false;

  if (newSample.timestamp - last5minTimestamp >= (5 * 60L)) {
    readings_5min.push(newSample);
    last5minTimestamp = newSample.timestamp;
    storedNewValue = true;
  }
  if (newSample.timestamp - last1hrTimestamp >= (60 * 60L)) {
    readings_1hr.push(newSample);
    last1hrTimestamp = newSample.timestamp;
    storedNewValue = true;
  }
  if (newSample.timestamp - last1dayTimestamp >= (24 * 60 * 60L)) {
    readings_1day.push(newSample);
    last1dayTimestamp = newSample.timestamp;
    storedNewValue = true;
  }
  if (storedNewValue && (millis() - lastWrite > WriteThreshold)) {
    storeHistoricalData(HistoryFilePath);
    lastWrite = millis();
  }
}

void AQIReader::loadHistoricalData(String historyFilePath) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations" 
  File historyFile = SPIFFS.open(historyFilePath, "r");
#pragma GCC diagnostic pop

  size_t size = 0;
  if (!historyFile) {
    Log.error(F("Failed to open history file for read: %s"), historyFilePath.c_str());
  } else {
    size = historyFile.size();
    if (size > MaxHistoryFileSize) {
      Log.warning(F("History file is too big: %d"), size);
      return;
    }    
  }

  DynamicJsonDocument doc(MaxHistoryFileSize);
  auto error = deserializeJson(doc, historyFile);
  historyFile.close();
  if (error) {
    Log.warning(F("Failed to parse history file: %s. Size: %d"), error.c_str(), size);
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
    data.timestamp += tzOffset;
    data.pm10_env = reading["pm10_env"];
    data.pm25_env = reading["pm25_env"];
    data.pm100_env = reading["pm100_env"];
    readings_5min.push(data);

    if (data.timestamp > last5minTimestamp) last5minTimestamp = data.timestamp;
  }
  JsonArrayConst dayData = doc[F("day")]["data"];
  for (JsonObjectConst reading : dayData) {
    data.timestamp = reading["ts"];
    data.timestamp += tzOffset;
    data.pm10_env = reading["pm10_env"];
    data.pm25_env = reading["pm25_env"];
    data.pm100_env = reading["pm100_env"];
    readings_1hr.push(data);
    if (data.timestamp > last1hrTimestamp) last1hrTimestamp = data.timestamp;
  }
  JsonArrayConst weekData = doc[F("week")]["data"];
  for (JsonObjectConst reading : weekData) {
    data.timestamp = reading["ts"];
    data.timestamp += tzOffset;
    data.pm10_env = reading["pm10_env"];
    data.pm25_env = reading["pm25_env"];
    data.pm100_env = reading["pm100_env"];
    readings_1day.push(data);
    if (data.timestamp > last1dayTimestamp) last1dayTimestamp = data.timestamp;
  }

// Log.verbose("History as read from file:");
// emitHistoryAsJson(Range_1Hour, Serial);
// emitHistoryAsJson(Range_1Day, Serial);
// emitHistoryAsJson(Range_1Week, Serial);
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
  Log.verbose(F("PM 1.0: %d\t\tPM 2.5: %d\t\tPM 10: %d"), data.pm10_env ,data.pm25_env, data.pm100_env);
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
  Log.verbose(F("Last 1 hour: %F"), pm25_env_1hr.getAverage());
  Log.verbose(F("Last 6 hours: %F"), pm25_env_6hr.getAverage());
  Log.verbose(F("Last 1 day: %F"), pm25_env_1day.getAverage());
  Log.verbose(F("Last 1 week: %F"), pm25_env_1week.getAverage());
}

