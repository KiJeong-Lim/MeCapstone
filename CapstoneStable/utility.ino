#include "header.h"

Timer::Timer()
  : curTime{ 0 }
{
  curTime = millis();
}
Timer::Timer(Timer const &other)
  : curTime{ other.curTime }
{
}
Timer::~Timer()
{
}
void Timer::syncTime()
{
  curTime = millis();
}
ms_t &&Timer::getDuration()
{
  ms_t const beg_time = curTime;
  syncTime();
  return curTime - beg_time;
}
bool Timer::wait(ms_t given_time)
{
  given_time -= getDuration();
  if (given_time >= 0)
  {
    delay(given_time);
    syncTime();
    return true;
  }
  else
  {
    return false;
  }
}

ReaderAnalogPin::ReaderAnalogPin(pinId_t const pin_no)
  : Pin{ .pinId = pin_no }
{
}
ReaderAnalogPin::~ReaderAnalogPin()
{
}
int ReaderAnalogPin::read_once() const
{
  return analogRead(pinId);
}
Val_t ReaderAnalogPin::readSignalOnce() const
{
  return read_once();
}
Val_t ReaderAnalogPin::readSignal(ms_t const duration) const
{
  bigInt_t sum_of_vals = 0;
  bigInt_t cnt_of_vals = 0;
  Timer hourglass;
  for (ms_t remaining_time = duration; remaining_time >= 0; remaining_time -= hourglass.getDuration())
  {
    sum_of_vals += read_once();
    cnt_of_vals++;
  }
  return ((double)sum_of_vals) / ((double)cnt_of_vals);
}

WriterDigitalPin::WriterDigitalPin(pinId_t const pin_no)
  : Pin{ .pinId = pin_no }
  , is_high{ false }
{
}
WriterDigitalPin::~WriterDigitalPin()
{
}
void WriterDigitalPin::syncPin()
{
  digitalWrite(pinId, is_high ? HIGH : LOW);
}
void WriterDigitalPin::initWith(bool const on_is_true)
{
  is_high = on_is_true;
  cout << "The pin " << pinId << " is initalized to " << (is_high ? "HIGH." : "LOW.");
  pinMode(pinId, OUTPUT);
  syncPin();
}
void WriterDigitalPin::turnOn()
{
  is_high = true;
  cout << "The pin " << pinId << " set to be " << "HIGH.";
  syncPin();
}
void WriterDigitalPin::turnOff()
{
  is_high = false;
  cout << "The pin " << pinId << " set to be " << "LOW.";  
  syncPin();
}
bool WriterDigitalPin::isHigh() const
{
  return is_high;
}

PwmDigitalPin::PwmDigitalPin(pinId_t const pin_no)
  : Pin{ .pinId = pin_no }
{
}
void PwmDigitalPin::setPWM(double const duty_ratio) const
{
  if (duty_ratio < 0.0)
  {
    cout << "The pin " << pinId << " set to be " << "LOW.";  
    analogWrite(pinId, LOW);
  }
  else if (duty_ratio >= 1.0)
  {
    cout << "The pin " << pinId << " set to be " << "HIGH.";  
    analogWrite(pinId, HIGH);
  }
  else
  {
    int const PWM_value = 256 * duty_ratio;
    cout << "The pin " << pinId << " set to be " << PWM_value << ".";  
    analogWrite(pinId, PWM_value);
  }
}
void PwmDigitalPin::init() const
{
  pinMode(pinId, OUTPUT);
  cout << "The pin " << pinId << " is initalized to " << "LOW.";
  analogWrite(pinId, LOW);
}
void PwmDigitalPin::setPwm(double const duty_ratio) const
{
  setPWM(duty_ratio);
}
