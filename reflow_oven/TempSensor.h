#ifndef TEMP_SENSOR_H
#define TEMP_SENSOR_H

#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 8 // D8
#define TEMPERATURE_PRECISION 9

class TempSensor {

public:
  TempSensor();
  void setup();
  void update();
  uint8_t getNumDevices();
  float getTempCByIndex(uint8_t i);

private:
  OneWire one_wire;
  DallasTemperature temp_sensor;
  DeviceAddress sensor_address;

};

#endif