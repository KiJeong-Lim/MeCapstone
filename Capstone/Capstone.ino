/* 2021-11-12 <CAPSTONE PROJECT>
** ===============================================================================
** MEMBERS       | AFFILIATION                                                   |
** ===============================================================================
** Hwan-hee Jeon | School of Mechanical Engineering, Chonnam National University |
** Hak-jung Im   | School of Mechanical Engineering, Chonnam National University |
** Ki-jeong Lim  | School of Mechanical Engineering, Chonnam National University |
** ===============================================================================
*/

#define BMS_VERSION "1.0.0"
#include "Capstone.h"

inline void printHexOnSerial(byte const integer_between_0_and_255)
{
#ifndef NO_DEBUGGING
  Serial.print("0x");
  if (integer_between_0_and_255 < 16)
  {
    Serial.print("0");
  }
  Serial.print(integer_between_0_and_255, HEX);
#endif
}

ReferenceCollection const refOf = {
  .analogSignalMax = 1024.0,
  .arduinoRegularV = 5.00,
  .zenerdiodeVfromRtoA = 2.48,
  .conversion_ratio_for_ampere_sensor = 1 / SENSITIVITY_OF_20A_CURRENT_SENSOR,
};

CELL cells[] = {
  { .voltageSensor_pin = { .pin_no = A0 }, .balanceCircuit_pin = { .pin_no = 2 } },
  { .voltageSensor_pin = { .pin_no = A1 }, .balanceCircuit_pin = { .pin_no = 3 } },
  { .voltageSensor_pin = { .pin_no = A2 }, .balanceCircuit_pin = { .pin_no = 4 } },
};

class BMS {
  ReaderAnalogPin arduino5V_pin = { .pin_no = A3 };
  ReaderAnalogPin Iin_pin = { .pin_no = A4 };
  WriterDigitalPin powerIn_pin = { .pin_no = 5 };
  byte wire_on = false;
  byte lcdOkay = false;
  byte measuredValuesAreFresh = false;
  byte charging_finished = false;
  LiquidCrystal_I2C *lcd_handle = nullptr;
  A_t Iin = 0.0;
  V_t arduino5V = refOf.arduinoRegularV;
  V_t cellV[LENGTH_OF(cells)] = {};
public:
  void init();
  void step();
private:
  void measure();
  void control();
  bool checkSafety();
  void goodbye();
  void execEmergencyMode();
  void showValues();
  void initWire();
  bool openLCD(int lcd_width, int lcd_height);
  void greeting();
} myBMS;

void setup()
{
  myBMS.init();
}

void loop()
{
  myBMS.step();
}

void BMS::init()
{
  ms_t const given_time = 500;
  ms_t beg_time, cur_time;

  beg_time = millis();
#ifndef NO_DEBUGGING
  Serial.begin(SERIAL_PORT);
  Serial.println("[log] Runtime started.");
#endif
  initWire();
  for (int i = 0; i < LENGTH_OF(cells); i++)
  {
    cells[i].balanceCircuit_pin.initWith(true);
  }
  powerIn_pin.initWith(true);
#ifndef NO_LCD_USE
  lcdOkay = openLCD(LCD_WIDTH, LCD_HEIGHT);
#endif
#ifndef NO_DEBUGGING
  if (!lcdOkay)
  {
    Serial.println("[Warning] No lcd connected.");
  }
#endif
  cur_time = millis();
  delay(beg_time < cur_time - given_time ? given_time - (cur_time - beg_time) : 0);
}

void BMS::step()
{
  ms_t const given_time = 3000;
  bool system_is_okay = true;
  ms_t beg_time, cur_time;

  beg_time = millis();
  measure();
  showValues();
  control();
  system_is_okay = checkSafety();

  if (system_is_okay && charging_finished)
  {
    goodbye();
  }
  else
  {
    for (cur_time = millis(); cur_time - beg_time < given_time; cur_time = millis())
    {
      if (!system_is_okay)
      {
        execEmergencyMode();
      }
      delay(100);
      system_is_okay = checkSafety();
    }
  }
}

