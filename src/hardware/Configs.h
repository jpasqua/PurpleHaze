//
// A Hardware configuration is a bundle of definitions that describe a
// particular collection of hardware; e.g., what kind of display is attached,
// which pins are being used as buttons, which pins to use for standard
// functions (e.g. SCL and SDA). Most of the definitions are runtime constants,
// but some may be compile-time constants (#define's)
//
//

#ifndef Configs_h
#define Configs_h

#if (BaseConfig == Config_PH2_Board)
  /*------------------------------------------------------------------------------
   *
   * Config Info for Purple Haze v2 Board with D1 Mini. Of course, a system of
   * discrete parts wired the same way can also use this configuration
   *
   *----------------------------------------------------------------------------*/
  #define PROCESSOR_ASSERT PROCESSOR_ESP8266

  // ----- I2C Settings
  constexpr Basics::Pin SDA_PIN = D3;
  constexpr Basics::Pin SCL_PIN = D5;

  // ----- Display Type
  #if defined(GUI_DSPLY)
    constexpr auto DISPLAY_DRIVER = DisplayDeviceOptions::DeviceType::SH1106;
    constexpr uint8_t DISPLAY_I2C_ADDRESS = 0x3c;
  #else
    constexpr auto DISPLAY_DRIVER = DisplayDeviceOptions::DeviceType::NONE;
    constexpr uint8_t DISPLAY_I2C_ADDRESS = 0x00;
  #endif

  // ----- Buttons
  #if (CTRL_BTNS == PRESENT)
    constexpr Basics::Pin physicalButtons[] = { 27, 14 };
  #else
    constexpr Basics::Pin physicalButtons[] = { Basics::UnusedPin, Basics::UnusedPin };
  #endif
  constexpr Basics::Pin syntheticGrounds[] = { Basics::UnusedPin };

  // ----- Air Quality Sensor
  #define USE_SW_SERIAL 1
  #if (AQI_SENSOR == PRESENT)
    #define PMS5003_READINGS PMS5003_AVAIL_READINGS
    constexpr Basics::Pin SENSOR_RX_PIN = D6;
    constexpr Basics::Pin SENSOR_TX_PIN = D8;
  #elif (AQI_SENSOR == MOCK)
    #define PMS5003_READINGS PMS5003_AVAIL_READINGS
    constexpr Basics::Pin SENSOR_RX_PIN = Basics::UnusedPin;
    constexpr Basics::Pin SENSOR_TX_PIN = Basics::UnusedPin;
  #else
    constexpr Basics::Pin SENSOR_RX_PIN = Basics::UnusedPin;
    constexpr Basics::Pin SENSOR_TX_PIN = Basics::UnusedPin;
  #endif

  #if (BME_SENSOR == PRESENT)
    #define BME280_READINGS (BME280_AVAIL_READINGS)
    constexpr uint8_t BME_I2C_ADDR = 0x76;
  #elif (BME_SENSOR == MOCK)
    #define BME280_READINGS (BME280_AVAIL_READINGS)
    constexpr uint8_t BME_I2C_ADDR = 0x00;
  #else
    constexpr uint8_t BME_I2C_ADDR = 0x00;
  #endif

  // ----- Indicators
  constexpr Basics::Pin NEOPIXEL_PIN = D2;
  constexpr neoPixelType NEOPIXEL_TYPE = NEO_RGB + NEO_KHZ800;


