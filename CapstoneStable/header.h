#ifndef HEADER
#define HEADER

#include <Wire.h>
#include "LiquidCrystal_I2C.h"
#include "version.h"

LiquidCrystal_I2C *openLcdI2C(int lcd_width, int lcd_height);

class SerialPrinter {
  char const *const _prefix;
public:
  SerialPrinter(char const *prefix);
  SerialPrinter(SerialPrinter const &other);
  SerialPrinter &operator<<(byte const &hex);
  SerialPrinter &operator<<(int const &num);
  SerialPrinter &operator<<(char const *const & str);
  SerialPrinter &operator<<(double const &val);
  ~SerialPrinter();
};

#endif
