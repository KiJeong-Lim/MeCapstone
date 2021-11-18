/* <CAPSTONE PROJECT>
** ===============================================================================
** MEMBER        | AFFILIATION                                                   |
** ===============================================================================
** Hwan-hee Jeon | School of Mechanical Engineering, Chonnam National University |
** Hak-jung Im   | School of Mechanical Engineering, Chonnam National University |
** Ki-jeong Lim  | School of Mechanical Engineering, Chonnam National University |
** ===============================================================================
*/

#include "capstone.h"

static inline
void delay1ms()
{
  delay(1);
}

bigInt_t POW(bigInt_t base, int expn)
{
  bigInt_t result = 1;
  
  for (int i = 0; i < expn; i++)
  {
    result *= base;
  }

  return result;
}

LiquidCrystal_I2C *openLcdI2C(int const lcdWidth, int const lcdHeight)
{
  LiquidCrystal_I2C *lcdHandle = nullptr;

  if (lcdWidth > 0 && lcdHeight > 0)
  {
    byte adr = 0xFF;

    do
    {
      byte response = 4;

      Wire.beginTransmission(adr);
      response = Wire.endTransmission(adr);
      if (response == 0)
      {
        sout << "I2C address found: address = " << adr << ".";
        lcdHandle = new LiquidCrystal_I2C(adr, lcdWidth, lcdHeight);
        if (lcdHandle)
        {
          sout << "I2C connected: address = " << adr << ".";
          break;
        }
      }
      adr--;
    } while (adr != 0x00);

    if (lcdHandle)
    {
      lcdHandle->init();
      lcdHandle->backlight();
    }
  }
  return lcdHandle;
}

Timer::Timer()
  : begTime{ 0 }
{
  begTime = millis();
}
Timer::~Timer()
{
}
void Timer::reset()
{
  begTime = millis();
}
millis_t Timer::time() const
{
  return millis() - begTime;
}
void Timer::delay(millis_t const duration) const
{
  while (time() < duration)
  {
    delay1ms();
  }
}

AscMap::AscMap(double const *const _ys, double const _left_bound_of_xs, size_t const _size_of_ys, double const _right_bound_of_xs)
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
