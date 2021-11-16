/* <CAPSTONE PROJECT>
** ===============================================================================
** MEMBER        | AFFILIATION                                                   |
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

#define MAJOR_VERSION    0
#define MINOR_VERSION    2
#define REVISION_NUMBER  0
#include "version.h"

// macro defns
#define LCD_SECTION_LEN     (LCD_WIDTH / LCD_SECTION_EA)
#define LENGTH_OF(array)    (sizeof(array) / sizeof(*(array)))
#define round(val)          ((val) >= 0 ? (bigInt_t)((val) + 0.5) : (bigInt_t)((val) - 0.5))

// type synonym defns
typedef int long long millis_t;
typedef double A_t;
typedef double V_t;
typedef double Ohm_t;
typedef double Val_t;
typedef uint8_t pinId_t;
typedef int long long bigInt_t;

// implemented in "utility.ino"
bigInt_t pow10(int expn);
class Timer {
  millis_t curTime;
public:
  Timer();
  Timer(Timer const &other);
  ~Timer();
private:
  void syncTime();
public:
  millis_t &&getDuration();
  bool wait(millis_t given_time);
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
  Val_t readSignal(millis_t duration) const;
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
class AscMap {
  const double left_bound_of_xs;
  const double right_bound_of_xs;
  const double *const ys;
  size_t const size_of_ys;
public:
  AscMap(const double *ys, double left_bound_of_xs, size_t size_of_ys, double right_bound_of_xs);
  ~AscMap();
private:
  double get_x_from_parameter(double param) const;
public:
  double get_x_from_y(double y) const;
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
  SerialPrinter(SerialPrinter &&other);
  SerialPrinter(char const *prefix);
  SerialPrinter(char const *prefix, bool lend);
  ~SerialPrinter();
private:
  void print_messenger();
public:
  SerialPrinter operator<<(byte const &hex);
  SerialPrinter operator<<(int const &num);
  SerialPrinter operator<<(char const *const &str);
  SerialPrinter operator<<(double const &val);
};

// implemented in "ocv.ino"
V_t getOcvFromVcell(V_t Vcell, A_t Iin);
extern AscMap const mySocOcvTable;

// implemented in "CapstoneStable.ino"
struct ReferenceCollection {
  Val_t const analogSignalMax;
  V_t const arduinoRegularV;
  V_t const zenerdiodeVfromRtoA;
  Ohm_t const sensitivityOfCurrentSensor;
};
extern SerialPrinter cout, cerr, chan;

#endif
