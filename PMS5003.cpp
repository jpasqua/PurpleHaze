/*
 * PMS5003.h
 *
 * This is a mash up of Adafruit's PM25 AQI driver
 * (https://github.com/adafruit/Adafruit_PM25AQI) with 
 * the SwapBap PMS driver (https://github.com/SwapBap/PMS)
 *
 * It is jsut the UART part of the Adafruit driver along
 * with the control commands of the SwapBap driver
 *
 * ORIGINAL NOTICES from Adafruit library:
 * Written by Ladyada for Adafruit Industries.
 * BSD license, all text here must be included in any redistribution.
 *
 */

#include "PMS5003.h"

PMS5003::PMS5003() {}

void PMS5003::sleep() {
  uint8_t command[] = { 0x42, 0x4D, 0xE4, 0x00, 0x00, 0x01, 0x73 };
  _serial_dev->write(command, sizeof(command));
}

void PMS5003::wakeUp() {
  uint8_t command[] = { 0x42, 0x4D, 0xE4, 0x00, 0x01, 0x01, 0x74 };
  _serial_dev->write(command, sizeof(command));
}

void PMS5003::activeMode() {
  uint8_t command[] = { 0x42, 0x4D, 0xE1, 0x00, 0x01, 0x01, 0x71 };
  _serial_dev->write(command, sizeof(command));
  _mode = MODE_ACTIVE;
}

void PMS5003::passiveMode() {
  uint8_t command[] = { 0x42, 0x4D, 0xE1, 0x00, 0x00, 0x01, 0x70 };
  _serial_dev->write(command, sizeof(command));
  _mode = MODE_PASSIVE;
}

void PMS5003::requestRead() {
  if (_mode == MODE_PASSIVE)
  {
    uint8_t command[] = { 0x42, 0x4D, 0xE2, 0x00, 0x00, 0x01, 0x71 };
    _serial_dev->write(command, sizeof(command));
  }
}

bool PMS5003::begin(Stream *theSerial) {
  _serial_dev = theSerial;
  return true;
}

bool PMS5003::read(AQIReadings *data) {
  uint8_t buffer[32];
  uint16_t sum = 0;

  if (!data) {
    return false;
  }

  if (!_serial_dev->available()) {
    return false;
  }
  if (_serial_dev->peek() != 0x42) {
    int p;
    while ( ((p = _serial_dev->peek()) != -1) && (p != 0x42) ) { _serial_dev->read(); }
    if (p == -1) return false;
  }
  // Now read all 32 bytes
  if (_serial_dev->available() < 32) {
    return false;
  }
  _serial_dev->readBytes(buffer, 32);


  // Check that start byte is correct!
  if (buffer[0] != 0x42) {
    return false;
  }

  // get checksum ready
// Serial.print("Raw buffer: ");
  for (uint8_t i = 0; i < 30; i++) {
    sum += buffer[i];
// if (i) Serial.print(", ");
// Serial.print(buffer[i], HEX); 
  }
// Serial.println();

  // The data comes in endian'd, this solves it so it works on all platforms
  uint16_t buffer_u16[15];
  for (uint8_t i = 0; i < 15; i++) {
    buffer_u16[i] = buffer[2 + i * 2 + 1];
    buffer_u16[i] += (buffer[2 + i * 2] << 8);
  }

  uint16_t checksum;
  memcpy(&checksum, (void *)&buffer_u16[14], 2);
  if (sum != checksum) {
    return false;
  }

  // put it into a nice struct :)
  memcpy((void *)&(data->pm10_standard), (void *)&buffer_u16[1], 24);

  if (data->particles_03um + data->particles_05um + data->particles_10um + 
      data->particles_25um + data->particles_50um + data->particles_100um == 0) {
    return false;
  }

  // success!
  return true;
}