#elif (BaseConfig == Config_PH1_Board)
  /*------------------------------------------------------------------------------
   *
   * Config Info for Purple Haze v1 Board with D1 Mini. This board does not allow
   * for a display or temperature sensor so these are hard-coded off.
   * Of course, a system of discrete parts wired the same way can also use
   * this configuration
   *
   *----------------------------------------------------------------------------*/
  #define PROCESSOR_ASSERT PROCESSOR_ESP8266

  // ----- I2C Settings
  constexpr Basics::Pin SDA_PIN = D3;
  constexpr Basics::Pin SCL_PIN = D5;

  // ----- Display Type
  #if defined(GUI_DSPLY)
    #error("No display may be attached in this configuration")
  #else
    constexpr auto DISPLAY_DRIVER = DisplayDeviceOptions::DeviceType::NONE;
    constexpr uint8_t DISPLAY_I2C_ADDRESS = 0x00;
  #endif

  // ----- Buttons
  #if (CTRL_BTNS == PRESENT)
    #error Config says CTRL_BTNS are present, but this HW has none
  #else
    constexpr Basics::Pin physicalButtons[] = { Basics::UnusedPin, Basics::UnusedPin };
    constexpr Basics::Pin syntheticGrounds[] = { Basics::UnusedPin };
  #endif

  // ----- Air Quality Sensor
  #define USE_SW_SERIAL 1
  #if (AQI_SENSOR == PRESENT)
    #define PMS5003_READINGS PMS5003_AVAIL_READINGS
    constexpr Basics::Pin SENSOR_RX_PIN = D6;
    constexpr Basics::Pin SENSOR_TX_PIN = D8;
  #elif (AQI_SENSOR == MOCK)
    #define PMS5003_READINGS PMS5003_AVAIL_READINGS
    constexpr Basics::Pin SENSOR_RX_PIN = Basics::UnusedPin;
    constexpr Basics::Pin SENSOR_TX_PIN = Basics::UnusedPin;
  #else
    constexpr Basics::Pin SENSOR_RX_PIN = Basics::UnusedPin;
    constexpr Basics::Pin SENSOR_TX_PIN = Basics::UnusedPin;
  #endif

  #if (BME_SENSOR == MOCK)
    #define BME280_READINGS (BME280_AVAIL_READINGS)
    constexpr uint8_t BME_I2C_ADDR = 0x00;
  #else
    constexpr uint8_t BME_I2C_ADDR = 0x00;
  #endif

  // ----- Indicators
  constexpr Basics::Pin NEOPIXEL_PIN = D2;
  constexpr neoPixelType NEOPIXEL_TYPE = NEO_RGB + NEO_KHZ800;


#elif (BaseConfig == Config_D1Mini_Enc)
  /*------------------------------------------------------------------------------
   *
   * Config Info for D1Mini with 1.3" SH1106 display in a small enclosure which
   * limits what may be connected and how. This configuration does not support
   * a physical AQI sensor (though you can mock one).
   *
   *----------------------------------------------------------------------------*/
  #define PROCESSOR_ASSERT PROCESSOR_ESP8266

  // ----- I2C Settings
  constexpr Basics::Pin SDA_PIN = D2;
  constexpr Basics::Pin SCL_PIN = D5;

  // ----- Display Type
  #if defined(GUI_DSPLY)
    constexpr auto DISPLAY_DRIVER = DisplayDeviceOptions::DeviceType::SH1106;
    constexpr uint8_t DISPLAY_I2C_ADDRESS = 0x3c;
  #else
    constexpr auto DISPLAY_DRIVER = DisplayDeviceOptions::DeviceType::NONE;
    constexpr uint8_t DISPLAY_I2C_ADDRESS = 0x00;
  #endif

  // ----- Buttons
  #if (CTRL_BTNS == PRESENT)
    constexpr Basics::Pin physicalButtons[] = { D6, Basics::UnusedPin };
    constexpr Basics::Pin syntheticGrounds[] = { D8 };
  #else
    constexpr Basics::Pin physicalButtons[] = { Basics::UnusedPin, Basics::UnusedPin };
    constexpr Basics::Pin syntheticGrounds[] = { Basics::UnusedPin };
  #endif

  // ----- Air Quality Sensor
  #define USE_SW_SERIAL 1
  constexpr Basics::Pin SENSOR_RX_PIN = Basics::UnusedPin;
  constexpr Basics::Pin SENSOR_TX_PIN = Basics::UnusedPin;
  #if (AQI_SENSOR == PRESENT)
    #error("An AQI Sensor may not be used in this configuration")
  #elif (AQI_SENSOR == MOCK)
    #define PMS5003_READINGS PMS5003_AVAIL_READINGS
  #endif

  #if (BME_SENSOR == PRESENT)
    #define BME280_READINGS (BME280_AVAIL_READINGS)
    constexpr uint8_t BME_I2C_ADDR = 0x76;
  #elif (BME_SENSOR == MOCK)
    #define BME280_READINGS (BME280_AVAIL_READINGS)
    constexpr uint8_t BME_I2C_ADDR = 0x00;
  #endif

  #if (DHT22_SENSOR == PRESENT)
    #define DHT22_READINGS (DHT22_AVAIL_READINGS)
    constexpr uint8_t DHT22_PIN = D5;
  #endif

  // ----- Indicators
  constexpr Basics::Pin NEOPIXEL_PIN = Basics::UnusedPin;
  constexpr neoPixelType NEOPIXEL_TYPE = 0;     // Not Used

