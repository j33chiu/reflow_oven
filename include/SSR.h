#ifndef SSR_H
#define SSR_H

#define DEFAULT_SSR_CONTROL_PIN 9 // D9

class SSR {
public:
  SSR(int pin = DEFAULT_SSR_CONTROL_PIN);
  void set_on();
  void set_off();
  void emer_stop();
  void reset();

private:
  bool force_off = false;
  bool state_on = false;
  int pin;


};

#endif