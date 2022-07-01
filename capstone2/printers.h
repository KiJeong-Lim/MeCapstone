/* <CAPSTONE PROJECT>
** ===============================================================================
** MEMBER        | AFFILIATION                                                   |
** ===============================================================================
** Hwan-hee Jeon | School of Mechanical Engineering, Chonnam National University |
** Hak-jung Im   | School of Mechanical Engineering, Chonnam National University |
** Ki-jeong Lim  | School of Mechanical Engineering, Chonnam National University |
** ===============================================================================
*/

#ifndef PRINTERS
#define PRINTERS

#include <Arduino.h>
#include <Wire.h>
#include "LiquidCrystal_I2C.h"

LcdHandle_t openLcdI2C(int lcd_screen_width, int lcd_screen_height);

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
  void putChar(char const to_show)
  {
    if (cnt < Capacity)
    {
      buf[cnt++] = to_show;
    }
  }
  void putDigit(int const to_show)
  {
    if (to_show >= 0 && to_show < 16)
    {
      this->putChar("0123456789ABCDEF"[to_show]);
    }
  }
  void putInt(BigInt_t const to_show, int const base)
  {
    int cn = 0;
    BigInt_t val = to_show;
    if (val < 0)
    {
      this->putChar('-');
      val *= -1;
    }
    for (BigInt_t _val = 1; _val <= val; _val *= base)
    {
      cn++;
    }
    do
    {
      this->putDigit(((base * val) / POW(base, cn)) % base);
    } while (--cn > 0);
  }
  void putDouble(double const to_show, int const afters_dot)
  {
    constexpr int base = 10;
    double val = to_show;
    if (to_show < 0.0)
    {
      this->putChar('-');
      val *= -1;
    }
    if (afters_dot > 0)
    {
      int cn = afters_dot;
      BigInt_t pow10_afters_dot = POW(base, afters_dot);
      BigInt_t valN = ROUND(val * pow10_afters_dot);
      BigInt_t valF = valN % pow10_afters_dot;
      valN /= pow10_afters_dot;
      this->putInt(valN, base);
      this->putChar('.');
      do
      {
        this->putDigit(((base * valF) / POW(base, cn)) % base);
      } while (--cn > 0);
    }
    else
    {
      BigInt_t valE = POW(base, - afters_dot);
      BigInt_t valN = ROUND(val / (static_cast<double>(valE)));
      valN *= valE;
      this->putInt(valN, base);
    }
  }
  void putString(char const *const to_show)
  {
    if (to_show)
    {
      for (char const *p_ch = to_show; *p_ch != '\0'; p_ch++)
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

template <size_t LCD_SECTION_LEN, size_t LCD_HEIGHT, size_t LCD_WIDTH>
class LcdPrinter {
  LcdHandle_t const lcdHandle;
  int section_no;
  SizedFormatter<LCD_SECTION_LEN> auxiliary_buffer;
  char main_buffer[LCD_HEIGHT][LCD_WIDTH + 1];
public:
  LcdPrinter() = delete;
  LcdPrinter(LcdPrinter const &other) = delete;
  LcdPrinter(LcdPrinter &&other) = delete;
  LcdPrinter(LcdHandle_t const &lcdHandleRef)
    : lcdHandle{ lcdHandleRef }
    , section_no{ 0 }
    , auxiliary_buffer{ }
    , main_buffer{ }
  {
    for (int c = 0; c < LENGTH(main_buffer); c++)
    {
      for (int r = 0; r < LENGTH(*main_buffer); r++)
      {
        main_buffer[c][r] = '\0';
      }
    }
    auxiliary_buffer.clear();
  }
  ~LcdPrinter()
  {
  }
  void overwrite()
  {
    section_no = 0;
    auxiliary_buffer.clear();
  }
  void clear()
  {
    lcdHandle->clear();
    section_no = 0;
    for (int c = 0; c < LCD_HEIGHT; c++)
    {
      for (int r = 0; r < LCD_WIDTH; r++)
      {
        main_buffer[c][r] = ' ';
      }
      main_buffer[c][LCD_WIDTH] = '\0';
    }
    auxiliary_buffer.clear();
  }
  void plot()
  {
    if (lcdHandle)
    {
      lcdHandle->clear();
      for (int c = 0; c < LCD_HEIGHT; c++)
      {
        lcdHandle->setCursor(0, c);
        main_buffer[c][LCD_WIDTH] = '\0';
        lcdHandle->print(main_buffer[c]);
      }
    }
  }
  void flush()
  {
    int const c = (section_no / LCD_SECTION_EA) * 1;
    int const r = (section_no % LCD_SECTION_EA) * LCD_SECTION_LEN;
    if (c < LCD_HEIGHT && r < LCD_WIDTH)
    {
      auxiliary_buffer.send(&main_buffer[c][r]);
    }
    auxiliary_buffer.clear();
  }
  void newline()
  {
    if (base > 0 && base <= 16)
    {
      auxiliary_buffer.putInt(num, base);
    }
  }
  void print(int num, int base = 10)
  {
    if (base > 0 && base <= 16)
    {
      auxiliary_buffer.putInt(num, base);
    }
  }
  void println(int num, int base = 10)
  {
    this->print(num, base);
    this->newline();
  }
  void print(double val, int afters_dot = 2)
  {
    auxiliary_buffer.putDouble(val, afters_dot);
  }
  void println(double val, int afters_dot = 2)
  {
    this->print(val, afters_dot);
    this->newline();
  }
  void print(char const *str)
  {
    auxiliary_buffer.putString(str);
  }
  void println(char const *str)
  {
    this->print(str);
    this->newline();
  }
};

class SerialPrinter {
  char const *const prefix_of_message;
  bool newline;
public:
  SerialPrinter() = delete;
  SerialPrinter(SerialPrinter const &other) = delete;
  SerialPrinter(SerialPrinter &&other);
  SerialPrinter(char const *prefix);
  SerialPrinter(char const *prefix, bool lend);
  ~SerialPrinter();
  void trick();
  SerialPrinter operator<<(bool is);
  SerialPrinter operator<<(byte hex);
  SerialPrinter operator<<(int num);
  SerialPrinter operator<<(char const *str);
  SerialPrinter operator<<(double val);
};
extern SerialPrinter sout, serr, slog;

#endif