#elif (BaseConfig == Config_D1Mini_JAWS)
  /*------------------------------------------------------------------------------
   *
   * Config Info for D1Mini with no display
   *
   *----------------------------------------------------------------------------*/
  #define PROCESSOR_ASSERT PROCESSOR_ESP8266

  // ----- I2C Settings
  constexpr Basics::Pin SDA_PIN = D2;
  constexpr Basics::Pin SCL_PIN = D1;

  // ----- Display Type
  #if defined(GUI_DSPLY)
    constexpr auto DISPLAY_DRIVER = DisplayDeviceOptions::DeviceType::SH1106;
    constexpr uint8_t DISPLAY_I2C_ADDRESS = 0x3c;
  #else
    constexpr auto DISPLAY_DRIVER = DisplayDeviceOptions::DeviceType::NONE;
    constexpr uint8_t DISPLAY_I2C_ADDRESS = 0x00;
  #endif

  // ----- Buttons
  #if (CTRL_BTNS == PRESENT)
    constexpr Basics::Pin physicalButtons[] = { D6, Basics::UnusedPin };
    constexpr Basics::Pin syntheticGrounds[] = { D8 };
  #else
    constexpr Basics::Pin physicalButtons[] = { Basics::UnusedPin, Basics::UnusedPin };
    constexpr Basics::Pin syntheticGrounds[] = { Basics::UnusedPin };
  #endif

  // ----- Air Quality Sensor
  #define USE_SW_SERIAL 1
  constexpr Basics::Pin SENSOR_RX_PIN = Basics::UnusedPin;
  constexpr Basics::Pin SENSOR_TX_PIN = Basics::UnusedPin;
  #if (AQI_SENSOR == PRESENT)
    #error("An AQI Sensor may not be used in this configuration")
  #elif (AQI_SENSOR == MOCK)
    #define PMS5003_READINGS PMS5003_AVAIL_READINGS
  #endif

  #if (BME_SENSOR == PRESENT)
    #define BME280_READINGS (BME280_AVAIL_READINGS)
    constexpr uint8_t BME_I2C_ADDR = 0x76;
  #elif (BME_SENSOR == MOCK)
    #define BME280_READINGS (BME280_AVAIL_READINGS)
    constexpr uint8_t BME_I2C_ADDR = 0x00;
  #endif

  #if (DHT22_SENSOR == PRESENT)
    #define DHT22_READINGS (DHT22_AVAIL_READINGS)
    constexpr uint8_t DHT22_PIN = D5;
  #endif

  // ----- Indicators
  constexpr Basics::Pin NEOPIXEL_PIN = Basics::UnusedPin;
  constexpr neoPixelType NEOPIXEL_TYPE = 0;     // Not Used


#elif (BaseConfig == Config_ESP8266WithOLED)
  /*------------------------------------------------------------------------------
   *
   * Config for an ESP8266 with embedded 0.96" OLED in a small enclosure which
   * limits what may be connected and how. This configuration does not support
   * a physical AQI sensor (though you can mock one).
   *
   *----------------------------------------------------------------------------*/
  #define PROCESSOR_ASSERT PROCESSOR_ESP8266

  // ----- I2C Settings
  constexpr Basics::Pin SDA_PIN = D1;
  constexpr Basics::Pin SCL_PIN = D2;

  // ----- Display Type
  constexpr auto DISPLAY_DRIVER = DisplayDeviceOptions::DeviceType::SSD1306;
  constexpr uint8_t DISPLAY_I2C_ADDRESS = 0x3c;

  // ----- Buttons
  #if (CTRL_BTNS == PRESENT)
    constexpr Basics::Pin physicalButtons[] = { D3, Basics::UnusedPin };
  #else
    constexpr Basics::Pin physicalButtons[] = { Basics::UnusedPin, Basics::UnusedPin };
  #endif
  constexpr Basics::Pin syntheticGrounds[] = { Basics::UnusedPin };

  // ----- Air Quality Sensor
  #define USE_SW_SERIAL 1
  constexpr Basics::Pin SENSOR_RX_PIN = Basics::UnusedPin;
  constexpr Basics::Pin SENSOR_TX_PIN = Basics::UnusedPin;
  #if (AQI_SENSOR == PRESENT)
    #error("An AQI Sensor may not be used in this configuration")
  #elif (AQI_SENSOR == MOCK)
    #define PMS5003_READINGS PMS5003_AVAIL_READINGS
  #endif

  #if (BME_SENSOR == PRESENT)
    #define BME280_READINGS (BME280_AVAIL_READINGS)
    constexpr uint8_t BME_I2C_ADDR = 0x76;
  #elif (BME_SENSOR == MOCK)
    #define BME280_READINGS (BME280_AVAIL_READINGS)
    constexpr uint8_t BME_I2C_ADDR = 0x00;
  #endif

  // ----- Indicators
  constexpr Basics::Pin NEOPIXEL_PIN = Basics::UnusedPin;
  constexpr neoPixelType NEOPIXEL_TYPE = 0;     // Not Used


