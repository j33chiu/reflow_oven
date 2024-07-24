#include "core/SSR.h"
#include <Arduino.h>

SSR& SSR::get_instance(int pin) {
  static SSR instance(pin);
  return instance;
}

SSR::SSR(int pin) {
  this->pin = pin;
  pinMode(pin, OUTPUT);
}

void SSR::set_on() {
  if (this->force_off) {
    this->emer_stop();
    return;
  }
  if (this->state_on) return;
  this->state_on = true;
  digitalWrite(this->pin, HIGH);
}

void SSR::set_off() {
  if (!this->state_on) return;
  this->state_on = false;
  digitalWrite(this->pin, LOW);
}

void SSR::emer_stop() {
  this->force_off = true;
  this->state_on = false;
  digitalWrite(this->pin, LOW);
}

void SSR::reset() {
  this->state_on = false;
  this->force_off = false;
}