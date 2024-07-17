#include "ReflowProfile.h"

ReflowProfile::ReflowProfile() {

}

void ReflowProfile::start() {
  this->start_millis = millis();
}

float ReflowProfile::get_target_temp() {
  float elapsed_s_from_start = (millis() - this->start_millis) / 1000.0f;
   
}