#elif (BaseConfig == Config_ESP32WithOLED)
  /*------------------------------------------------------------------------------
   *
   * Config for an ESP32 with embedded 0.96" OLED in a small enclosure which
   * limits what may be connected and how. This configuration does not support
   * a physical AQI sensor (though you can mock one).
   * - WEMOS LOLIN32
   *----------------------------------------------------------------------------*/
  #define PROCESSOR_ASSERT PROCESSOR_ESP32

  // ----- I2C Settings
  constexpr Basics::Pin SDA_PIN = 5;
  constexpr Basics::Pin SCL_PIN = 4;

  // ----- Display Type
  constexpr auto DISPLAY_DRIVER = DisplayDeviceOptions::DeviceType::SSD1306;
  constexpr uint8_t DISPLAY_I2C_ADDRESS = 0x3c;

  // ----- Buttons
  #if (CTRL_BTNS == PRESENT)
    constexpr Basics::Pin physicalButtons[] = { 25, 13 };
  #else
    constexpr Basics::Pin physicalButtons[] = { Basics::UnusedPin, Basics::UnusedPin };
  #endif
  constexpr Basics::Pin syntheticGrounds[] = { Basics::UnusedPin };

  // ----- Air Quality Sensor
  constexpr Basics::Pin SENSOR_RX_PIN = 16;
  constexpr Basics::Pin SENSOR_TX_PIN = 17;
  #if (AQI_SENSOR == PRESENT)
    #error("An AQI Sensor may not be used in this configuration")
  #elif (AQI_SENSOR == MOCK)
    #define PMS5003_READINGS PMS5003_AVAIL_READINGS
  #endif

  #if (BME_SENSOR == PRESENT)
    #define BME280_READINGS (BME280_AVAIL_READINGS)
    constexpr uint8_t BME_I2C_ADDR = 0x76;
  #elif (BME_SENSOR == MOCK)
    #define BME280_READINGS (BME280_AVAIL_READINGS)
    constexpr uint8_t BME_I2C_ADDR = 0x00;
  #endif

  // ----- Indicators
  constexpr Basics::Pin NEOPIXEL_PIN = Basics::UnusedPin;
  constexpr neoPixelType NEOPIXEL_TYPE = 0;     // Not Used


