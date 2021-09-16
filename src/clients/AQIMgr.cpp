/*
 * AQIMgr
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
#include <WTBasics.h>
#include <WebThing.h>
//                                  Local Includes
#include "AQIMgr.h"
//--------------- End:    Includes ---------------------------------------------


const uint32_t AQIMgr::ColorForState[] = {
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

static const char* HistoryFilePath = "/aqihist.json";

AQIMgr::AQIMgr() {
  aqi = new PMS5003();
  pm25env_10min.setSamplesToConsider(2);         // (2) 5-minute periods
  pm25env_30min.setSamplesToConsider(6);         // (6) 5-minute periods
  pm25env_1hr.setSamplesToConsider(12);          // (12) 5-minute periods
  pm25env_6hr.setSamplesToConsider(6 * 12);      // (6) 1-hour periods
}

bool AQIMgr::init(Stream* streamToSensor, Indicator* indicator) {
  _indicator = indicator;
  if (!aqi->begin(streamToSensor)) {
    _indicator->setColor(255, 0, 0);
    return false;
  }
  readHistoryfromFile();

  enterState(waking);
  return true;
}

uint16_t AQIMgr::derivedAQI(uint16_t reading) {
  int i;
  for (i = 0; i < AQITableLength; i++) {
    if (reading < AQITable[i].pMin) break;
  }
  i--;
  float aqi = ((reading -  AQITable[i].pMin)*(AQITable[i].aqRange))/AQITable[i].pRange + AQITable[i].aqMin;
  return (uint16_t)aqi;
}

void AQIMgr::loop() {
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
    data.timestamp = millis();
    logData(data);
  } else {
    enterState(retrying);
Serial.print('.');
    return;
  }

  takeNoteOfNewData(data);

  enterState(asleep);
}

uint16_t AQIMgr::sizeOfRange(HistoryRange r) {
  switch (r) {
    case Range_1Hour: return readings_5min.size();
    case Range_1Day: return readings_1hr.size();
    case Range_1Week: return readings_6hr.size();
    case Range_Combined: return readings_5min.size() + readings_1hr.size() + readings_6hr.size();
  }
  return 0; // Assert(CantHappen)
}

AQIMgr::SavedReadings AQIMgr::getFromRange(HistoryRange r, uint16_t index) {
  switch (r) {
    case Range_1Hour: return readings_5min.peekAt(index);
    case Range_1Day: return readings_1hr.peekAt(index);
    case Range_1Week: return readings_6hr.peekAt(index);
    case Range_Combined:
      if (index < readings_5min.size()) return readings_5min.peekAt(index);
      index -= readings_5min.size();
      if (index < readings_1hr.size()) return readings_1hr.peekAt(index);;
      index -= readings_1hr.size();
      // Fall out...
  }
  return readings_6hr.peekAt(index);
}

void AQIMgr::enterState(State newState) {
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

void AQIMgr::takeNoteOfNewData(AQIReadings& newSample) {
  // We want to do (3) things here:
  // 1. Add this reading to the appropriate set of history buffers
  // 2. If it has been an appropriate period of time, save the history to a file
  // 3. Update the moving averages and log them


  bool storedNewValue = false;
  SavedReadings readings;
  readings.timestamp = WTBasics::wallClockFromMillis(newSample.timestamp) + WebThing::getGMTOffset();
  readings.env = newSample.env;

  // 1. Add this reading to the appropriate set of history buffers
  if (readings.timestamp - last5minTimestamp >= (5 * 60)) {
    readings_5min.push(readings);
    last5minTimestamp = readings.timestamp;
    storedNewValue = true;
  }
  if (readings.timestamp - last1hrTimestamp >= (60 * 60)) {
    readings_1hr.push(readings);
    last1hrTimestamp = readings.timestamp;
    storedNewValue = true;
  }
  if (readings.timestamp - last1dayTimestamp >= (6 * 60 * 60)) {
    readings_6hr.push(readings);
    last1dayTimestamp = readings.timestamp;
    storedNewValue = true;
  }

  // 2. If it has been an appropriate period of time, save the history to a file
  static const uint32_t WriteThreshold = 10 * 60 * 1000L; // Write every 10 Minutes
  static uint32_t lastWrite = 0;
  if (storedNewValue && (millis() - lastWrite > WriteThreshold)) {
    saveHistoryToFile();
    lastWrite = millis();
  }

  // 3. Update the moving averages and log them
  pm25env_10min.addSample(newSample.env.pm25);
  pm25env_30min.addSample(newSample.env.pm25);
  pm25env_1hr.addSample(newSample.env.pm25);
  pm25env_6hr.addSample(newSample.env.pm25);

  logAvgs();
}

void AQIMgr::readHistoryfromFile() {
  DynamicJsonDocument doc(MaxHistoryFileSize);

  File historyFile = ESP_FS::open(HistoryFilePath, "r");
  if (!historyFile) {
    Log.notice(F("No AQI History file exists."));
    return;
  }

  size_t size = historyFile.size();
  if (size > MaxHistoryFileSize) {
    Log.error(F("History file size is too large (%d)"), size);
    return;
  }

  auto error = deserializeJson(doc, historyFile);
  historyFile.close();
  if (error) {
    Log.warning(F("Failed to parse history file: %s"), error.c_str());
    return;
  }

  JsonObjectConst obj;
  obj = doc["hour"];  readings_5min.load(obj);
  obj = doc["day"];   readings_1hr.load(obj);
  obj = doc["week"];  readings_6hr.load(obj);
}

void AQIMgr::emitHistoryAsJson(HistoryRange r, Stream& s) {
  switch (r) {
    case Range_1Hour: readings_5min.store(s); break;
    case Range_1Day: readings_1hr.store(s); break;
    case Range_1Week: readings_6hr.store(s); break;
    case Range_Combined: {
      s.print("{ \"hour\":"); readings_5min.store(s); s.println(",");
      s.print("  \"day\":");  readings_1hr.store(s);  s.println(",");
      s.print("  \"week\":"); readings_6hr.store(s); s.println("}");
    }
  }
}

void AQIMgr::saveHistoryToFile() {
  File historyFile = ESP_FS::open(HistoryFilePath, "w");
  if (!historyFile) {
    Log.notice(F("Could not open history file for write: %s"), HistoryFilePath);
    return;
  }

  emitHistoryAsJson(Range_Combined, historyFile);
}


void AQIMgr::logData(AQIReadings& data) {
  time_t wallClockOfReading = WTBasics::wallClockFromMillis(data.timestamp);
  Log.verbose("-- %s --", WebThing::formattedTime(wallClockOfReading, true, true).c_str());
  Log.verbose(F("---------------------------------------"));
  Log.verbose(F("Concentration Units (standard)"));
  Log.verbose(F("---------------------------------------"));
  Log.verbose(F("PM 1.0: %d\t\tPM 2.5: %d\t\tPM 10: %d"), data.standard.pm10 ,data.standard.pm25, data.standard.pm100);
  Log.verbose(F("---------------------------------------"));
  Log.verbose(F("PM 1.0: %d\t\tPM 2.5: %d\t\tPM 10: %d"), data.env.pm10 ,data.env.pm25, data.env.pm100);
  Log.verbose(F("---------------------------------------"));
  Log.verbose(F("Particles > 0.3um / 0.1L air: %d"), data.particles_03um);
  Log.verbose(F("Particles > 0.5um / 0.1L air: %d"), data.particles_05um);
  Log.verbose(F("Particles > 1.0um / 0.1L air: %d"), data.particles_10um);
  Log.verbose(F("Particles > 2.5um / 0.1L air: %d"), data.particles_25um);
  Log.verbose(F("Particles > 5.0um / 0.1L air: %d"), data.particles_50um);
  Log.verbose(F("Particles > 50 um / 0.1L air: %d"), data.particles_100um);
  Log.verbose(F("---------------------------------------"));
}

void AQIMgr::logAvgs() {
  time_t wallClockOfReading = WTBasics::wallClockFromMillis(data.timestamp);
  Log.verbose("-- %s --", WebThing::formattedTime(wallClockOfReading, true, true).c_str());
  Log.verbose(F("Last 30 Minutes: %F"), pm25env_30min.getAverage());
  Log.verbose(F("Last 1 hour: %F"), pm25env_1hr.getAverage());
  Log.verbose(F("Last 6 hours: %F"), pm25env_6hr.getAverage());
}


static const struct {
  uint16_t max;
  uint32_t color;
  const char* shortDesc;
  const char* longDesc;
} QualityBrackets[] = {
  { 50, 0x00ff00, "Good",
      "Air quality is satisfactory, and air pollution poses little or no risk."},
  {100, 0xffff00, "Moderate",
      "Air quality is acceptable. However, there may be a risk for some people, "
      "particularly those who are unusually sensitive to air pollution."},
  {150, 0xffa500, "Unhealthy for Sensitive Groups",
      "Members of sensitive groups may experience health effects. The general "
      "public is less likely to be affected."},
  {200, 0xff0000, "Unhealthy",
      "Some members of the general public may experience health effects; "
      "members of sensitive groups may experience more serious health effects."},
  {300, 0x9400D3, "Very Unhealthy",
      "Health alert: The risk of health effects is increased for everyone."},
  {999, 0x9400D3, "Hazardous",
      "Health warning of emergency conditions: "
      "everyone is more likely to be affected."},
};
static const uint16_t N_QualityBrackets = (sizeof(QualityBrackets)/sizeof((QualityBrackets)[0]));


void AQIMgr::aqiAsJSON(uint16_t quality, time_t timestamp, String& result) {
  const char* shortDesc = "Unknown";
  const char* longDesc = "Unknown";
  uint32_t color = 0x000000;

  for (int i = 0; i < N_QualityBrackets; i++) {
    if (quality <= QualityBrackets[i].max) {
      shortDesc = QualityBrackets[i].shortDesc;
      longDesc = QualityBrackets[i].longDesc;
      color = QualityBrackets[i].color;
      break;
    }
  }

  time_t wallClock = WTBasics::wallClockFromMillis(timestamp) - WebThing::getGMTOffset();
  result += "{\n";
  result += "  \"timestamp\": "; result += wallClock; result += ",\n";
  result += "  \"aqi\": "; result += quality; result += ",\n";
  result += "  \"shortDesc\": "; result += "\""; result += shortDesc; result += "\",\n";
  result += "  \"longDesc\": "; result += "\""; result += longDesc; result += "\",\n";
  result += "  \"color\": "; result += color; result += "\n";
  result += "}";
}

uint32_t AQIMgr::colorForQuality(uint16_t quality) {
  for (int i = 0; i < N_QualityBrackets; i++) {
    if (quality <= QualityBrackets[i].max) {
      return(QualityBrackets[i].color);
    }
  }
  return 0; // ASSERT: Should not get here
}

// ----- Implementation of the SavedReadings Serializable interface
// JSON externalization is of the form:
// { "timestamp": 2345678,
//   "env": { "pm10": 3, "pm25": 6, "pm100": 1 } }

void AQIMgr::SavedReadings::internalize(const JsonObjectConst &obj) {
  timestamp = obj["timestamp"];
  JsonObjectConst jsonEnv = obj["env"];
  env.pm10 = jsonEnv["pm10"];
  env.pm25 = jsonEnv["pm25"];
  env.pm100 = jsonEnv["pm100"];
}

void AQIMgr::SavedReadings::externalize(Stream& writeStream) const {
  StaticJsonDocument<96> doc; // Size provided by https://arduinojson.org/v6/assistant

  doc["timestamp"] = timestamp;
  JsonObject jsonEnv = doc.createNestedObject("env");
  jsonEnv["pm10"] = env.pm10;
  jsonEnv["pm25"] = env.pm25;
  jsonEnv["pm100"] = env.pm100;

  serializeJson(doc, writeStream);
}

