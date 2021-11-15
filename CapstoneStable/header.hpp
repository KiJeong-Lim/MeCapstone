/* <CAPSTONE PROJECT>
** ===============================================================================
** MEMBERS       | AFFILIATION                                                   |
** ===============================================================================
** Hwan-hee Jeon | School of Mechanical Engineering, Chonnam National University |
** Hak-jung Im   | School of Mechanical Engineering, Chonnam National University |
** Ki-jeong Lim  | School of Mechanical Engineering, Chonnam National University |
** ===============================================================================
*/

#ifndef HEADER
#define HEADER

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "version.h"

// macro defns
#define LCD_SECTION_LEN                     (LCD_WIDTH / LCD_SECTION_EA)
#define LENGTH_OF(array)                    (sizeof(array) / sizeof(*(array)))
#define round(x)                            ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))

// type synonym defns
typedef int long long ms_t;
typedef double A_t;
typedef double V_t;
typedef double Ohm_t;
typedef double Val_t;
typedef uint8_t pinId_t;
typedef int long long bigInt_t;

// implemented in "utility.ino"
bigInt_t pow10(int expn);
class Timer {
  ms_t curTime;
public:
  Timer();
  Timer(Timer const &other);
  ~Timer();
private:
  void syncTime();
public:
  ms_t &&getDuration();
  bool wait(ms_t given_time);
};
struct Pin {
  pinId_t const pinId;
};
class ReaderAnalogPin : public Pin {
public:
  ReaderAnalogPin() = delete;
  ReaderAnalogPin(ReaderAnalogPin const &other) = delete;
  ReaderAnalogPin(pinId_t pin_no);
  ~ReaderAnalogPin();
private:
  int read_once() const;
public:
  Val_t readSignalOnce() const;
  Val_t readSignal(ms_t duration) const;
};
class WriterDigitalPin : public Pin {
  bool is_high;
public:
  WriterDigitalPin() = delete;
  WriterDigitalPin(WriterDigitalPin const &other) = delete;
  WriterDigitalPin(pinId_t pin_no);
  ~WriterDigitalPin();
private:
  void syncPin();
public:
  void initWith(bool on_is_true);
  void turnOn();
  void turnOff();
  bool isHigh() const;
};
class PwmDigitalPin : public Pin {
public:
  PwmDigitalPin() = delete;
  PwmDigitalPin(PwmDigitalPin const &other) = delete;
  PwmDigitalPin(pinId_t pin_no);
private:
  void setPWM(double duty_ratio) const;
public:
  void init() const;
  void set(double duty_ratio) const;
  void initWith(double duty_ratio) const;
};
struct CELL {
  ReaderAnalogPin const voltageSensor_pin;
  WriterDigitalPin const balanceCircuit_pin;
};

// implemented in "printer.ino"
LiquidCrystal_I2C *openLcdI2C();
#include "formatter.hpp"
class LcdPrettyPrinter {
  LiquidCrystal_I2C *const lcdHandle;
  int section_no;
  Formatter<LCD_SECTION_LEN> fbuf;
  char mybuf[LCD_HEIGHT][LCD_WIDTH + 1];
public:
  LcdPrettyPrinter() = delete;
  LcdPrettyPrinter(LcdPrettyPrinter const &other) = delete;
  LcdPrettyPrinter(LiquidCrystal_I2C *controllerOfLCD);
  ~LcdPrettyPrinter();
private:
  void newline();
public:
  void flush();
  void print(int num);
  void print(double val);
  void print(char const *str);
  void println(int num);
  void println(double val);
  void println(char const *str);
};
class SerialPrinter {
  char const *const messenger;
  bool newline;
public:
  SerialPrinter() = delete;
  SerialPrinter(SerialPrinter const &other) = delete;
  SerialPrinter(SerialPrinter &&other) = delete;
  SerialPrinter(char const *prefix);
  SerialPrinter(char const *prefix, bool lend);
  ~SerialPrinter();
private:
  void print_messenger();
public:
  SerialPrinter &&operator<<(byte const &hex);
  SerialPrinter &&operator<<(int const &num);
  SerialPrinter &&operator<<(char const *const &str);
  SerialPrinter &&operator<<(double const &val);
};

// implemented in "CapstoneStable.ino"
struct ReferenceCollection {
  Val_t const analogSignalMax;
  V_t const arduinoRegularV;
  V_t const zenerdiodeVfromRtoA;
  Ohm_t const sensitivityOfCurrentSensor;
};
extern SerialPrinter const cout, cerr, chan;

#endif