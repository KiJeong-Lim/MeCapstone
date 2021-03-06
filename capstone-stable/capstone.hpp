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
#define MAJOR_VERSION     1
#define MINOR_VERSION     2
#define REVISION_NUMBER   0
#include "version.h"

// macro defns
#define LCD_SECTION_LEN   ((LCD_WIDTH) / (LCD_SECTION_EA))
#define LENGTH(ary)       (sizeof(ary) / sizeof(*(ary)))
#define ROUND(val)        (static_cast<BigInt_t>((val) + 0.5))
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
void invokingSerial();
void drawlineSerial();
BigInt_t POW(BigInt_t base, int expn);
template <typename UnsignedIntegers = byte>
class BitArray {
  UnsignedIntegers my_bits;
public:
  BitArray(UnsignedIntegers const _my_bits)
    : my_bits{ _my_bits }
  {
  }
  BitArray(BitArray const &other)
    : my_bits{ other.my_bits }
  {
  }
  ~BitArray()
  {
  }
  bool get(int8_t const n) const
  {
    return (my_bits & (1u << n));
  }
  BitArray &set(int8_t const n, bool const to_be)
  {
    if (to_be)
    {
      my_bits |= (1u << n);
    }
    else
    {
      my_bits &= ~(1u << n);
    }
    return *this;
  }
  BitArray &operator=(UnsignedIntegers bits)
  {
    my_bits = bits;
    return *this;
  }
};
class Timer {
  ms_t volatile begTime;
public:
  Timer();
  Timer(Timer const &other) = delete;
  Timer(Timer &&other) = delete;
  Timer(ms_t init_time);
  ~Timer();
  void reset();
  ms_t time() const;
  ms_t getDuration() const;
  void delay(ms_t duration) const;
};
class AscList {
  Val_t const left_bound_of_xs;
  Val_t const right_bound_of_xs;
  Val_t const *const ys;
  int const number_of_intervals;
public:
  AscList() = delete;
  AscList(AscList const &other) = delete;
  AscList(AscList &&other) = delete;
  template <size_t size_of_data_sheet>
  AscList(Val_t const (*const data_sheet_ref)[size_of_data_sheet], Val_t const left_bound, Val_t const right_bound)
    : left_bound_of_xs{ left_bound }
    , right_bound_of_xs{ right_bound }
    , ys{ *data_sheet_ref }
    , number_of_intervals{ static_cast<int>(size_of_data_sheet) - 1 }
  {
  }
  ~AscList();
  bool isValid() const;
  Val_t get_y_by_x(Val_t x) const;
  Val_t get_x_by_parameter(Val_t param) const;
  Val_t get_x_by_y(Val_t y) const;
};
template <size_t TableWidth, typename Fractional_t = Val_t>
class Map2d {
  Fractional_t const left_bound_of_xs;
  Fractional_t const right_bound_of_xs;
  Fractional_t const min_of_s;
  Fractional_t const max_of_s;
  int const number_of_intervals;
  int const number_of_s_levels;
  Fractional_t const (*table)[TableWidth];
  Fractional_t ys[TableWidth];
public:
  template <size_t TableHeight>
  Map2d(Fractional_t const (*data_sheet_ref)[TableHeight][TableWidth], Fractional_t const left_bound, Fractional_t const right_bound, Fractional_t const s_min, Fractional_t const s_max)
    : left_bound_of_xs{ left_bound }
    , right_bound_of_xs{ right_bound }
    , min_of_s{ s_min }
    , max_of_s{ s_max }
    , number_of_intervals{ static_cast<int>(TableWidth) - 1 }
    , number_of_s_levels{ static_cast<int>(TableHeight) - 1 }
    , table{ *data_sheet_ref }
    , ys{ }
  {
  }
  Map2d() = delete;
  Map2d(Map2d const &other) = delete;
  Map2d(Map2d &&other) = delete;
  ~Map2d()
  {
  }
  Fractional_t get_x_by_parameter(Fractional_t const param) const
  {
    return ((param * (right_bound_of_xs - left_bound_of_xs) / number_of_intervals) + left_bound_of_xs);
  }
  Fractional_t get_x_by_y(Fractional_t const y) const
  {
    int low = 0, high = number_of_intervals;
  
    while (low <= high)
    {
      int mid = low + ((high - low) / 2);
  
      if (ys[mid] > y)
      {
        high = mid - 1;
      }
      else if (ys[mid] < y)
      {
        low = mid + 1;
      }
      else
      {
        return this->get_x_by_parameter(mid);
      }
    }
    if (low > number_of_intervals)
    {
      return this->get_x_by_parameter(number_of_intervals);
    }
    else if (high < 0)
    {
      return this->get_x_by_parameter(0);
    }
    else
    {
      return this->get_x_by_parameter(((y - ys[high]) / (ys[low] - ys[high])) * (low - high) + high);
    } 
  }
  Fractional_t with_s_get_x_by_y(Fractional_t const s, Fractional_t const y)
  {
    if (s <= min_of_s)
    {
      for (int i = 0; i < TableWidth; i++)
      {
        ys[i] = table[0][i];
      }
    }
    else if (s >= max_of_s)
    {
      for (int i = 0; i < TableWidth; i++)
      {
        ys[i] = table[number_of_s_levels][i];
      }
    }
    else
    {
      Fractional_t const param_s = (s - min_of_s) * (number_of_s_levels / (max_of_s - min_of_s));
      int const idx = param_s;
      
      for (int i = 0; i < TableWidth; i++)
      {        
        ys[i] = ((table[idx + 1][i] - table[idx][i]) * (param_s - idx)) + table[idx][i];
      }
    }
    return this->get_x_by_y(y);
  }
};
/* Comments
** [invokingSerial]
** 1. A function to start serial connection.
** [drawlineSerial]
** 1. A function to draw line in the serial monitor.
** [POW]
** 1. Usage
** > y = POW(x, n);
** - Requirements
**   [A] x >= 0
**   [B] n >= 0
**   [C] x^n =< 2^63 - 1
** - Guarantees
**   [A] y = x^n
**   [B] y >= 1
** [Timer]
** 1. A class, which imitates hourglass.
** [AscList]
** 1. A class to calculate the inverse of the strictly increasing function.
** [Map2d]
** 1. A class, which is equivalent to the class `AscList` with parameter `s`.
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
      this->putChar("0123456789ABCDEF"[printMe]);
    }
  }
  void putInt(BigInt_t const printMe, int const base)
  {
    int cn = 0;
    BigInt_t val = printMe;
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
  void putDouble(double const printMe, int const afters_dot)
  {
    constexpr int base = 10;
    double val = printMe;
    if (printMe < 0.0)
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
  SizedFormatter<LCD_SECTION_LEN> auxiliary_buffer;
  char main_buffer[LCD_HEIGHT][LCD_WIDTH + 1];
public:
  LcdPrinter() = delete;
  LcdPrinter(LcdPrinter const &other) = delete;
  LcdPrinter(LcdPrinter &&other) = delete;
  LcdPrinter(LcdHandle_t const &lcdHandleRef);
  ~LcdPrinter();
  void overwrite();
  void clear();
  void send();
  void flush();
  void newline();
  void print(int num, int base = 10);
  void println(int num, int base = 10);
  void print(double val, int afters_dot = 2);
  void println(double val, int afters_dot = 2);
  void print(char const *str);
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
  void trick();
  SerialPrinter operator<<(bool is);
  SerialPrinter operator<<(byte hex);
  SerialPrinter operator<<(int num);
  SerialPrinter operator<<(char const *str);
  SerialPrinter operator<<(double val);
};
extern SerialPrinter sout, serr, slog;
/* Comments
** [openLcdI2C]
** 1. Usage
** > lcdHandle = openLcdI2C(lcd_screen_width, lcd_screen_height);
** - Requirements
**   [A] `Wire.begin();` must be executed before calling this function.
**   [B] lcd_screen_width > 0
**   [C] lcd_screen_height > 0
**   [D] If the arduino board is Uno,
**       then I2C::SDA must be connected Uno::A4;
**            I2C::SCL must be connected Uno::A5;
**            I2C::VCC must be connected Uno::5V; and
**            I2C::GND must be connected Uno::GND.
**   [E] If the arduino board is Nano,
**       then I2C::SDA must be connected Nano::A4;
**            I2C::SCL must be connected Nano::A5;
**            I2C::VCC must be connected Nano::5V; and
**            I2C::GND must be connected Nano::GND.
** - Guarantees
**   [A] If `lcdHandle` is not a null-pointer,
**       the screen of an LCD is initialized,
**       which is being handled by `lcdHandle`.
** 2. References
**    [1] https://codingrun.com/119
**    [2] https://m.blog.naver.com/hy10101010/221562445464
** [SizedFormatter]
** 1. A class, which helps the class `LcdPrinter`.
** [LcdPrinter]
** 1. A class, whose destructor prints contents in the screen.
** [SerialPrinter]
** 1. A class, which is similar to `std::ostream` of C++.
** 2. But the major difference is that line breaks in this class become `;`.
**    This feature is carried out by `SerialPrinter::~SerialPrinter` and `SerialPrinter::trick`.
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
  int readSignalOnce() const;
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
  void openPin() const;
  void syncPin();
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
  void openPin() const;
  void init() const;
  void set(double duty_ratio) const;
};
/* Comments
** [PinHandler]
** 1. The base class of pin-handling classes.
** 2. Its instances consist of a pin to contol.
** [PinReader]
** 1. A class, read analog signal from the sensor.
** [PinSetter]
** 1. A class, make the pin send digital signal. 
** [PwmSetter]
** 1. A class, make the pin send PWM-wave. 
*/

// implemented in "data.cpp"
extern AscList const mySocOcvTable, mySocVcellTable;
/* Comments
** [mySocOcvTable]
** 1. A table which maps `soc` to `ocv`,
**    where `0.00 =< soc =< 100.00`.
** 2. Usage
** > soc = mySocOcvTable.get_x_by_y(ocv);
** - Guarantees
**   [A] 0.00 =< soc =< 100.00
** [mySocVcellTable]
** 1. A table which maps `soc` to `Vcell`,
**    where `0.00 =< soc =< 98.00`.
** 2. Usage
** > soc = mySocVcellTable.get_x_by_y(Vcell);
** - Guarantees
**   [A] 0.00 =< soc =< 98.00
*/

// implemented in "capstone.ino"
struct PinsOfCell {
  PinReader voltage_sensor_pin;
  PinSetter BalanceCircuit_pin;
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
