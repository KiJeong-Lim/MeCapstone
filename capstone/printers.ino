/* <CAPSTONE PROJECT>
** ===============================================================================
** MEMBER        | AFFILIATION                                                   |
** ===============================================================================
** Hwan-hee Jeon | School of Mechanical Engineering, Chonnam National University |
** Hak-jung Im   | School of Mechanical Engineering, Chonnam National University |
** Ki-jeong Lim  | School of Mechanical Engineering, Chonnam National University |
** ===============================================================================
*/

#include "capstone.h"

LcdPrinter::LcdPrinter(LiquidCrystal_I2C *const addressOfLcdI2C)
  : lcdHandle{ addressOfLcdI2C }
  , section_no{ 0 }
  , fbuf{ }
  , mybuf{ }
{
  for (int c = 0; c < LENGTH_OF(mybuf); c++)
  {
    for (int r = 0; r < LENGTH_OF(*mybuf); r++)
    {
      mybuf[c][r] = '\0';
    }
  }
  fbuf.clear();
}
LcdPrinter::~LcdPrinter()
{
  if (lcdHandle)
  {
    lcdHandle->clear();
    for (int c = 0; c < LCD_HEIGHT; c++)
    {
      lcdHandle->setCursor(0, c);
      mybuf[c][LCD_WIDTH] = '\0';
      lcdHandle->print(mybuf[c]);
    }
  }
}
void LcdPrinter::flush()
{
  int const c = (section_no / LCD_SECTION_EA) * 1;
  int const r = (section_no % LCD_SECTION_EA) * LCD_SECTION_LEN;
  if (c < LCD_HEIGHT && r < LCD_WIDTH)
  {
    fbuf.send(&mybuf[c][r]);
  }
  fbuf.clear();
}
void LcdPrinter::newline()
{
  flush();
  section_no++;
}
void LcdPrinter::print(int const num)
{
  fbuf.putInt(num, 10);
}
void LcdPrinter::print(double const val)
{
  fbuf.putDouble(val, 2);
}
void LcdPrinter::print(char const *const str)
{
  fbuf.putString(str);
}
void LcdPrinter::println(int const num)
{
  fbuf.putInt(num, 10);
  newline();
}
void LcdPrinter::println(double const val)
{
  fbuf.putDouble(val, 2);
  newline();
}
void LcdPrinter::println(char const *const str)
{
  fbuf.putString(str);
  newline();
}

SerialPrinter::SerialPrinter(SerialPrinter &&other)
  : prefix_of_message{ other.prefix_of_message }
  , newline{ other.newline }
{
  other.newline = false;
}
SerialPrinter::SerialPrinter(char const *const prefix)
  : prefix_of_message{ prefix }
  , newline{ false }
{
}
SerialPrinter::SerialPrinter(char const *const prefix, bool const lend)
  : prefix_of_message{ prefix }
  , newline{ lend }
{
}
SerialPrinter::~SerialPrinter()
{
#if defined(SERIAL_PORT)
  if (newline)
  {
    Serial.println("");
    delay(5);
  }
#endif
}
void SerialPrinter::trick()
{
  newline = false;
#if defined(SERIAL_PORT)
  if (prefix_of_message)
  {
    Serial.print(prefix_of_message);
  }
#endif
}
SerialPrinter SerialPrinter::operator<<(byte const &hex)
{
  trick();
#if defined(SERIAL_PORT)
  Serial.print("0x");
  if (hex < 16)
  {
    Serial.print("0");
  }
  Serial.print(hex, HEX);
#endif
  return { .prefix = nullptr, .lend = true };
}
SerialPrinter SerialPrinter::operator<<(int const &num)
{
  trick();
#if defined(SERIAL_PORT)
  Serial.print(num);
#endif
  return { .prefix = nullptr, .lend = true };
}
SerialPrinter SerialPrinter::operator<<(char const *const &str)
{
  trick();
#if defined(SERIAL_PORT)
  Serial.print(str);
#endif
  return { .prefix = nullptr, .lend = true };
}
SerialPrinter SerialPrinter::operator<<(double const &val)
{
  trick();
#if defined(SERIAL_PORT)
  Serial.print(val);
#endif
  return { .prefix = nullptr, .lend = true };
}

SerialPrinter sout = { .prefix = "arduino> " };
SerialPrinter serr = { .prefix = "WARNING> " };
SerialPrinter slog = { .prefix = "       > " };