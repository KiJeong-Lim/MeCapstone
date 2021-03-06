/* <CAPSTONE PROJECT>
** ===============================================================================
** MEMBER        | AFFILIATION                                                   |
** ===============================================================================
** Hwan-hee Jeon | School of Mechanical Engineering, Chonnam National University |
** Hak-jung Im   | School of Mechanical Engineering, Chonnam National University |
** Ki-jeong Lim  | School of Mechanical Engineering, Chonnam National University |
** ===============================================================================
*/

#include "capstone.hpp"

LcdHandle_t openLcdI2C(int const lcdWidth, int const lcdHeight)
{
  LcdHandle_t myLcdHandle = nullptr;

  if (lcdWidth > 0 && lcdHeight > 0)
  {
    byte adr = 0x7F;

    do
    {
      byte response = 4;

      Wire.beginTransmission(adr);
      response = Wire.endTransmission(adr);
      if (response == 0)
      {
        sout << "I2C address found: address = " << adr << ".";
        myLcdHandle = new LiquidCrystal_I2C(adr, lcdWidth, lcdHeight);
        if (myLcdHandle)
        {
          sout << "I2C connected: address = " << adr << ".";
          break;
        }
      }
      adr--;
    } while (adr != 0x00);

    if (myLcdHandle)
    {
      myLcdHandle->init();
      myLcdHandle->backlight();
    }
  }
  return myLcdHandle;
}

LcdPrinter::LcdPrinter(LcdHandle_t const &lcdHandleRef)
  : lcdHandle{ lcdHandleRef }
  , section_no{ 0 }
  , auxiliary_buffer{ }
  , main_buffer{ }
{
  for (int c = 0; c < LENGTH(main_buffer); c++)
  {
    for (int r = 0; r < LENGTH(*main_buffer); r++)
    {
      main_buffer[c][r] = '\0';
    }
  }
  auxiliary_buffer.clear();
}
LcdPrinter::~LcdPrinter()
{
  this->flush();
  this->send();
}
void LcdPrinter::overwrite()
{
  section_no = 0;
  auxiliary_buffer.clear();
}
void LcdPrinter::clear()
{
  lcdHandle->clear();
  section_no = 0;
  for (int c = 0; c < LCD_HEIGHT; c++)
  {
    for (int r = 0; r < LCD_WIDTH; r++)
    {
      main_buffer[c][r] = ' ';
    }
    main_buffer[c][LCD_WIDTH] = '\0';
  }
  auxiliary_buffer.clear();
}
void LcdPrinter::send()
{
  if (lcdHandle)
  {
    lcdHandle->clear();
    for (int c = 0; c < LCD_HEIGHT; c++)
    {
      lcdHandle->setCursor(0, c);
      main_buffer[c][LCD_WIDTH] = '\0';
      lcdHandle->print(main_buffer[c]);
    }
  }
}
void LcdPrinter::flush()
{
  int const c = (section_no / LCD_SECTION_EA) * 1;
  int const r = (section_no % LCD_SECTION_EA) * LCD_SECTION_LEN;
  if (c < LCD_HEIGHT && r < LCD_WIDTH)
  {
    auxiliary_buffer.send(&main_buffer[c][r]);
  }
  auxiliary_buffer.clear();
}
void LcdPrinter::newline()
{
  this->flush();
  section_no++;
}
void LcdPrinter::print(int const num, int const base)
{
  if (base > 0 && base <= 16)
  {
    auxiliary_buffer.putInt(num, base);
  }
}
void LcdPrinter::print(double const val, int const afters_dot)
{
  auxiliary_buffer.putDouble(val, afters_dot);
}
void LcdPrinter::print(char const *const str)
{
  auxiliary_buffer.putString(str);
}
void LcdPrinter::println(int const num, int const base)
{
  this->print(num, base);
  this->newline();
}
void LcdPrinter::println(double const val, int const afters_dot)
{
  this->print(val, afters_dot);
  this->newline();
}
void LcdPrinter::println(char const *const str)
{
  this->print(str);
  this->newline();
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
  if (Serial)
  { if (newline)
    {
      Serial.println("");
      delay(5);
    }
  }
#endif
}
void SerialPrinter::trick()
{
  newline = false;
#if defined(SERIAL_PORT)
  if (Serial)
  {
    if (prefix_of_message)
    {
      Serial.print(prefix_of_message);
    }
  }
#endif
}
SerialPrinter SerialPrinter::operator<<(bool is)
{
  this->trick();
#if defined(SERIAL_PORT)
  if (Serial)
  {
    Serial.print(is ? "true" : "false");
  }
#endif
  return { .prefix = nullptr, .lend = true };
}
SerialPrinter SerialPrinter::operator<<(byte const hex)
{
  this->trick();
#if defined(SERIAL_PORT)
  if (Serial)
  {
    Serial.print("0x");
    if (hex < 16)
    {
      Serial.print("0");
    }
    Serial.print(hex, HEX);
  }
#endif
  return { .prefix = nullptr, .lend = true };
}
SerialPrinter SerialPrinter::operator<<(int const num)
{
  this->trick();
#if defined(SERIAL_PORT)
  if (Serial)
  {
    Serial.print(num);
  }
#endif
  return { .prefix = nullptr, .lend = true };
}
SerialPrinter SerialPrinter::operator<<(char const *const str)
{
  this->trick();
#if defined(SERIAL_PORT)
  if (Serial)
  {
    Serial.print(str);
  }
#endif
  return { .prefix = nullptr, .lend = true };
}
SerialPrinter SerialPrinter::operator<<(double const val)
{
  this->trick();
#if defined(SERIAL_PORT)
  if (Serial)
  {
    Serial.print(val);
  }
#endif
  return { .prefix = nullptr, .lend = true };
}

SerialPrinter sout = { .prefix = "arduino> " };
SerialPrinter serr = { .prefix = "WARNING> " };
SerialPrinter slog = { .prefix = "       > " };
