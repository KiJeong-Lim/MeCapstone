/* 2021-11-12 <CAPSTONE PROJECT>
** ===============================================================================
** MEMBERS       | AFFILIATION                                                   |
** ===============================================================================
** Hwan-hee Jeon | School of Mechanical Engineering, Chonnam National University |
** Hak-jung Im   | School of Mechanical Engineering, Chonnam National University |
** Ki-jeong Lim  | School of Mechanical Engineering, Chonnam National University |
** ===============================================================================
** Notes
** [1] Do not change the definition of 'ms_t' to some type of unsigned integers.
**     Because it is used to present time-differences which might be negative.
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
#else
#include <stdlib.h>
#include <stdint.h>
#endif

// macro defns
#define SERIAL_PORT                         9600
#define LCD_WIDTH                           16
#define LCD_HEIGHT                          2
#define LCD_SECTION_EA                      2
#define LCD_SECTION_LEN                     (LCD_WIDTH / LCD_SECTION_EA)
#define SENSITIVITY_OF_20A_CURRENT_SENSOR   100.0
#define LENGTH_OF(ARR)                      (sizeof(ARR) / sizeof(*(ARR)))
#define toMilliSeconds(secs)                ((ms_t)(1000 * (secs)))
// #define NO_DEBUGGING
// #define NO_LCD_USE
// #define NOT_CONSIDER_SUPPLY_VOLTAGE
// #define NOT_CONSIDER_SUPPLY_CURRENT
// #define NOT_CONTROL_BALANCE_CIRCUIT

// type synonym defns
typedef int long long ms_t; // type for milliseconds
typedef double A_t; // type for ampere;
typedef double V_t; // type for voltage;
typedef double Ohm_t; // type for ohm;
typedef double Val_t; // type for arduino analog signal
typedef uint8_t pinId_t; // type for arduino pin
typedef int long long bigInt_t; // type for big integer

// class defns
#ifndef NOT_MAIN_INO_FILE
struct ReferenceCollection {
  Val_t analogSignalMax;
  V_t arduinoRegularV;
  V_t zenerdiodeVfromRtoA;
  Ohm_t conversionRatioForCurrentSensor;
};
#endif
#ifndef NOT_MAIN_INO_FILE
struct Pin {
  pinId_t const pinId;
};
#endif
#ifndef NOT_MAIN_INO_FILE
class ReaderAnalogPin : public Pin {
public:
  ReaderAnalogPin() = delete;
  ReaderAnalogPin(pinId_t const pin_no)
    : Pin{ .pinId = pin_no }
  {
  }
  ~ReaderAnalogPin()
  {
  }
private:
  int read_once() const
  {
    return analogRead(pinId);
  }
public:
  Val_t readSignalOnce() const
  {
    Val_t val = read_once();
    return val;
  }
  Val_t readSignal(ms_t const duration) const
  {
    bigInt_t sum_of_vals = 0;
    bigInt_t cnt_of_vals = 0;
    ms_t const beg_time = millis();
    for (ms_t cur_time = beg_time; cur_time - beg_time < duration; cur_time = millis())
    {
      sum_of_vals += read_once();
      cnt_of_vals++;
    }
    return ((double)sum_of_vals) / ((double)cnt_of_vals);
  }
};
#endif
#ifndef NOT_MAIN_INO_FILE
class WriterDigitalPin : public Pin {
  bool is_high;
public:
  WriterDigitalPin() = delete;
  WriterDigitalPin(pinId_t const pin_no)
    : Pin{ .pinId = pin_no }
    , is_high{ false }
  {
  }
  ~WriterDigitalPin()
  {
  }
private:
  void syncPin()
  {
    digitalWrite(pinId, is_high ? HIGH : LOW);
  }
public:
  void initWith(bool const on_is_true)
  {
    is_high = on_is_true;
#ifndef NO_DEBUGGING
    Serial.print("[log] The pin ");
    Serial.print(pinId);
    Serial.print(" is initalized to ");
    Serial.print(is_high ? "HIGH" : "LOW");
    Serial.println(".");
    delay(5);
#endif
    pinMode(pinId, OUTPUT);
    syncPin();
  }
  void turnOn()
  {
    is_high = true;
#ifndef NO_DEBUGGING
    Serial.print("[log] The pin ");
    Serial.print(pinId);
    Serial.println(" set to be HIGH.");
    delay(5);
#endif
    syncPin();
  }
  void turnOff()
  {
    is_high = false;
#ifndef NO_DEBUGGING
    Serial.print("[log] The pin ");
    Serial.print(pinId);
    Serial.println(" set to be LOW.");
    delay(5);
#endif
    syncPin();
  }
  bool isHigh() const
  {
    return is_high;
  }
};
#endif
#ifndef NOT_MAIN_INO_FILE
class PwmDigitalPin : public Pin {
};
#endif
#ifndef NOT_MAIN_INO_FILE
struct CELL {
  ReaderAnalogPin const voltageSensor_pin;
  WriterDigitalPin const balanceCircuit_pin;
};
#endif
#ifndef NO_LCD_USE
class BufferWithFormat {
  int cnt = 0;
  char buf[LCD_SECTION_LEN + 1] = { };
public:
  void ready();
  char const *get();
  void put(char character_being_printed);
  void putDigit(int digit_being_printed);
  void putInt(int value_being_printed);
  void putString(char const *string_being_printed);
  void putDouble(double value_being_printed, int number_of_digits_after_dot);
};
#endif
#ifndef NOT_MAIN_INO_FILE
class LcdPrettyPrinter {
#ifndef NO_LCD_USE
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
private:
  void flush()
  {
    int const c = (section_no / LCD_SECTION_EA) * 1;
    int const r = (section_no % LCD_SECTION_EA) * LCD_SECTION_LEN;
    char const *p_ch = nullptr;
    if (c < LCD_HEIGHT && r < LCD_WIDTH)
    {
      p_ch = fbuf.get();
      for (int j = 0; j < LCD_SECTION_LEN; j++)
      {
        buffer[c][r + j] = p_ch[j];
      }
    }
    section_no++;
    fbuf.ready();
  }
public:
  void print(int const value)
  {
    fbuf.putInt(value);
  }
  void print(double const value)
  {
    fbuf.putDouble(value, 2);
  }
  void print(char const *const string)
  {
    fbuf.putString(string);
  }
  void println(int const value)
  {
    fbuf.putInt(value);
    flush();
  }
  void println(double const value)
  {
    fbuf.putDouble(value, 2);
    flush();
  }
  void println(char const *const string)
  {
    fbuf.putString(string);
    flush();
  }
#endif
};
#endif
#ifndef NOT_MAIN_INO_FILE
class Timer {
  ms_t beg_time = 0;
public:
  Timer()
  {
    beg_time = millis();
  }
  ~Timer()
  {
  }
private:
  ms_t getTimeDiff(ms_t const currentTime) const
  {
    return currentTime - beg_time;
  }
public:
  ms_t getDuration()
  {
    ms_t const currentTime = millis();
    ms_t const result = getTimeDiff(currentTime);
    beg_time = currentTime;
    return result;
  }
};
#endif

// global variable decls
#ifndef NOT_MAIN_INO_FILE
extern ReferenceCollection const refOf;
#endif

#endif
