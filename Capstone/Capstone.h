/* 2021-11-12 <CAPSTONE PROJECT>
** ===============================================================================
** MEMBERS       | AFFILIATION                                                   |
** ===============================================================================
** Hwan-hee Jeon | School of Mechanical Engineering, Chonnam National University |
** Hak-jung Im   | School of Mechanical Engineering, Chonnam National University |
** Ki-jeong Lim  | School of Mechanical Engineering, Chonnam National University |
** ===============================================================================
*/

#ifndef CAPSTONE
#define CAPSTONE

#ifndef BMS_VERSION
#define NOT_MAIN_INO_FILE
#endif

// includes
#ifndef NOT_MAIN_INO_FILE
#include <Wire.h>
#include "LiquidCrystal_I2C.h"
#endif

// macro defns
#define SERIAL_PORT                         9600
#define LCD_WIDTH                           16
#define LCD_HEIGHT                          2
#define LCD_SECTION_EA                      2
#define LCD_SECTION_LEN                     (LCD_WIDTH / LCD_SECTION_EA)
#define SENSITIVITY_OF_20A_CURRENT_SENSOR   100.0
#define LENGTH_OF(ARR)                      (sizeof(ARR) / sizeof(*(ARR)))
// #define NO_DEBUGGING
// #define NO_LCD_USE

// type synonym defns
typedef int long long ms_t;
typedef double A_t;
typedef double V_t;
typedef double Ohm_t;
typedef double Val_t;

// class defns
struct ReferenceCollection {
  Val_t analogSignalMax;
  V_t arduinoRegularV;
  V_t zenerdiodeVfromRtoA;
  Ohm_t conversion_ratio_for_ampere_sensor;
};
#ifndef NOT_MAIN_INO_FILE
struct Pin {
  int const pinId;
};
class ReaderAnalogPin : public Pin {
public:
  ReaderAnalogPin() = delete;
  ReaderAnalogPin(int const pin_no)
    : Pin{ .pinId = pin_no }
  {
  }
  Val_t readSignalOnce() const
  {
    Val_t val = analogRead(pinId);
    return val;
  }
  Val_t readSignal(ms_t const duration) const
  {
    int long long sum_of_vals = 0;
    int long cnt_of_vals = 0;
    ms_t const beg_time = millis();
    for (ms_t cur_time = beg_time; cur_time - beg_time < duration; cur_time = millis())
    {
      sum_of_vals += analogRead(pinId);
      cnt_of_vals++;
    }
    return ((double)sum_of_vals) / ((double)cnt_of_vals);
  }
};
class WriterDigitalPin : public Pin {
  bool is_high;
public:
  WriterDigitalPin() = delete;
  WriterDigitalPin(int const pin_no)
    : Pin{ .pinId = pin_no }
    , is_high{ false }
  {
  }
  void initWith(bool const on_is_true)
  {
    pinMode(pinId, OUTPUT);
    digitalWrite(pinId, on_is_true);
    is_high = on_is_true;
#ifndef NO_DEBUGGING
    Serial.print("[log] The pin ");
    Serial.print(pinId);
    Serial.print(" is initalized to ");
    Serial.print(on_is_true ? "HIGH" : "LOW");
    Serial.println(".");
#endif
  }
  void turnOn()
  {
    digitalWrite(pinId, HIGH);
    is_high = true;
#ifndef NO_DEBUGGING
    Serial.print("[log] The pin ");
    Serial.print(pinId);
    Serial.println(" set to be HIGH.");
#endif
  }
  void turnOff()
  {
    digitalWrite(pinId, LOW);
    is_high = false;
#ifndef NO_DEBUGGING
    Serial.print("[log] The pin ");
    Serial.print(pinId);
    Serial.println(" set to be LOW.");
#endif
  }
  bool isHigh() const
  {
    return is_high;
  }
};
struct CELL {
  ReaderAnalogPin const voltageSensor_pin;
  WriterDigitalPin const balanceCircuit_pin;
};
#endif
class BufferWithFormat {
  int cnt = 0;
  char buf[LCD_SECTION_LEN + 1] = {};
public:
  void ready();
  char const *get();
  void put(char character_being_printed);
  void putDigit(int digit_being_printed);
  void putInt(int value_being_printed);
  void putString(char const *string_being_printed);
  void putDouble(double value_being_printed, int number_of_digits_after_dot);
};
#ifndef NOT_MAIN_INO_FILE
class LcdPrettyPrinter {
  LiquidCrystal_I2C *const lcd_handle;
  int section_no;
  BufferWithFormat fbuf;
  char buffer[LCD_HEIGHT][LCD_WIDTH + 1];
public:
  LcdPrettyPrinter() = delete;
  LcdPrettyPrinter(LiquidCrystal_I2C *const controllerOfLCD)
    : lcd_handle{ controllerOfLCD }
    , section_no{ 0 }
    , fbuf{ }
    , buffer{ }
  {
    for (int c = 0; c < LENGTH_OF(buffer); c++)
    {
      for (int r = 0; r < LENGTH_OF(*buffer); r++)
      {
        buffer[c][r] = '\0';
      }
    }
    fbuf.ready();
  }
  ~LcdPrettyPrinter()
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
  void print(char const * string);
  void println(int value);
  void println(double value);
  void println(char const *string);
private:
  void flush();
};
#endif

// global variable decls
extern ReferenceCollection const refOf;

#endif
