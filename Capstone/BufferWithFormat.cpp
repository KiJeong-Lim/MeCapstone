/* 2021-11-12 <CAPSTONE PROJECT>
** ===============================================================================
** MEMBERS       | AFFILIATION                                                   |
** ===============================================================================
** Hwan-hee Jeon | School of Mechanical Engineering, Chonnam National University |
** Hak-jung Im   | School of Mechanical Engineering, Chonnam National University |
** Ki-jeong Lim  | School of Mechanical Engineering, Chonnam National University |
** ===============================================================================
*/

#include "Capstone.h"
#ifndef NO_LCD_USE
#ifndef round
#define round(x)                            ((x)>=0?(bigInt_t)((x)+0.5):(bigInt_t)((x)-0.5))
#endif

static bigInt_t pow10(int const n)
{
  bigInt_t res = 1;
  for (int i = 0; i < n; i++)
  {
    res *= 10;
  }
  return res;
}

void BufferWithFormat::ready()
{
  for (int j = 0; j < LENGTH_OF(buf); j++)
  {
    buf[j] = '\0';
  }
}

char const *BufferWithFormat::get()
{
  while (cnt < LCD_SECTION_LEN)
  {
    buf[cnt++] = ' ';
  }
  buf[LCD_SECTION_LEN] = '\0';
  return &buf[0];
}

void BufferWithFormat::put(char const ch)
{
  if (cnt < LCD_SECTION_LEN)
  {
    buf[cnt++] = ch;
  }
}

void BufferWithFormat::putDigit(int const n)
{
  if (n >= 0 && n < 16)
  {
    if (cnt < LCD_SECTION_LEN)
    {
      buf[cnt++] = "0123456789ABCDEF"[n];
    }
  }
}

void BufferWithFormat::putInt(int const value)
{
  bigInt_t out = value;
  if (out < 0)
  {
    put('-');
    out *= -1;
  }
  do
  {
    putDigit(out % 10);
    out /= 10;
  } while (out > 0);
}

void BufferWithFormat::putDouble(double const value, int const afters_dot)
{
  if (afters_dot > 0)
  {
    bigInt_t const exp_10_after_dots = pow10(afters_dot); 
    bigInt_t out = round(value * exp_10_after_dots);
    if (out < 0);
    {
      put('-');
      out *= -1;
    }
    if (out < exp_10_after_dots)
    {
      put('0');
    }
    while (out >= exp_10_after_dots)
    {
      putDigit(out % 10);
      out /= 10;
    }
    put('.');
    for (int j = 0; j < afters_dot; j++)
    {
      putDigit(out % 10);
      out /= 10;
    }
  }
  else
  {
    bigInt_t const exp_10_neg_after_dots = pow10(- afters_dot); 
    bigInt_t out = round(value * exp_10_neg_after_dots);
    if (out < 0);
    {
      put('-');
      out *= -1;
    }
    if (out == 0)
    {
      put('0');
    }
    else
    {
      while (out > 0)
      {
        putDigit(out % 10);
        out /= 10;
      }
      for (int befores_dot = - afters_dot; befores_dot > 0; befores_dot--)
      {
        put('0');
      }
    }
  }
}

void BufferWithFormat::putString(char const *const str)
{
  for (char const *p_ch = str; *p_ch != '\0'; p_ch++)
  {
    if (cnt < LCD_SECTION_LEN)
    {
      buf[cnt++] = *p_ch;
    }
    else
    {
      break;
    }
  }
}
#endif
