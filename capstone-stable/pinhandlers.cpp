/* <CAPSTONE PROJECT>
** ===============================================================================
** MEMBER        | AFFILIATION                                                   |
** ===============================================================================
** Hwan-hee Jeon | School of Mechanical Engineering, Chonnam National University |
** Hak-jung Im   | School of Mechanical Engineering, Chonnam National University |
** Ki-jeong Lim  | School of Mechanical Engineering, Chonnam National University |
** ===============================================================================
*/

#include "capstone.hpp"

PinReader::PinReader(pinId_t const pinId)
  : PinHandler{ .pin_to_handle = pinId }
{
}
PinReader::~PinReader()
{
}
int PinReader::readSignalOnce() const
{
  return analogRead(pin_to_handle);
}
Val_t PinReader::readSignal(ms_t const duration) const
{
  BigInt_t sum_of_vals = 0;
  BigInt_t cnt_of_vals = 0;

  for (Timer hourglass = { }; hourglass.getDuration() < duration; cnt_of_vals++)
  {
    sum_of_vals += this->readSignalOnce();
  }
  return (static_cast<Val_t>(sum_of_vals)) / (static_cast<Val_t>(cnt_of_vals));
}

PinSetter::PinSetter(pinId_t const pinId)
  : PinHandler{ .pin_to_handle = pinId }
  , is_high{ false }
{
}
PinSetter::~PinSetter()
{
}
void PinSetter::openPin() const
{
  pinMode(pin_to_handle, OUTPUT);
}
void PinSetter::syncPin()
{
  digitalWrite(pin_to_handle, is_high ? HIGH : LOW);
}
void PinSetter::initWith(bool const be_high)
{
  is_high = be_high;
  sout << "The pin " << pin_to_handle << " is initalized to " << (is_high ? "HIGH." : "LOW.");
  this->openPin();
  this->syncPin();
}
void PinSetter::turnOn()
{
  is_high = true;
  sout << "The pin " << pin_to_handle << " set to be " << "HIGH.";
  this->syncPin();
}
void PinSetter::turnOff()
{
  is_high = false;
  sout << "The pin " << pin_to_handle << " set to be " << "LOW.";  
  this->syncPin();
}
bool PinSetter::isHigh() const
{
  return is_high;
}

PwmSetter::PwmSetter(pinId_t const pinId)
  : PinHandler{ .pin_to_handle = pinId }
{
}
PwmSetter::~PwmSetter()
{
}
void PwmSetter::openPin() const
{
  pinMode(pin_to_handle, OUTPUT);
}
void PwmSetter::init() const
{
  sout << "The pin " << pin_to_handle << " is initalized to " << "LOW.";
  this->openPin();
  analogWrite(pin_to_handle, LOW);
}
void PwmSetter::set(double const duty_ratio) const
{
  if (duty_ratio < 0.0)
  {
    sout << "The pin " << pin_to_handle << " set to be " << "LOW.";  
    analogWrite(pin_to_handle, LOW);
  }
  else if (duty_ratio >= 1.0)
  {
    sout << "The pin " << pin_to_handle << " set to be " << "HIGH.";  
    analogWrite(pin_to_handle, HIGH);
  }
  else
  {
    int const PWM_value = 256 * duty_ratio;
    sout << "The pin " << pin_to_handle << " set to be " << PWM_value << ".";  
    analogWrite(pin_to_handle, PWM_value);
  }
}
