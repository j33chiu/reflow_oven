#include "TempSensor.h"

TempSensor& TempSensor::get_instance() {
  static TempSensor instance;
  return instance;
}

TempSensor::TempSensor() {
  one_wire = OneWire(ONE_WIRE_BUS);
  temp_sensor = DallasTemperature(&one_wire);
}

void TempSensor::setup() {
  this->temp_sensor.begin();
}

void TempSensor::update() {
  this->temp_sensor.requestTemperatures();
}

uint8_t TempSensor::getNumDevices() {
  return this->temp_sensor.getDeviceCount();
}

float TempSensor::getTempCByIndex(uint8_t i) {
  return this->temp_sensor.getTempCByIndex(i);
}