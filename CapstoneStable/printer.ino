#include "header.h"

static bigInt_t pow10(int const n)
{
  bigInt_t res = 1;
  for (int i = 0; i < n; i++)
  {
    res *= 10;
  }
  return res;
}

static SerialPrinter &&serialPrinter_trick()
{
  return { .prefix = nullptr, .is_last = true };
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
        consoleLog << "I2C address found: address = " << adr << ".";
        lcdHandle = new LiquidCrystal_I2C(adr, row_dim, col_dim);
        if (lcdHandle)
        {
          consoleLog << "I2C connected: address = " << adr << ".";
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

SerialPrinter::SerialPrinter(const char *prefix)
  : messenger{ prefix }
  , newline{ false }
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
SerialPrinter::SerialPrinter(const char *prefix, bool is_last)
  : messenger{ prefix }, newline{ is_last }
{
}
SerialPrinter &&SerialPrinter::operator<<(byte const &hex)
{
#if defined(SERIAL_PORT)
  this->newline = false;
  Serial.print("0x");
  if (hex < 16)
  {
    Serial.print("0");
  }
  Serial.print(hex, HEX);
#endif
  return serialPrinter_trick();
}
SerialPrinter &&SerialPrinter::operator<<(int const &num)
{
#if defined(SERIAL_PORT)
  this->newline = false;
  Serial.print(num);
#endif
  return serialPrinter_trick();
}
SerialPrinter &&SerialPrinter::operator<<(char const *const &str)
{
#if defined(SERIAL_PORT)
  this->newline = false;
  Serial.print(str);
#endif
  return serialPrinter_trick();
}
SerialPrinter &&SerialPrinter::operator<<(double const &val)
{
#if defined(SERIAL_PORT)
  Serial.print(val);
#endif
  return serialPrinter_trick();
}

void BufferWithFormat::memzero()
{
  for (cnt = 0; cnt < LCD_SECTION_LEN; cnt++)
  {
    buf[cnt] = ' ';
  }
  buf[cnt] = '\0';
  cnt = 0;
}
void BufferWithFormat::memsend(char *p_ch)
{
  for (cnt = 0; cnt < LCD_SECTION_LEN; cnt++)
  {
    *p_ch++ = buf[cnt];
  }
}
void BufferWithFormat::putChar(char const ch)
{
  if (cnt < LCD_SECTION_LEN)
  {
    buf[cnt++] = ch;
  }
}
void BufferWithFormat::putDigit(int const d)
{
  putChar("0123456789ABCDEF"[d]);
}
void BufferWithFormat::putInt(bigInt_t const printMe)
{
  bigInt_t val = printMe;
  int cn = 0;
  if (val < 0)
  {
    putChar('-');
    val *= -1;
  }
  for (bigInt_t _val = 1; _val <= val; _val *= 10)
  {
    cn++;
  }
  do
  {
    putDigit(((10 * val) / pow10(cn)) % 10);
  } while (--cn > 0);
}
void BufferWithFormat::putDouble(double const printMe, int const afters_dot)
{
  double val = printMe;
  if (printMe < 0.0)
  {
    putChar('-');
    val *= -1;
  }
  if (afters_dot > 0)
  {
    bigInt_t valN = val;
    bigInt_t valF = val * pow10(afters_dot);
    int cn = afters_dot;
    putInt(valN);
    putChar('.');
    do
    {
      putDigit(((10 * valF) / pow10(cn)) % 10);
    } while (--cn > 0);
  }
  else
  {
    bigInt_t valN = val;
    bigInt_t valE = pow10(- afters_dot);
    valN /= valE;
    valN *= valE;
    putInt(valN);
  }
}
void BufferWithFormat::putString(char const *const str)
{
  for (char const *p_ch = str; *p_ch != '\0'; p_ch++)
  {
    if (cnt < LCD_SECTION_LEN)
    {
      buf[cnt++] = *p_ch;
    }
    else
    {
      break;
    }
  }
}

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
  fbuf.memzero();
}
LcdPrettyPrinter::~LcdPrettyPrinter()
{
  if (lcdHandle)
  {
    lcdHandle->clear();
    for (int c = 0; c < LCD_HEIGHT; c++)
    {
      lcdHandle->setCursor(0, c);
      mybuf[c][LCD_WIDTH - 1] = '\0';
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
    fbuf.memsend(&mybuf[c][r]);
  }
  section_no++;
  fbuf.memzero();
}
void LcdPrettyPrinter::print(int const value)
{
  fbuf.putInt(value);
}
void LcdPrettyPrinter::print(double const value)
{
  fbuf.putDouble(value, 2);
}
void LcdPrettyPrinter::print(char const *const string)
{
  fbuf.putString(string);
}
void LcdPrettyPrinter::println(int const value)
{
  fbuf.putInt(value);
  flush();
}
void LcdPrettyPrinter::println(double const value)
{
  fbuf.putDouble(value, 2);
  flush();
}
void LcdPrettyPrinter::println(char const *const string)
{
  fbuf.putString(string);
  flush();
}
