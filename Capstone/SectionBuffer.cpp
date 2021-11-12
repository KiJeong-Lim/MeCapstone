/* 2021-11-12 <CAPSTONE PROJECT>
** ===============================================================================
** MEMBERS       | AFFILIATION                                                   |
** ===============================================================================
** Hwan-hee Jeon | School of Mechanical Engineering, Chonnam National University |
** Hak-jung Im   | School of Mechanical Engineering, Chonnam National University |
** Ki-jeong Lim  | School of Mechanical Engineering, Chonnam National University |
** ===============================================================================
*/

#define PURE_CPP "SectionBuffer"
#include "Capstone.h"
#ifndef round
#define round(x)                            ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))
#endif

inline int long long pow10(int n)
{
  int long long res = 1;
  for (int i = 0; i < n; i++)
  {
    res *= 10;
  }
  return res;
}

void SectionBuffer::ready()
{
  for (int j = 0; j < LENGTH_OF(line); j++)
  {
    line[j] = '\0';
  }
}

char const *SectionBuffer::get()
{
  while (cnt < LCD_SECTION_LEN)
  {
    line[cnt++] = ' ';
  }
  return &line[0];
}

void SectionBuffer::put(char const ch)
{
  if (cnt < LCD_SECTION_LEN)
  {
    line[cnt++] = ch;
  }
}

void SectionBuffer::putDigit(int const n)
{
  if (n >= 0 && n < 16)
  {
    if (cnt < LCD_SECTION_LEN)
    {
      line[cnt++] = "0123456789ABCDEF"[n];
    }
  }
}

void SectionBuffer::putInt(int value)
{
  if (value < 0)
  {
    put('-');
    value *= -1;
  }
  do
  {
    putDigit(value % 10);
    value /= 10;
  } while(value > 0);
}

void SectionBuffer::putDouble(double const value, int const afters_dot)
{
  if (afters_dot > 0)
  {
    int long long const exp_10_after_dots = pow10(afters_dot); 
    int long long out = round(value * exp_10_after_dots);

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
    int long long const exp_10_neg_after_dots = pow10(- afters_dot); 
    int long out = round(value * exp_10_neg_after_dots);

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

void SectionBuffer::putString(char const *const str)
{
  for (char const *p_ch = str; *p_ch != '\0'; p_ch++)
  {
    if (cnt < LCD_SECTION_LEN)
    {
      line[cnt++] = *p_ch;
    }
  }
}
