/* <CAPSTONE PROJECT>
** ===============================================================================
** MEMBER        | AFFILIATION                                                   |
** ===============================================================================
** Hwan-hee Jeon | School of Mechanical Engineering, Chonnam National University |
** Hak-jung Im   | School of Mechanical Engineering, Chonnam National University |
** Ki-jeong Lim  | School of Mechanical Engineering, Chonnam National University |
** ===============================================================================
*/

#include "header.hpp"

bigInt_t pow10(int const n)
{
  bigInt_t res = 1;
  for (int i = 0; i < n; i++)
  {
    res *= 10;
  }
  return res;
}

Timer::Timer()
  : begTime{ 0 }
{
  begTime = millis();
}
Timer::~Timer()
{
}
millis_t Timer::getDuration() const
{
  return millis() - begTime;
}
void Timer::wait(millis_t const duration) const
{
  while (getDuration() < duration)
  {
    delay(1);
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
Val_t ReaderAnalogPin::readSignal(millis_t const duration) const
{
  Timer hourglass;
  bigInt_t sum_of_vals = 0;
  bigInt_t cnt_of_vals = 0;

  while (hourglass.getDuration() < duration)
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
  sout << "The pin " << pinId << " is initalized to " << (is_high ? "HIGH." : "LOW.");
  pinMode(pinId, OUTPUT);
  syncPin();
}
void WriterDigitalPin::turnOn()
{
  is_high = true;
  sout << "The pin " << pinId << " set to be " << "HIGH.";
  syncPin();
}
void WriterDigitalPin::turnOff()
{
  is_high = false;
  sout << "The pin " << pinId << " set to be " << "LOW.";  
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
    sout << "The pin " << pinId << " set to be " << "LOW.";  
    analogWrite(pinId, LOW);
  }
  else if (duty_ratio >= 1.0)
  {
    sout << "The pin " << pinId << " set to be " << "HIGH.";  
    analogWrite(pinId, HIGH);
  }
  else
  {
    int const PWM_value = 256 * duty_ratio;
    sout << "The pin " << pinId << " set to be " << PWM_value << ".";  
    analogWrite(pinId, PWM_value);
  }
}
void PwmDigitalPin::init() const
{
  pinMode(pinId, OUTPUT);
  sout << "The pin " << pinId << " is initalized to " << "LOW.";
  analogWrite(pinId, LOW);
}
void PwmDigitalPin::set(double const duty_ratio) const
{
  setPWM(duty_ratio);
}
void PwmDigitalPin::initWith(double const duty_ratio) const
{
  init();
  setPWM(duty_ratio);
}

AscMap::AscMap(const double *const _ys, double const _left_bound_of_xs, size_t const _size_of_ys, double const _right_bound_of_xs)
  : left_bound_of_xs{ _left_bound_of_xs }
  , right_bound_of_xs{ _right_bound_of_xs }
  , ys{ _ys }
  , size_of_ys{ _size_of_ys }
{
}
AscMap::~AscMap()
{
}
double AscMap::get_x_from_parameter(double const param) const
{
  return ((right_bound_of_xs - left_bound_of_xs) / ((int)size_of_ys - 1.0) * param + left_bound_of_xs);
}
double AscMap::get_x_from_y(double const y) const
{
  int low = 0, high = size_of_ys - 1;

  while (low <= high)
  {
    int mid = low + ((high - low) / 2);

    if (ys[mid] > y)
    {
      high = mid - 1;
    }
    else if (ys[mid] < y)
    {
      low = mid + 1;
    }
    else
    {
      return get_x_from_parameter((double)mid);
    }
  }
  if (low >= size_of_ys)
  {
    return get_x_from_parameter((int)size_of_ys - 1.0);
  }
  else if (high < 0)
  {
    return get_x_from_parameter(0.0);
  }
  else
  {
    return get_x_from_parameter(((y - ys[high]) / (ys[low] - ys[high])) * (low - high) + high);
  }
}