void BMS::measure()
{
  ms_t const measuring_time = 10;
  V_t sensorV = 0.0;
  V_t accumV = 0.0;

  sensorV = refOf.arduinoRegularV * arduino5V_pin.readSignal(measuring_time) / refOf.analogSignalMax;
  arduino5V = refOf.arduinoRegularV * refOf.zenerdiodeVfromRtoA / sensorV;

  sensorV = arduino5V * Iin_pin.readSignal(measuring_time) / refOf.analogSignalMax;
  Iin = refOf.conversion_ratio_for_ampere_sensor * (sensorV - arduino5V * 0.5);

  for (int i = 0; i < LENGTH_OF(cells); i++)
  {
    sensorV = arduino5V * cells[i].voltageSensor_pin.readSignal(measuring_time) / refOf.analogSignalMax;
    cellV[i] = sensorV - accumV;
    accumV += cellV[i];
  }

  measuredValuesAreFresh = true;
}

void BMS::control()
{
  V_t const targetV = 3.8, targetV_overloaded = 4.1;

  while (measuredValuesAreFresh)
  {
    measure();
  }

  charging_finished = true;
  for (int i = 0; i < LENGTH_OF(cellV); i++)
  {
    bool const this_cell_being_charged_now = not cells[i].balanceCircuit_pin.isHigh();
    bool const this_cell_charging_finished = cellV[i] >= (this_cell_being_charged_now ? targetV_overloaded : targetV);
    charging_finished &= this_cell_charging_finished;
    if ((not this_cell_charging_finished) and (not this_cell_being_charged_now))
    {
      cells[i].balanceCircuit_pin.turnOff();
    }
    if (this_cell_charging_finished and this_cell_being_charged_now)
    {
      cells[i].balanceCircuit_pin.turnOn();
    }
  }

  measuredValuesAreFresh = false;
}

bool BMS::checkSafety()
{
  V_t const allowedV_high = 4.2, allowedV_low = 2.7;
  A_t const allowedA_high = 2.0, allowedA_low = -0.1;
  bool isBad = false;

  while (measuredValuesAreFresh)
  {
    measure();
  }

  if (Iin >= allowedA_high)
  {
    isBad = true;
#ifndef NO_DEBUGGING
    Serial.println("[Warning] 'Iin' too high.");
#endif
  }
  else if (Iin <= allowedA_low)
  {
    isBad = true;
#ifndef NO_DEBUGGING
    Serial.println("[Warning] 'Iin' too low.");
#endif
  }

  for (int i = 0; i < LENGTH_OF(cellV); i++)
  {
    if (cellV[i] >= allowedV_high)
    {
      isBad = true;
#ifndef NO_DEBUGGING
      Serial.print("[Warning] 'cellV[");
      Serial.print(i);
      Serial.print("]'");
      Serial.println(" too high.");
#endif
    }
    else if (cellV[i] <= allowedV_low)
    {
      isBad = true;
#ifndef NO_DEBUGGING
      Serial.print("[Warning] 'cellV[");
      Serial.print(i);
      Serial.print("]'");
      Serial.println(" too low.");
#endif
    }
  }

  measuredValuesAreFresh = false;

  return isBad;
}

void BMS::goodbye()
{
#ifndef NO_DEBUGGING
  Serial.println("[log] CHARGING COMPLETED.");
#endif
#ifndef NO_LCD_USE
  if (lcdOkay)
  {
    lcd_handle->clear();
    lcd_handle->setCursor(0, 0);
    lcd_handle->print("FULLY CHARGED");
    lcd_handle->setCursor(0, 1);
    lcd_handle->print("REMOVE BATTERY");
    for (int i = 0; i < LENGTH_OF(cells); i++)
    {
      cells[i].balanceCircuit_pin.turnOn();
    }
    delay(10000);
  }
#endif
  powerIn_pin.turnOff();
  abort();
}

