#ifndef SensorConfig_h
#define SensorConfig_h

/*------------------------------------------------------------------------------
 *
 * Weather Sensors
 *
 *----------------------------------------------------------------------------*/

// The types of weather sensor readings. The values of these constants
// must remain in sync with the values defined in WeatherSensor.h
#define READ_TEMP (1 << 0)
#define READ_HUMI (1 << 1)
#define READ_PRES (1 << 2)

// For reference, these are the types of readings that are available
// from each of the sensor types
#define BME280_AVAIL_READINGS   (READ_TEMP | READ_HUMI | READ_PRES)
#define DHT22_AVAIL_READINGS    (READ_TEMP | READ_HUMI )
#define DS18B20_AVAIL_READINGS  (READ_TEMP )

//
// SAMPLE CONFIGURATIONS
//

// constexpr uint8_t BME_I2C_ADDR = 0x76;
// constexpr uint8_t DHT22_PIN = XX;
// constexpr uint8_t DS18B20_PIN = XX;

// Here is a configuration where there is a single BME280 sensor
// #define BME280_READINGS (BME280_AVAIL_READINGS)

// Here is a config with a BME280 and a DS18B20. We want the temp
// value to come from the DS18B20 because it is more stable, accurate
// #define BME280_READINGS   (0         | READ_HUMI | READ_PRES )
// #define DS18B20_READINGS  (READ_TEMP | 0         | 0         )

// Here is a contrived config with a BME280, DHT22, and DS18B20. We
// configure things to get one type of reading from each
// #define BME280_READINGS   (0         | 0         | READ_PRES )
// #define DHT22_READINGS    (0         | READ_HUMI |           )
// #define DS18B20_READINGS  (READ_TEMP | 0         | 0         )


/*------------------------------------------------------------------------------
 *
 * Air Quality Sensors
 *
 *----------------------------------------------------------------------------*/

// The types of air quality sensor readings.
#define PARTICULATE_LEVELS 	(1 << 0)
#define CO2_LEVELS		 			(1 << 1)

// For reference, these are the types of readings that are available
// from each of the sensor types
#define PMS5003_AVAIL_READINGS   (PARTICULATE_LEVELS)

//
// SAMPLE CONFIGURATIONS
//
// #define PMS5003_READINGS PMS5003_AVAIL_READINGS

#endif	// SensorConfig_h