#elif (BaseConfig == Config_ESP32Mini)
  /*------------------------------------------------------------------------------
   *
   * Config for an ESP32 D1 Mini built up with discrete components
   *
   *----------------------------------------------------------------------------*/
  #define PROCESSOR_ASSERT PROCESSOR_ESP32

  // ----- I2C Settings
  constexpr Basics::Pin SDA_PIN = 21;
  constexpr Basics::Pin SCL_PIN = 22;

  // ----- Display Type
  #if defined(GUI_DSPLY)
    constexpr auto DISPLAY_DRIVER = DisplayDeviceOptions::DeviceType::SH1106;
    constexpr uint8_t DISPLAY_I2C_ADDRESS = 0x3c;
  #else
    constexpr auto DISPLAY_DRIVER = DisplayDeviceOptions::DeviceType::NONE;
    constexpr uint8_t DISPLAY_I2C_ADDRESS = 0x00;
  #endif

  // ----- Buttons
  #if (CTRL_BTNS == PRESENT)
    constexpr Basics::Pin physicalButtons[] = { 13, Basics::UnusedPin };
  #else
    constexpr Basics::Pin physicalButtons[] = { Basics::UnusedPin, Basics::UnusedPin };
  #endif
  constexpr Basics::Pin syntheticGrounds[] = { Basics::UnusedPin };


  // ----- Air Quality Sensor
  #if (AQI_SENSOR == PRESENT)
    #define PMS5003_READINGS PMS5003_AVAIL_READINGS
    constexpr Basics::Pin SENSOR_RX_PIN = 16;
    constexpr Basics::Pin SENSOR_TX_PIN = 17;
  #elif (AQI_SENSOR == MOCK)
    #define PMS5003_READINGS PMS5003_AVAIL_READINGS
    constexpr Basics::Pin SENSOR_RX_PIN = Basics::UnusedPin;
    constexpr Basics::Pin SENSOR_TX_PIN = Basics::UnusedPin;
  #else
    constexpr Basics::Pin SENSOR_RX_PIN = Basics::UnusedPin;
    constexpr Basics::Pin SENSOR_TX_PIN = Basics::UnusedPin;
  #endif

  #if (BME_SENSOR == PRESENT)
    #define BME280_READINGS (BME280_AVAIL_READINGS)
    constexpr uint8_t BME_I2C_ADDR = 0x76;
  #elif (BME_SENSOR == MOCK)
    #define BME280_READINGS (BME280_AVAIL_READINGS)
    constexpr uint8_t BME_I2C_ADDR = 0x00;
  #endif

  // ----- Indicators
  constexpr Basics::Pin NEOPIXEL_PIN = 5;
  constexpr neoPixelType NEOPIXEL_TYPE = NEO_RGB + NEO_KHZ800;


#elif (BaseConfig == Config_PH2_ESP32)
  /*------------------------------------------------------------------------------
   *
   * Config for an ESP32 D1 Mini (or dev board)
   *
   *----------------------------------------------------------------------------*/
  #define PROCESSOR_ASSERT PROCESSOR_ESP32

  // ----- I2C Settings
  constexpr Basics::Pin SDA_PIN = 21;
  constexpr Basics::Pin SCL_PIN = 22;

  // ----- Display Type
  #if defined(GUI_DSPLY)
    constexpr auto DISPLAY_DRIVER = DisplayDeviceOptions::DeviceType::SH1106;
    constexpr uint8_t DISPLAY_I2C_ADDRESS = 0x3c;
  #else
    constexpr auto DISPLAY_DRIVER = DisplayDeviceOptions::DeviceType::NONE;
    constexpr uint8_t DISPLAY_I2C_ADDRESS = 0x00;
  #endif

  // ----- Buttons
  #if (CTRL_BTNS == PRESENT)
      #error Config says CTRL_BTNS are present, but this HW has none
  #else
    constexpr Basics::Pin physicalButtons[] = { Basics::UnusedPin, Basics::UnusedPin };
    constexpr Basics::Pin syntheticGrounds[] = { Basics::UnusedPin };
  #endif


  // ----- Air Quality Sensor
  #if (AQI_SENSOR == PRESENT)
    #define PMS5003_READINGS PMS5003_AVAIL_READINGS
    constexpr Basics::Pin SENSOR_RX_PIN = 16;
    constexpr Basics::Pin SENSOR_TX_PIN = 17;
  #elif (AQI_SENSOR == MOCK)
    #define PMS5003_READINGS PMS5003_AVAIL_READINGS
    constexpr Basics::Pin SENSOR_RX_PIN = Basics::UnusedPin;
    constexpr Basics::Pin SENSOR_TX_PIN = Basics::UnusedPin;
  #else
    constexpr Basics::Pin SENSOR_RX_PIN = Basics::UnusedPin;
    constexpr Basics::Pin SENSOR_TX_PIN = Basics::UnusedPin;
  #endif

  #if (BME_SENSOR == PRESENT)
    #define BME280_READINGS (BME280_AVAIL_READINGS)
    constexpr uint8_t BME_I2C_ADDR = 0x76;
  #elif (BME_SENSOR == MOCK)
    #define BME280_READINGS (BME280_AVAIL_READINGS)
    constexpr uint8_t BME_I2C_ADDR = 0x00;
  #endif

  // ----- Indicators
  constexpr Basics::Pin NEOPIXEL_PIN = 13;
  constexpr neoPixelType NEOPIXEL_TYPE = NEO_RGB + NEO_KHZ800;

#endif


#endif  // Configs_h
