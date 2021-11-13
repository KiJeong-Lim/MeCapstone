/* 2021-11-13 <CAPSTONE PROJECT>
** ===============================================================================
** MEMBERS       | AFFILIATION                                                   |
** ===============================================================================
** Hwan-hee Jeon | School of Mechanical Engineering, Chonnam National University |
** Hak-jung Im   | School of Mechanical Engineering, Chonnam National University |
** Ki-jeong Lim  | School of Mechanical Engineering, Chonnam National University |
** ===============================================================================
*/

#include "Capstone.h"

static bigInt_t pow10(int const n)
{
  bigInt_t res = 1;
  for (int i = 0; i < n; i++)
  {
    res *= 10;
  }
  return res;
}

void BufferWithFormat::memzero()
{
  for (cnt = 0; cnt < LCD_SECTION_LEN; cnt++)
  {
    buf[cnt] = ' ';
  }
  buf[cnt] = '\0';
  cnt = 0;
}

void BufferWithFormat::memsend(char *p_ch)
{
  for (cnt = 0; cnt < LCD_SECTION_LEN; cnt++)
  {
    *p_ch++ = buf[cnt];
  }
}

void BufferWithFormat::putChar(char const ch)
{
  if (cnt < LCD_SECTION_LEN)
  {
    buf[cnt++] = ch;
  }
}

void BufferWithFormat::putDigit(int const d)
{
  putChar("0123456789ABCDEF"[d]);
}

void BufferWithFormat::putInt(bigInt_t const printMe)
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

void BufferWithFormat::putDouble(double const printMe, int const afters_dot)
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
    bigInt_t valN = val;
    bigInt_t valE = pow10(- afters_dot);
    valN /= valE;
    valN *= valE;
    putInt(valN);
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
