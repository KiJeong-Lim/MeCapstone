#ifndef HEADER
#define HEADER

#include <Wire.h>
#include "LiquidCrystal_I2C.h"
#include "version.h"

// macro defns
#define LCD_WIDTH                           16
#define LCD_HEIGHT                          2
#define LCD_SECTION_EA                      2
#define LCD_SECTION_LEN                     (LCD_WIDTH / LCD_SECTION_EA)
#define SENSITIVITY_OF_20A_CURRENT_SENSOR   0.100
#define LENGTH_OF(array)                    (sizeof(array) / sizeof(*(array)))

// type synonym defns
typedef int long long ms_t;
typedef double A_t;
typedef double V_t;
typedef double Ohm_t;
typedef double Val_t;
typedef uint8_t pinId_t;
typedef int long long bigInt_t;

// Implemented in "utility.ino"
class Timer {
  ms_t curTime;
public:
  Timer();
  Timer(Timer const &other);
  ~Timer();
private:
  void syncTime();
public:
  ms_t getDuration();
  bool wait(ms_t given_time);
};
struct Pin {
  pinId_t const pinId;
};
class ReaderAnalogPin : public Pin {
public:
  ReaderAnalogPin() = delete;
  ReaderAnalogPin(ReaderAnalogPin const &other) = delete;
  ReaderAnalogPin(pinId_t const pin_no);
  ~ReaderAnalogPin();
private:
  int read_once() const;
public:
  Val_t readSignalOnce() const;
  Val_t readSignal(ms_t const duration) const;
};
class WriterDigitalPin : public Pin {
  bool is_high;
public:
  WriterDigitalPin() = delete;
  WriterDigitalPin(WriterDigitalPin const &other) = delete;
  WriterDigitalPin(pinId_t const pin_no);
  ~WriterDigitalPin();
private:
  void syncPin();
public:
  void initWith(bool const on_is_true);
  void turnOn();
  void turnOff();
  bool isHigh() const;
};
class PwmDigitalPin : public Pin {
public:
  PwmDigitalPin() = delete;
  PwmDigitalPin(PwmDigitalPin const &other) = delete;
  PwmDigitalPin(pinId_t const pin_no);
private:
  void setPWM(double const duty_ratio) const;
public:
  void init() const;
  void setPwm(double const duty_ratio) const;
};
struct CELL {
  ReaderAnalogPin const voltageSensor_pin;
  WriterDigitalPin const balanceCircuit_pin;
};

// Implemented in "printer.ino"
class SerialPrinter {
  char const *const messenger;
  bool newline;
public:
  SerialPrinter() = delete;
  SerialPrinter(SerialPrinter const &other) = delete;
  SerialPrinter(char const *prefix);
  SerialPrinter(char const *prefix, bool is_last);
  ~SerialPrinter();
public:
  SerialPrinter &&operator<<(byte const &hex);
  SerialPrinter &&operator<<(int const &num);
  SerialPrinter &&operator<<(char const *const & str);
  SerialPrinter &&operator<<(double const &val);
};
class BufferWithFormat {
  int cnt = 0;
  char buf[LCD_SECTION_LEN + 1] = { };
public:
  void memzero();
  void memsend(char *tgt);
  void putChar(char character_being_printed);
  void putDigit(int digit_being_printed);
  void putInt(bigInt_t value_being_printed);
  void putString(char const *string_being_printed);
  void putDouble(double value_being_printed, int number_of_digits_after_dot);
};
class LcdPrettyPrinter {
  LiquidCrystal_I2C *const lcdHandle;
  int section_no;
  BufferWithFormat fbuf;
  char mybuf[LCD_HEIGHT][LCD_WIDTH + 1];
public:
  LcdPrettyPrinter() = delete;
  LcdPrettyPrinter(LcdPrettyPrinter const &other) = delete;
  LcdPrettyPrinter(LiquidCrystal_I2C *const controllerOfLCD);
  ~LcdPrettyPrinter();
private:
  void flush();
public:
  void print(int const value);
  void print(double const value);
  void print(char const *const string);
  void println(int const value);
  void println(double const value);
  void println(char const *const string);
};
LiquidCrystal_I2C *openLcdI2C(int lcd_width, int lcd_height);

// Implemented in "CapstoneStable.ino"
struct ReferenceCollection {
  Val_t const analogSignalMax;
  V_t const arduinoRegularV;
  V_t const zenerdiodeVfromRtoA;
  Ohm_t const conversionRatioOfCurrentSensor;
};
extern SerialPrinter const consoleLog, alert, shell;
extern ReferenceCollection const refOf;

#endif
