/* 2021-11-12 <CAPSTONE PROJECT>
** ===============================================================================
** MEMBERS       | AFFILIATION                                                   |
** ===============================================================================
** Hwan-hee Jeon | School of Mechanical Engineering, Chonnam National University |
** Hak-jung Im   | School of Mechanical Engineering, Chonnam National University |
** Ki-jeong Lim  | School of Mechanical Engineering, Chonnam National University |
** ===============================================================================
*/

#include "Capstone.h"

struct ReferenceCollection {
  Val_t analogSignalMax;
  V_t arduinoRegularV;
  V_t zenerdiodeVfromRtoA;
  Ohm_t conversion_ratio_for_ampere_sensor;
} const refOf = {
  .analogSignalMax = 1024.0,
  .arduinoRegularV = 5.0,
  .zenerdiodeVfromRtoA = 2.48,
  .conversion_ratio_for_ampere_sensor = 1 / SENSITIVITY_OF_20A_AMPERE_SENSOR,
};

struct CELL {
  ReaderAnalogPin const voltageSensor_pin;
  WriterDigitalPin const balanceCircuit_pin;
};

CELL cells[] = {
  { .voltageSensor_pin = { .pos = A0 }, .balanceCircuit_pin = { .pos = 2 } },
  { .voltageSensor_pin = { .pos = A1 }, .balanceCircuit_pin = { .pos = 3 } },
  { .voltageSensor_pin = { .pos = A2 }, .balanceCircuit_pin = { .pos = 4 } },
};

class BMS {
  ReaderAnalogPin arduino5V_pin = { .pos = A3 };
  ReaderAnalogPin Iin_pin = { .pos = A4 };
  WriterDigitalPin powerIn_pin = { .pos = 5 };
  bool wire_on = false;
  bool lcdOkay = false;
  LiquidCrystal_I2C *lcd_handle = nullptr;
  A_t Iin = 0.0;
  V_t arduino5V = 5.0;
  V_t cellV[LENGTH_OF(cells)] = {};
public:
  void init();
  void step();
private:
  void measure();
  bool control();
  bool checkSafety();
  void execEmergencyMode();
  bool openLCD(int lcd_width, int lcd_height);
  void initWire();
  void greeting();
} my_bms;

void setup()
{
  my_bms.init();
}

void loop()
{
  my_bms.step();
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
  bool isOkay = true, charging_finished = false;
  ms_t beg_time, cur_time;

  beg_time = millis();
  measure();
#ifndef NO_DEBUGGING
  Serial.print("> A5V = ");
  Serial.print(arduino5V);
  Serial.print("[V], ");
  for (int i = 0; i < LENGTH_OF(cellV); i++)
  {
    Serial.print("C");
    Serial.print(i);
    Serial.print("V = ");
    Serial.print(cellV[i]);
    Serial.print("[V], ");
  }
  Serial.print("I = ");
  Serial.print(Iin);
  Serial.println("[A]. ");
#endif
#ifndef NO_LCD_USE
  if (lcdOkay)
  {
    LcdSplitPrinter lcd = { .controllerOfLCD = lcd_handle };

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
  charging_finished = control();
  isOkay = checkSafety();

  if (isOkay && charging_finished)
  {
#ifndef NO_DEBUGGING
    Serial.print("[log] CHARGING COMPLETED");
#endif
#ifndef NO_LCD_USE
    if (lcdOkay)
    {
      lcd_handle->clear();
      lcd_handle->setCursor(0, 0);
      lcd_handle->print("FULLY CHARGED");
      lcd_handle->setCursor(0, 1);
      lcd_handle->print("REMOVE BATTERY");
    }
#endif
    powerIn_pin.turnOff();
    delay(10000);
  }
  else
  {
    for (cur_time = millis(); cur_time - beg_time < given_time; cur_time = millis())
    {
      isOkay = checkSafety();
      if (!isOkay)
      {
        execEmergencyMode();
      }
    }
  }
}

void BMS::measure()
{
  ms_t measuring_time = 10;
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
}

bool BMS::control()
{
  V_t const targetV_min = 3.8, targetV_max = 4.1;
  bool are_all_cells_charged = true;

  for (int i = 0; i < LENGTH_OF(cellV); i++)
  {
    if (cellV[i] >= targetV_max)
    {
      cells[i].balanceCircuit_pin.turnOn();
    }
    else
    {
      are_all_cells_charged = false;
    }

    if (cellV[i] < targetV_min)
    {
      cells[i].balanceCircuit_pin.turnOff();
    }
  }

  return are_all_cells_charged;
}

bool BMS::checkSafety()
{
  V_t const allowedV_high = 4.2, allowedV_low = 2.7;
  A_t const allowedA_high = 2.0, allowedA_low = -0.1;
  bool isBad = false;
  
  measure();

  if (Iin >= allowedA_high)
  {
    isBad = true;
#ifndef NO_DEBUGGING
    Serial.println("[Warning] Iin too high!");
#endif
  }
  if (Iin <= allowedA_low)
  {
    isBad = true;
#ifndef NO_DEBUGGING
    Serial.println("[Warning] Iin too low!");
#endif
  }
  for (int i = 0; i < LENGTH_OF(cellV); i++)
  {
    if (cellV[i] >= allowedV_high)
    {
      isBad = true;
#ifndef NO_DEBUGGING
      Serial.print("[Warning] cellV[");
      Serial.print(i);
      Serial.print("]");
      Serial.println(" too high!");
#endif
    }
  }
  for (int i = 0; i < LENGTH_OF(cellV); i++)
  {
    if (cellV[i] <= allowedV_low)
    {
      isBad = true;
#ifndef NO_DEBUGGING
      Serial.print("[Warning] cellV[");
      Serial.print(i);
      Serial.print("]");
      Serial.println(" too low!");
#endif
    }
  }
  return isBad;
}

void BMS::execEmergencyMode()
{
  powerIn_pin.turnOff();
  for (int i = 0; i < LENGTH_OF(cells); i++)
  {
    cells[i].balanceCircuit_pin.turnOn();
  }
  delay(100);
  powerIn_pin.turnOn();
  for (int i = 0; i < LENGTH_OF(cells); i++)
  {
    cells[i].balanceCircuit_pin.turnOff();
  }
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

void BMS::initWire()
{
  Wire.begin();
  wire_on = true;
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
    lcd_handle->print("VER = ");
    lcd_handle->print(CAPSTONE_VERSION);
  }
#endif
}