void BMS::execEmergencyMode()
{
  for (int i = 0; i < LENGTH_OF(cells); i++)
  {
    cells[i].balanceCircuit_pin.turnOn();
  }
  delay(500);
  for (int i = 0; i < LENGTH_OF(cells); i++)
  {
    cells[i].balanceCircuit_pin.turnOff();
  }
}

void BMS::showValues()
{
#ifndef NO_DEBUGGING
  for (int i = 0; i < LENGTH_OF(cellV); i++)
  {
    Serial.print(">>> ");
    Serial.print("C");
    Serial.print(i);
    Serial.print("V = ");
    Serial.print(cellV[i]);
    Serial.println("[V].");
  }
#endif
#ifndef NO_LCD_USE
  if (lcdOkay)
  {
    LcdPrettyPrinter lcd = { .controllerOfLCD = lcd_handle };

    for (int i = 0; i < LENGTH_OF(cellV); i++)
    {
      lcd.print("C");
      lcd.print(i);
      lcd.print("=");
      lcd.print(cellV[i]);
      lcd.println(" ");
    }
    lcd.print("I");
    lcd.print("=");
    lcd.print(Iin);
    lcd.println(" ");
  }
#endif
#ifndef NO_DEBUGGING
  Serial.print(">>> ");
  Serial.print("A5V = ");
  Serial.print(arduino5V);
  Serial.print("[V], ");
  Serial.print("Iin = ");
  Serial.print(Iin);
  Serial.println("[A].");
#endif
}

void BMS::initWire()
{
  Wire.begin();
  wire_on = true;
}

bool BMS::openLCD(int const row_dim, int const col_dim)
{
  byte isGood = false;

  if (wire_on)
  {
#ifndef NO_LCD_USE
    if (row_dim > 0 && col_dim > 0)
    {
      for (byte adr = 0x01; adr < 0xFF; adr++)
      {
        byte response = 4;
        Wire.beginTransmission(adr);
        response = Wire.endTransmission(adr);
        if (response == 0)
        {
#ifndef NO_DEBUGGING
          Serial.print("[log] I2C address found: address = ");
          printHexOnSerial(adr);
          Serial.println(".");
#endif
          lcd_handle = new LiquidCrystal_I2C(adr, row_dim, col_dim);
          if (lcd_handle)
          {
#ifndef NO_DEBUGGING
            Serial.print("[log] I2C connected: address = ");
            printHexOnSerial(adr);
            Serial.println(".");
#endif
            break;
          }
        }
      }

      if (lcd_handle)
      {
        lcd_handle->init();
        lcd_handle->backlight();
        isGood = true;
      }
    }
#endif
  }
#ifndef NO_DEBUGGING
  else
  {
    Serial.println("[Warning] please execute 'this->initWire()' before calling 'BMS::openLCD'.");
  }
#endif

  return isGood;
}

void BMS::greeting()
{
#ifndef NO_LCD_USE
  if (lcdOkay)
  {
    lcd_handle->clear();
    lcd_handle->setCursor(0, 0);
    lcd_handle->print("SYSTEM ONLINE");
    lcd_handle->setCursor(0, 1);
    lcd_handle->print("VERSION = ");
    lcd_handle->print(BMS_VERSION);
  }
#endif
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

void LcdPrettyPrinter::flush()
{
  int const c = (section_no / LCD_SECTION_EA) * 1;
  int const r = (section_no % LCD_SECTION_EA) * LCD_SECTION_LEN;
  char const *p_ch = nullptr;
  if (c < LCD_HEIGHT && r < LCD_WIDTH)
  {
    p_ch = fbuf.get();
    for (int j = 0; j < LCD_SECTION_LEN; j++)
    {
      buffer[c][r + j] = p_ch[j];
    }
  }
  section_no++;
  fbuf.ready();
}
