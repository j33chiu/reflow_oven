#ifndef PID_H
#define PID_H

#define HISTORY_LENGTH 3

class PID {

public:
  PID(float p = 1.0f, float i = 0.0f, float d = 0.0f);
  float update(float error, unsigned long timestamp);

private:
  int measurements = 0;
  float p;
  float i;
  float d;

  float history[HISTORY_LENGTH];
  unsigned long history_timestamp[HISTORY_LENGTH];
};

#endif