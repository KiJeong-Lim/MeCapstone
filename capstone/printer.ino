/* <CAPSTONE PROJECT>
** ===============================================================================
** MEMBER        | AFFILIATION                                                   |
** ===============================================================================
** Hwan-hee Jeon | School of Mechanical Engineering, Chonnam National University |
** Hak-jung Im   | School of Mechanical Engineering, Chonnam National University |
** Ki-jeong Lim  | School of Mechanical Engineering, Chonnam National University |
** ===============================================================================
*/

#include "header.h"

// openLcdI2C
LiquidCrystal_I2C *openLcdI2C()
{
  LiquidCrystal_I2C *lcdHandle = nullptr;

  if (LCD_WIDTH > 0 && LCD_HEIGHT > 0)
  {
    byte adr = 0xFF;

    do
    {
      byte response = 4;

      Wire.beginTransmission(adr);
      response = Wire.endTransmission(adr);
      if (response == 0)
      {
        sout << "I2C address found: address = " << adr << ".";
        lcdHandle = new LiquidCrystal_I2C(adr, LCD_WIDTH, LCD_HEIGHT);
        if (lcdHandle)
        {
          sout << "I2C connected: address = " << adr << ".";
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

// LcdPrettyPrinter
LcdPrettyPrinter::LcdPrettyPrinter(LiquidCrystal_I2C *const controllerOfLCD)
  : lcdHandle{ controllerOfLCD }
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
LcdPrettyPrinter::~LcdPrettyPrinter()
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
void LcdPrettyPrinter::flush()
{
  int const c = (section_no / LCD_SECTION_EA) * 1;
  int const r = (section_no % LCD_SECTION_EA) * LCD_SECTION_LEN;
  if (c < LCD_HEIGHT && r < LCD_WIDTH)
  {
    fbuf.send(&mybuf[c][r]);
  }
  fbuf.clear();
}
void LcdPrettyPrinter::newline()
{
  flush();
  section_no++;
}
void LcdPrettyPrinter::print(int const num)
{
  fbuf.putInt(num);
}
void LcdPrettyPrinter::print(double const val)
{
  fbuf.putDouble(val, 2);
}
void LcdPrettyPrinter::print(char const *const str)
{
  fbuf.putString(str);
}
void LcdPrettyPrinter::println(int const num)
{
  fbuf.putInt(num);
  newline();
}
void LcdPrettyPrinter::println(double const val)
{
  fbuf.putDouble(val, 2);
  newline();
}
void LcdPrettyPrinter::println(char const *const str)
{
  fbuf.putString(str);
  newline();
}

// SerialPrinter
SerialPrinter::SerialPrinter(SerialPrinter &&other)
  : messenger{ other.messenger }
  , newline{ true }
{
  other.newline = false;
}
SerialPrinter::SerialPrinter(char const *const prefix)
  : messenger{ prefix }
  , newline{ false }
{
}
SerialPrinter::SerialPrinter(char const *const prefix, bool const lend)
  : messenger{ prefix }
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
void SerialPrinter::print_messenger()
{
  newline = false;
#if defined(SERIAL_PORT)
  if (messenger)
  {
    Serial.print(messenger);
  }
#endif
}
SerialPrinter SerialPrinter::operator<<(byte const &hex)
{
  print_messenger();
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
  print_messenger();
#if defined(SERIAL_PORT)
  Serial.print(num);
#endif
  return { .prefix = nullptr, .lend = true };
}
SerialPrinter SerialPrinter::operator<<(char const *const &str)
{
  print_messenger();
#if defined(SERIAL_PORT)
  Serial.print(str);
#endif
  return { .prefix = nullptr, .lend = true };
}
SerialPrinter SerialPrinter::operator<<(double const &val)
{
  print_messenger();
#if defined(SERIAL_PORT)
  Serial.print(val);
#endif
  return { .prefix = nullptr, .lend = true };
}
