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
#define LENGTH_OF(ary)    (sizeof(ary) / sizeof(*(ary)))
#define ROUND(val)        ((bigInt_t)((val) + 0.5))
#define Apin(pin_no)      (A##pin_no)
#define Dpin(pin_no)      (pin_no)

// type synonym defns
typedef int long long ms_t;
typedef double A_t;
typedef double V_t;
typedef double Ohm_t;
typedef double long mAh_t;
typedef double Val_t;
typedef uint8_t pinId_t;
typedef int long long bigInt_t;

// implemented in "utilities.ino"
bigInt_t POW(bigInt_t base, int expn);
class Timer {
  ms_t volatile begTime;
public:
  Timer();
  Timer(Timer const &other) = delete;
  Timer(Timer &&other) = delete;
  ~Timer();
  void reset();
  ms_t time() const;
  void delay(ms_t duration) const;
};
class AscMap {
  double const left_bound_of_xs;
  double const right_bound_of_xs;
  double const *const ys;
  int const number_of_intervals;
public:
  AscMap() = delete;
  AscMap(AscMap const &other) = delete;
  AscMap(AscMap &&other) = delete;
  template <size_t size_of_data_sheet>
  AscMap(double const (*const data_sheet_ref)[size_of_data_sheet], double const left_bound, double const right_bound)
    : left_bound_of_xs{ left_bound }
    , right_bound_of_xs{ right_bound }
    , ys{ *data_sheet_ref }
    , number_of_intervals{ static_cast<int>(size_of_data_sheet) - 1 }
  {
  }
  ~AscMap()
  {
  }
private:
  double get_x_from_parameter(double param) const;
public:
  double get_x_from_y(double y) const;
};

// implemented in "printers.ino"
LiquidCrystal_I2C *openLcdI2C(int lcd_screen_width, int lcd_screen_height);
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
      putDigit(((base * val) / POW(base, cn)) % base);
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
      bigInt_t pow10_afters_dot = POW(base, afters_dot);
      bigInt_t valN = ROUND(val * pow10_afters_dot);
      bigInt_t valF = valN % pow10_afters_dot;
      valN /= pow10_afters_dot;
      putInt(valN, base);
      putChar('.');
      do
      {
        putDigit(((base * valF) / POW(base, cn)) % base);
      } while (--cn > 0);
    }
    else
    {
      bigInt_t valE = POW(base, - afters_dot);
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
extern SerialPrinter sout, serr, slog;

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
  Val_t readSignal(ms_t duration) const;
};
class PinSetter : public PinHandler {
  bool volatile is_high;
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
  ~PwmSetter();
private:
  void openPin() const;
public:
  void init() const;
  void set(double duty_ratio) const;
};

// implemented in "data.ino"
extern AscMap const mySocOcvTable, mySocVcellTable;

// implemented in "capstone.ino"
struct PinsOfCell {
  PinReader const voltage_sensor_pin;
  PinSetter const BalanceCircuit_pin;
};
struct ReferenceCollection {
  Val_t const analogSignalMax;
  V_t const arduinoRegularV;
  mAh_t const batteryCapacity;
  Ohm_t const sensitivityOfCurrentSensor;
  V_t const zenerdiodeVfromRtoA;
};

#endif
