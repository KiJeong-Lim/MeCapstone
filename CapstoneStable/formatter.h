#ifndef FORMATTER
#define FORMATTER

#include "header.h"

template <size_t Capacity>
class Formatter {
  int cnt = 0;
  char buf[Capacity + 1] = { };
public:
  void clear()
  {
    for (cnt = 0; cnt < Capacity; cnt++)
    {
      buf[cnt] = ' ';
    }
    buf[cnt] = '\0';
    cnt = 0;
  }
  void write(char *p_ch)
  {
    for (cnt = 0; cnt < Capacity; cnt++)
    {
      *p_ch++ = buf[cnt];
    }
  }
  void putChar(char const ch)
  {
    if (cnt < Capacity)
    {
      buf[cnt++] = ch;
    }
  }
  void putDigit(int const d)
  {
    putChar("0123456789ABCDEF"[d]);
  }
  void putInt(bigInt_t const printMe)
  {
    bigInt_t val = printMe;
    int cn = 0;
    if (val < 0)
    {
      putChar('-');
      val *= -1;
    }
    for (bigInt_t _val = 1; _val <= val; _val *= 10)
    {
      cn++;
    }
    do
    {
      putDigit(((10 * val) / pow10(cn)) % 10);
    } while (--cn > 0);
  }
  void putDouble(double const printMe, int const afters_dot)
  {
    double val = printMe;
    if (printMe < 0.0)
    {
      putChar('-');
      val *= -1;
    }
    if (afters_dot > 0)
    {
      bigInt_t valN = val;
      bigInt_t valF = val * pow10(afters_dot);
      int cn = afters_dot;
      putInt(valN);
      putChar('.');
      do
      {
        putDigit(((10 * valF) / pow10(cn)) % 10);
      } while (--cn > 0);
    }
    else
    {
      bigInt_t valE = pow10(- afters_dot);
      bigInt_t valN = round((double)val / (double)valE);
      valN *= valE;
      putInt(valN);
    }
  }
  void putString(char const *const str)
  {
    for (char const *p_ch = str; *p_ch != '\0'; p_ch++)
    {
      if (cnt < Capacity)
      {
        buf[cnt++] = *p_ch;
      }
      else
      {
        break;
      }
    }
  }
};

#endif
