#include "BMESensor.h"


BMESensor::BMESensor(float tempCorrection, float humiCorrection, int elevation) {
  setAttributes(tempCorrection, humiCorrection, elevation);
}

void BMESensor::setAttributes(float tempCorrection, float humiCorrection, int elevation) {
  this->tempCorrection = tempCorrection;
  this->humiCorrection = humiCorrection;
  this->elevation = elevation;
 }

void BMESensor::begin(int addr) {
  bool status;

  mock = false;
  status = bme.begin(addr);
  if (status == 0) {
    Log.error("Could not find a valid BME280 sensor, check wiring!");
    mock = true;
    mockTemp = 20;
    mockHumidity = 30;
    mockPressure = 1013;
    return;
  }

  Log.verbose("forced mode, 1x temperature / 1x humidity / 1x pressure oversampling, filter off");
  bme.setSampling(
    Adafruit_BME280::MODE_FORCED,
    Adafruit_BME280::SAMPLING_X1, // temperature
    Adafruit_BME280::SAMPLING_X1, // pressure
    Adafruit_BME280::SAMPLING_X1, // humidity
    Adafruit_BME280::FILTER_OFF   );
}

void BMESensor::takeReadings(WeatherReadings& readings) {
  readings.timestamp = millis();
  if (mock) {
    readings.temp = mockTemp = constrain(mockTemp + ((float)random(-5, 6))/10.0, 0.0, 36.0);
    readings.humidity = mockHumidity = constrain(mockHumidity + ((float)random(-3, 4))/10.0, 0.0, 100.0);
    readings.pressure = mockPressure = constrain(mockPressure + ((float)random(-1, 2))/10.0, 1011.0, 1016.0);
  } else {
    // Measure: absolute Pressure, Temperature, Humidity, voltage
    // Calculate: Relative Pressure, Dewpoint, Dewpoint Spread, Heat Index
    bme.takeForcedMeasurement();

    // Get temperature
    readings.temp = bme.readTemperature();
    readings.temp += tempCorrection;
    Log.verbose("Temp: %F°C", readings.temp);

    // Get humidity
    readings.humidity = bme.readHumidity();
    readings.humidity += humiCorrection;
    if (readings.humidity > 100) readings.humidity = 100;    // the humiCorrection might lead in a value higher than 100%
    Log.verbose("Humidity: %F%%", readings.humidity);

    // Get pressure
    readings.pressure = bme.readPressure() / 100.0F;
    Log.verbose("Pressure: %F hPa", readings.pressure);    
  }

  // Calculate and print relative pressure
  float SLpressure_hPa = (((readings.pressure * 100.0)/pow((1-((float)(elevation))/44330), 5.255))/100.0);
  readings.relPressure=(int)(SLpressure_hPa+.5);
  Log.verbose("Relative Pressure: %d hPa", readings.relPressure);

  // Calculate dewpoint
  double a = 17.271;
  double b = 237.7;
  double tempcalc = (a * readings.temp) / (b + readings.temp) + log(readings.humidity*0.01);
  readings.dewPointTemp = (b * tempcalc) / (a - tempcalc);
  Log.verbose("Dewpoint: %F°C", readings.dewPointTemp);

  // Calculate dewpoint spread (difference between actual temp and dewpoint -> the smaller the number: rain or fog
  readings.dewPointSpread = readings.temp - readings.dewPointTemp;
  Log.verbose("Dewpoint Spread: %F°C", readings.dewPointSpread);

  // Calculate HI (readings.heatIndex in °C) --> HI starts working above 26.7 °C
  if (readings.temp > 26.7) {
    double c1 = -8.784, c2 = 1.611, c3 = 2.338, c4 = -0.146, c5= -1.230e-2, c6=-1.642e-2, c7=2.211e-3, c8=7.254e-4, c9=-2.582e-6  ;
    double T = readings.temp;
    double R = readings.humidity;
    
    double A = (( c5 * T) + c2) * T + c1;
    double B = ((c7 * T) + c4) * T + c3;
    double C = ((c9 * T) + c8) * T + c6;
    readings.heatIndex = (C * R + B) * R + A; 
  } else {
    readings.heatIndex = readings.temp;
    Log.verbose("Not warm enough (less than 26.7 °C) for heat index");
  }
  Log.verbose("Heat Index: %F°C", readings.heatIndex);
}