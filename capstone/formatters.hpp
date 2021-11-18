/* <CAPSTONE PROJECT>
** ===============================================================================
** MEMBER        | AFFILIATION                                                   |
** ===============================================================================
** Hwan-hee Jeon | School of Mechanical Engineering, Chonnam National University |
** Hak-jung Im   | School of Mechanical Engineering, Chonnam National University |
** Ki-jeong Lim  | School of Mechanical Engineering, Chonnam National University |
** ===============================================================================
*/

#ifndef FORMATTERS
#define FORMATTERS

#include "capstone.h"

template <size_t Capacity>
class SizedFormatter {
  int cnt = 0;
  char buf[Capacity] = { };
public:
  void clear()
  {
    for (cnt = 0; cnt < Capacity; cnt++)
    {
      buf[cnt] = ' ';
    }
    cnt = 0;
  }
  void send(char *const target_address)
  {
    if (target_address)
    {
      memcpy(target_address, &buf[0], sizeof(buf));
    }
  }
  void putChar(char const printMe)
  {
    if (cnt < Capacity)
    {
      buf[cnt++] = printMe;
    }
  }
  void putDigit(int const printMe)
  {
    if (printMe >= 0 && printMe < 16)
    {
      putChar("0123456789ABCDEF"[printMe]);
    }
  }
  void putInt(bigInt_t const printMe, int const base)
  {
    int cn = 0;
    bigInt_t val = printMe;
    if (val < 0)
    {
      putChar('-');
      val *= -1;
    }
    for (bigInt_t _val = 1; _val <= val; _val *= base)
    {
      cn++;
    }
    do
    {
      putDigit(((base * val) / pown(base, cn)) % base);
    } while (--cn > 0);
  }
  void putDouble(double const printMe, int const afters_dot)
  {
    constexpr int base = 10;
    double val = printMe;
    if (printMe < 0.0)
    {
      putChar('-');
      val *= -1;
    }
    if (afters_dot > 0)
    {
      int cn = afters_dot;
      bigInt_t pow10_afters_dot = pown(base, afters_dot);
      bigInt_t valN = ROUND(val * pow10_afters_dot);
      bigInt_t valF = valN % pow10_afters_dot;
      valN /= pow10_afters_dot;
      putInt(valN, base);
      putChar('.');
      do
      {
        putDigit(((base * valF) / pown(base, cn)) % base);
      } while (--cn > 0);
    }
    else
    {
      bigInt_t valE = pown(base, - afters_dot);
      bigInt_t valN = ROUND(val / ((double)valE));
      valN *= valE;
      putInt(valN, base);
    }
  }
  void putString(char const *const printMe)
  {
    if (printMe)
    {
      for (char const *p_ch = printMe; *p_ch != '\0'; p_ch++)
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
  }
};

#endif
