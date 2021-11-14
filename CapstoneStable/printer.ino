#include "header.h"

SerialPrinter::SerialPrinter(const char *prefix)
  : _prefix{ prefix }
{
}
SerialPrinter::SerialPrinter(SerialPrinter const & other)
  : _prefix{ nullptr }
{
#if defined(SERIAL_PORT)
  if (other._prefix)
  {
    Serial.print(other._prefix);
  }
#endif
}
SerialPrinter &SerialPrinter::operator<<(byte const &hex)
{
#if defined(SERIAL_PORT)
  Serial.print("0x");
  if (hex < 16)
  {
    Serial.print("0");
  }
  Serial.print(hex, HEX);
#endif
  return *this;
}
SerialPrinter &SerialPrinter::operator<<(int const &num)
{
#if defined(SERIAL_PORT)
  Serial.print(num);
#endif
  return *this;
}
SerialPrinter &SerialPrinter::operator<<(char const *const &str)
{
#if defined(SERIAL_PORT)
  Serial.print(str);
#endif
  return *this;
}
SerialPrinter &SerialPrinter::operator<<(double const &val)
{
#if defined(SERIAL_PORT)
  Serial.print(val);
#endif
  return *this;
}
SerialPrinter::~SerialPrinter()
{
#if defined(SERIAL_PORT)
  Serial.println("");
#endif
}

LiquidCrystal_I2C *openLcdI2C(int const row_dim, int const col_dim)
{
  LiquidCrystal_I2C *lcdHandle = nullptr;

  if (row_dim > 0 && col_dim > 0)
  {
    byte adr = 0xFF;

    do
    {
      byte response = 4;

      Wire.beginTransmission(adr);
      response = Wire.endTransmission(adr);
      if (response == 0)
      {
        lcdHandle = new LiquidCrystal_I2C(adr, row_dim, col_dim);
        if (lcdHandle)
        {
          break;
        }
      }
      adr--;
    } while (adr != 0x00);

    if (lcdHandle)
    {
      lcdHandle->init();
      lcdHandle->backlight();
    }
  }

  return lcdHandle;
}
