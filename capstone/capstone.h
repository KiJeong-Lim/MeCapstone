/* <CAPSTONE PROJECT>
** ===============================================================================
** MEMBER        | AFFILIATION                                                   |
** ===============================================================================
** Hwan-hee Jeon | School of Mechanical Engineering, Chonnam National University |
** Hak-jung Im   | School of Mechanical Engineering, Chonnam National University |
** Ki-jeong Lim  | School of Mechanical Engineering, Chonnam National University |
** ===============================================================================
*/

#ifndef CAPSTONE
#define CAPSTONE

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define MAJOR_VERSION     0
#define MINOR_VERSION     4
#define REVISION_NUMBER   0
#include "version.h"

// macro defns
#define LCD_SECTION_LEN   (LCD_WIDTH / LCD_SECTION_EA)
#define LENGTH_OF(array)  (sizeof(array) / sizeof(*(array)))
#define ROUND(val)        ((bigInt_t)((val) + 0.5))

// type synonym defns
typedef int long long millis_t;
typedef double A_t;
typedef double V_t;
typedef double Ohm_t;
typedef double Val_t;
typedef uint8_t pinId_t;
typedef int long long bigInt_t;

// implemented in "utilities.ino"
bigInt_t pown(bigInt_t base, int expn);
LiquidCrystal_I2C *openLcdI2C(int lcd_screen_width, int lcd_screen_height);
class Timer {
  millis_t volatile begTime;
public:
  Timer();
  Timer(Timer const &other) = delete;
  Timer(Timer &&other) = delete;
  ~Timer();
  void reset();
  millis_t time() const;
  void delay(millis_t duration) const;
};
class AscMap {
  double const left_bound_of_xs;
  double const right_bound_of_xs;
  double const *const ys;
  size_t const size_of_ys;
public:
  AscMap() = delete;
  AscMap(AscMap const &other) = delete;
  AscMap(AscMap &&other) = delete;
  AscMap(double const *ys, double left_bound_of_xs, size_t size_of_ys, double right_bound_of_xs);
  ~AscMap();
private:
  double get_x_from_parameter(double param) const;
public:
  double get_x_from_y(double y) const;
};

// implemented in "printers.ino"
#include "formatters.hpp"
class LcdPrinter {
  LiquidCrystal_I2C *const lcdHandle;
  int section_no;
  SizedFormatter<LCD_SECTION_LEN> fbuf;
  char mybuf[LCD_HEIGHT][LCD_WIDTH + 1];
public:
  LcdPrinter() = delete;
  LcdPrinter(LcdPrinter const &other) = delete;
  LcdPrinter(LcdPrinter &&other) = delete;
  LcdPrinter(LiquidCrystal_I2C *addressOfLcdI2C);
  ~LcdPrinter();
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
  char const *const prefix_of_message;
  bool newline;
public:
  SerialPrinter() = delete;
  SerialPrinter(SerialPrinter const &other) = delete;
  SerialPrinter(SerialPrinter &&other);
  SerialPrinter(char const *prefix);
  SerialPrinter(char const *prefix, bool lend);
  ~SerialPrinter();
private:
  void trick();
public:
  SerialPrinter operator<<(byte const &hex);
  SerialPrinter operator<<(int const &num);
  SerialPrinter operator<<(char const *const &str);
  SerialPrinter operator<<(double const &val);
};

// implemented in "pinhandlers.ino"
struct PinHandler {
  pinId_t const pin_to_handle;
};
class PinReader : public PinHandler {
public:
  PinReader() = delete;
  PinReader(PinReader const &other) = delete;
  PinReader(PinReader &&other) = delete;
  PinReader(pinId_t pinId);
  ~PinReader();
private:
  int read_once() const;
public:
  Val_t readSignalOnce() const;
  Val_t readSignal(millis_t duration) const;
};
class PinSetter : public PinHandler {
  bool is_high;
public:
  PinSetter() = delete;
  PinSetter(PinSetter const &other) = delete;
  PinSetter(PinSetter &&other) = delete;
  PinSetter(pinId_t pinId);
  ~PinSetter();
private:
  void openPin() const;
  void syncPin();
public:
  void initWith(bool is_high);
  void turnOn();
  void turnOff();
  bool isHigh() const;
};
class PwmSetter : public PinHandler {
public:
  PwmSetter() = delete;
  PwmSetter(PwmSetter const &other) = delete;
  PwmSetter(PwmSetter &&other) = delete;
  PwmSetter(pinId_t pinId);
private:
  void openPin() const;
public:
  void init() const;
  void set(double duty_ratio) const;
};

// implemented in "data.ino"
extern AscMap const mySocVcellTable, mySocOcvTable;

// implemented in "capstone.ino"
struct PinsOfCell {
  PinReader const voltage_sensor_pin;
  PinSetter const BalanceCircuit_pin;
};
struct ReferenceCollection {
  Val_t const analogSignalMax;
  V_t const arduinoRegularV;
  V_t const zenerdiodeVfromRtoA;
  Ohm_t const sensitivityOfCurrentSensor;
};
extern SerialPrinter sout, serr, slog;

#endif
