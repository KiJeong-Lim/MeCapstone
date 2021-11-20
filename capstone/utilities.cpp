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

static inline
void delay1ms()
{
  delay(1);
}

void invokeSerial()
{
#if defined(SERIAL_PORT)
  Serial.begin(SERIAL_PORT);
  while (!Serial)
  {
    delay1ms();
  }
#endif
}

void mklineSerial()
{
#if defined(SERIAL_PORT)
  if (Serial)
  {
    Serial.println("=======");
  }
#endif
}

BigInt_t POW(BigInt_t base, int expn)
{
  BigInt_t result = 1;
  
  for (int i = 0; i < expn; i++)
  {
    result *= base;
  }

  return result;
}

Timer::Timer()
  : begTime{ 0 }
{
  begTime = millis();
}
Timer::Timer(ms_t beg_time)
  : begTime{ beg_time }
{
}
Timer::~Timer()
{
}
void Timer::reset()
{
  begTime = millis();
}
ms_t Timer::getDuration() const
{
  return millis() - begTime;
}
ms_t Timer::time() const
{
  return millis() - begTime;
}
void Timer::delay(ms_t const duration) const
{
  while (time() < duration)
  {
    delay1ms();
  }
}

AscMap::~AscMap()
{
}
double AscMap::get_x_from_parameter(double const param) const
{
  return ((param * (right_bound_of_xs - left_bound_of_xs) / number_of_intervals) + left_bound_of_xs);
}
double AscMap::get_x_from_y(double const y) const
{
  int low = 0, high = number_of_intervals;

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
      return get_x_from_parameter(mid);
    }
  }
  if (low > number_of_intervals)
  {
    return get_x_from_parameter(number_of_intervals);
  }
  else if (high < 0)
  {
    return get_x_from_parameter(0);
  }
  else
  {
    return get_x_from_parameter(((y - ys[high]) / (ys[low] - ys[high])) * (low - high) + high);
  }
}
