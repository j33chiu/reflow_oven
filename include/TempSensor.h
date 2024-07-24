#ifndef TEMP_SENSOR_H
#define TEMP_SENSOR_H

#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 8 // D8
#define TEMPERATURE_PRECISION 9

// singleton temperature sensor class
class TempSensor {

public:
  // make assignment and copy inaccessible
  TempSensor(TempSensor const&) = delete;
  TempSensor(TempSensor &&) = delete;
  TempSensor& operator=(TempSensor const&) = delete;
  TempSensor& operator=(TempSensor &&) = delete;

  static TempSensor& get_instance();

  void setup();
  void update();
  uint8_t getNumDevices();
  float getTempCByIndex(uint8_t i);

private:
  TempSensor();

  OneWire one_wire;
  DallasTemperature temp_sensor;
  DeviceAddress sensor_address;

};

#endif