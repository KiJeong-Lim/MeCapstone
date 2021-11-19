/* <CAPSTONE PROJECT>
** ===============================================================================
** MEMBER        | AFFILIATION                                                   |
** ===============================================================================
** Hwan-hee Jeon | School of Mechanical Engineering, Chonnam National University |
** Hak-jung Im   | School of Mechanical Engineering, Chonnam National University |
** Ki-jeong Lim  | School of Mechanical Engineering, Chonnam National University |
** ===============================================================================
*/

// include-guard
#ifndef CAPSTONE
#define CAPSTONE

// required libraries
#include <Arduino.h>
#include <Wire.h>
#include "LiquidCrystal_I2C.h"

// version information
#define MAJOR_VERSION     0
#define MINOR_VERSION     4
#define REVISION_NUMBER   1
#include "version.h"

// macro defns
#define LCD_SECTION_LEN   ((LCD_WIDTH) / (LCD_SECTION_EA))
#define LENGTH(ary)       (sizeof(ary) / sizeof(*(ary)))
#define ROUND(val)        ((BigInt_t)((val) + 0.5))
#define Apin(pin_no)      A##pin_no
#define Dpin(pin_no)      pin_no
/* Comments
** [LCD_SECTION_LEN]
** 1. `LCD_SECTION_LEN` returns the length of sections.
** [LENGTH]
** 1. `LENGTH(ary)` returns the number of elements of `ary`.
** [ROUND]
** 1. `ROUND(val)` returns the rounding of `val`.
** [Apin]
** 1. `Apin` stands for analog pin.
** 2. For example, `Apin(0)` refers to the analog pin `A0`.
** [Dpin]
** 1. `Dpin` stands for digital pin.
** 2. For example, `Dpin(2)` refers to the digital pin `2`.
*/

// type synonym defns
typedef int long long ms_t;
typedef double Amp_t;
typedef double Vol_t;
typedef double Ohm_t;
typedef double long mAh_t;
typedef double Val_t;
typedef uint8_t pinId_t;
typedef int64_t BigInt_t;
typedef LiquidCrystal_I2C *LcdHandle_t;
/* Comments
** [ms_t]
** 1. `ms_t` stands for the type of milliseconds.
** [Amp_t]
** 1. `Amp_t` stands for the type of ampere.
** [Vol_t]
** 1. `Vol_t` stands for the type of voltage.
** [Ohm_t]
** 1. `Ohm_t` stands for the type of ohm.
** [mAh_t]
** 1. `mAh_t` stands for the type of milliampere hours.
** [Val_t]
** 1. `Val_t` stands for the type of the real numbers.
** [pinId_t]
** 1. `pinId_t` stands for the type of pins.
** [BigInt_t]
** 1. `BigInt_t` stands for the type of integers between `-9223372036854775807LL - 1` and `9223372036854775807LL`.
** [LcdHandle_t]
** 1. `LcdHandle_t` stands for the type of pointers of `LiquidCrystal_I2C`.
*/

// implemented in "utilities.cpp"
BigInt_t POW(BigInt_t base, int expn);
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
  ~AscMap();
private:
  double get_x_from_parameter(double param) const;
public:
  double get_x_from_y(double y) const;
};
/* Comments
** [POW]
** 1. Usage
**    > y = POW(x, n);
**    - Requirements
**      [A] x > 0
**      [B] n >= 0
**      [C] x^n =< 9223372036854775807
**    - Guarantees
**      [A] y = x^n
** [Timer]
** [AscMap]
*/

