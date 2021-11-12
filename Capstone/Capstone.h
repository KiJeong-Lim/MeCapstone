/* 2021-11-12 <CAPSTONE PROJECT>
** ===============================================================================
** MEMBERS       | AFFILIATION                                                   |
** ===============================================================================
** Hwan-hee Jeon | School of Mechanical Engineering, Chonnam National University |
** Hak-jung Im   | School of Mechanical Engineering, Chonnam National University |
** Ki-jeong Lim  | School of Mechanical Engineering, Chonnam National University |
** ===============================================================================
*/

#ifndef CAPSTONE_VERSION

// includes
#ifndef PURE_CPP
#include <Wire.h>
#include "LiquidCrystal_I2C.h"
#endif

// macro-defs
#define CAPSTONE_VERSION                    "1.0.0"
#define SERIAL_PORT                         9600
#define LCD_WIDTH                           16
#define LCD_HEIGHT                          2
#define LCD_SECTION_EA                      2
#define LCD_SECTION_LEN                     (LCD_WIDTH / LCD_SECTION_EA)
#define SENSITIVITY_OF_20A_AMPERE_SENSOR    100.0
#define LENGTH_OF(ARR)                      (sizeof(ARR) / sizeof(*(ARR)))

// type-defs
typedef int long long ms_t;
typedef double A_t;
typedef double V_t;
typedef double Ohm_t;
typedef double Val_t;

// inline-defs
#ifndef PURE_CPP
inline void printHexOnSerial(byte const integer_between_0_and_255)
{
#ifndef NO_DEBUGGING
  Serial.print("0x");
  if (integer_between_0_and_255 < 16)
  {
    Serial.print("0");
  }
  Serial.print(integer_between_0_and_255, HEX);
#endif
}
#endif

// class-defs
class SectionBuffer {
  int cnt = 0;
  char line[LCD_SECTION_LEN + 1] = {};
public:
  void ready();
  char const *get();
  void put(char character_being_printed);
  void putDigit(int digit_being_printed);
  void putInt(int value_being_printed);
  void putString(char const *string_being_printed);
  void putDouble(double value_being_printed, int number_of_digits_after_dot);
};
#ifndef PURE_CPP
struct Pin {
  int const pin_no;
};
class ReaderAnalogPin : public Pin {
  ReaderAnalogPin() = delete;
public:
  ReaderAnalogPin(int const pos)
    : Pin{ .pin_no = pos }
  {
  }
  Val_t readSignal(ms_t const duration)
  {
    int long long sum_of_vals = 0;
    int long cnt_of_vals = 0;
    ms_t const beg_time = millis();

    for (ms_t cur_time = beg_time; cur_time - beg_time < duration; cur_time = millis())
    {
      sum_of_vals += analogRead(pin_no);
      cnt_of_vals++;
    }

    return ((double)sum_of_vals) / ((double)cnt_of_vals);
  }
};
class WriterDigitalPin : public Pin {
  WriterDigitalPin() = delete;
public:
  WriterDigitalPin(int pos)
    : Pin{ .pin_no = pos }
  {
  }
  void initWith(bool on_is_true)
  {
    pinMode(pin_no, OUTPUT);
    digitalWrite(pin_no, on_is_true);
#ifndef NO_DEBUGGING
    Serial.print("[log] The pin ");
    Serial.print(pin_no);
    Serial.print(" is initalized to ");
    Serial.print(on_is_true ? "HIGH" : "LOW");
    Serial.println(".");
#endif
  }
  void turnOn()
  {
    digitalWrite(pin_no, HIGH);
  }
  void turnOff()
  {
    digitalWrite(pin_no, LOW);
  }
};
class LcdSplitPrinter {
  LcdSplitPrinter() = delete;
  LiquidCrystal_I2C *const lcd_handle;
  int section_no = 0;
  SectionBuffer line = {};
  char buffer[LCD_HEIGHT][LCD_WIDTH + 1] = {};
public:
  LcdSplitPrinter(LiquidCrystal_I2C *const controllerOfLCD)
    : lcd_handle{ controllerOfLCD }
  {
    for (int c = 0; c < LENGTH_OF(buffer); c++)
    {
      for (int r = 0; r < LENGTH_OF(*buffer); r++)
      {
        buffer[c][r] = '\0';
      }
    }
    line.ready();
  }
  ~LcdSplitPrinter()
  {
    if (lcd_handle)
    {
      lcd_handle->clear();
      for (int c = 0; c < LCD_HEIGHT; c++)
      {
        lcd_handle->setCursor(0, c);
        buffer[c][LCD_WIDTH] = '\0';
        lcd_handle->print(buffer[c]);
      }
    }
  }
  void print(int value);
  void print(double value);
  void print(char const *string);
  void println(int value);
  void println(double value);
  void println(char const *string);
private:
  void flushLine();
};
#endif

#endif
 