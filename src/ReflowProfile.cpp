#include "ReflowProfile.h"

ReflowProfile::ReflowProfile() {

}

bool ReflowProfile::load_profile(ReflowProfileNode* start_node) {
  return false;
}

bool ReflowProfile::is_loaded() {
  return this->profile_loaded;
}

bool ReflowProfile::start_profile() {
  if (!profile_loaded) return false;

  this->start_millis = millis();
  return false;
}

float ReflowProfile::get_target_temp() {
  float elapsed_s_from_start = (millis() - this->start_millis) / 1000.0f;
  return 0.0;
}

bool ReflowProfile::done_profile() {
  return false;
}