// implemented in "printers.cpp"
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
  void putInt(BigInt_t const printMe, int const base)
  {
    int cn = 0;
    BigInt_t val = printMe;
    if (val < 0)
    {
      putChar('-');
      val *= -1;
    }
    for (BigInt_t _val = 1; _val <= val; _val *= base)
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
      BigInt_t pow10_afters_dot = POW(base, afters_dot);
      BigInt_t valN = ROUND(val * pow10_afters_dot);
      BigInt_t valF = valN % pow10_afters_dot;
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
      BigInt_t valE = POW(base, - afters_dot);
      BigInt_t valN = ROUND(val / ((double)valE));
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
  LcdHandle_t const lcdHandle;
  int section_no;
  SizedFormatter<LCD_SECTION_LEN> fbuf;
  char mybuf[LCD_HEIGHT][LCD_WIDTH + 1];
public:
  LcdPrinter() = delete;
  LcdPrinter(LcdPrinter const &other) = delete;
  LcdPrinter(LcdPrinter &&other) = delete;
  LcdPrinter(LcdHandle_t const &lcdHandleRef);
  ~LcdPrinter();
private:
  void newline();
public:
  void print(int num, int base = 10);
  void println(int num, int base = 10);
  void print(double val, int afters_dot = 2);
  void println(double val, int afters_dot = 2);
  void print(char const *str);
  void println(char const *str);
  void flush();
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
  SerialPrinter operator<<(byte hex);
  SerialPrinter operator<<(int num);
  SerialPrinter operator<<(char const *str);
  SerialPrinter operator<<(double val);
};
extern SerialPrinter sout, serr, slog;
/* Comments
** [openLcdI2C]
** 1. Usage
**    > lcdHandle = openLcdI2C(lcd_screen_width, lcd_screen_height);
**    - Requirements
**      [A] `Wire.begin();` must be executed before calling this function.
**      [B] lcd_screen_width > 0
**      [C] lcd_screen_height > 0
**      [D] If the arduino board is Uno,
**          then: I2C::SDA must be connected Uno::A4;
**                I2C::SCL must be connected Uno::A5;
**                I2C::VCC must be connected Uno::5V; and
**                I2C::GND must be connected Uno::GND.
**      [E] If the arduino board is Nano,
**          then: I2C::SDA must be connected Nano::A4;
**                I2C::SCL must be connected Nano::A5;
**                I2C::VCC must be connected Nano::5V; and
**                I2C::GND must be connected Nano::GND.
**   - Guarantees
**     [A] If `lcdHandle` is not a null-pointer,
**         the screen of an LCD is initialized,
**         which is being handled by `lcdHandle`.
** 2. References
**    [1] https://codingrun.com/119
**    [2] https://m.blog.naver.com/hy10101010/221562445464
** [SizedFormatter]
** [LcdPrinter]
** [SerialPrinter]
** [sout]
** 1. `sout` stands for serial output.
** [serr]
** 1. `serr` stands for serial error.
** [slog]
** 1. `slog` stands for serial logger.
*/

// implemented in "pinhandlers.cpp"
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
  void initWith(bool be_high);
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
/* Comments
** [PinHandler]
** [PinReader]
** [PinSetter]
** [PwmSetter]
*/

// implemented in "data.cpp"
extern AscMap const mySocOcvTable, mySocVcellTable;
/* Comments
** [mySocOcvTable]
** 1. A table which maps `soc` to `ocv`,
**    where `0.00 =< soc =< 100.00`.
** 2. Usage
**    > soc = mySocOcvTable.get_x_from_y(ocv);
**    - Guarantees
**      [A] 0.00 =< soc =< 100.00
** [mySocVcellTable]
** 1. A table which maps `soc` to `Vcell`,
**    where `0.00 =< soc =< 98.00`.
** 2. Usage
**    > soc = mySocVcellTable.get_x_from_y(Vcell);
**    - Guarantees
**      [A] 0.00 =< soc =< 98.00
*/

// implemented in "capstone.ino"
struct PinsOfCell {
  PinReader const voltage_sensor_pin;
  PinSetter const BalanceCircuit_pin;
};
struct ReferenceCollection {
  Val_t const analogSignalMax;
  Vol_t const arduinoRegularV;
  mAh_t const batteryCapacity;
  Ohm_t const sensitivityOfCurrentSensor;
  Vol_t const zenerdiodeVfromRtoA;
};
/* Comments
** [PinsOfCell]
** 1. A class whose instances consist of pins associated with a cell.
** [ReferenceCollection]
** 1. A class, each instance of which is a collection of value references.
*/

#endif
