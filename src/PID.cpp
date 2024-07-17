#include "PID.h"

PID::PID(float p, float i, float d) {
  this->p = p;
  this->i = i;
  this->d = d;
}

float PID::update(float error, unsigned long timestamp) {
  float latest_error = error;

  if (measurements < HISTORY_LENGTH) {
    this->history[measurements] = error;
    this->history_timestamp[measurements] = timestamp;
    measurements++;
  } else {
    // history longer than HISTORY_LENGTH, remove oldest data and shift
    for (int i = 1; i < HISTORY_LENGTH; i++) {
      this->history[i-1] = history[i];
      this->history_timestamp[i-1] = history_timestamp[i];
    }
  }

  if (measurements < HISTORY_LENGTH) {
    // too little history for full pid, just use p
    return this->p * latest_error;
  }

  float dt_avg = 0;
  float err_dot = 0;
  float sum_err = this->history[0];

  for (int i = 1; i < HISTORY_LENGTH; i++) {
    unsigned long t0 = this->history_timestamp[i-1];
    unsigned long t1 = this->history_timestamp[i];

    float dt = (t1 - t0) / 1e9;
    dt_avg += dt;
    sum_err += this->history[i];
    err_dot += (this->history[i] - this->history[i-1]) / dt;
  }

  err_dot /= HISTORY_LENGTH - 1.0;
  dt_avg /= HISTORY_LENGTH - 1.0;
  float err_int = sum_err * dt_avg;

  return (this->p * latest_error) + (this->i * err_int) + (this->d * err_dot);
}