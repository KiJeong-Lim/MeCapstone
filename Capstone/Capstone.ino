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

#ifndef NO_DEBUGGING
static void printByteOnSerial(byte const integer_between_0_and_255)
{
  Serial.print("0x");
  if (integer_between_0_and_255 < 16)
  {
    Serial.print("0");
  }
  Serial.print(integer_between_0_and_255, HEX);
}
#endif

ReferenceCollection const refOf = {
  .analogSignalMax = 1024.0,
  .arduinoRegularV = 5.00,
  .zenerdiodeVfromRtoA = 2.48,
  .conversionRatioForCurrentSensor = 1 / SENSITIVITY_OF_20A_CURRENT_SENSOR,
};

CELL cells[] = {
  { .voltageSensor_pin = { .pin_no = A0 }, .balanceCircuit_pin = { .pin_no = 2 } },
  { .voltageSensor_pin = { .pin_no = A1 }, .balanceCircuit_pin = { .pin_no = 3 } },
  { .voltageSensor_pin = { .pin_no = A2 }, .balanceCircuit_pin = { .pin_no = 4 } },
};

class BMS {
#ifndef NOT_CONSIDER_SUPPLY_VOLTAGE
  ReaderAnalogPin arduino5V_pin = { .pin_no = A3 };
#endif
#ifndef NOT_CONSIDER_SUPPLY_CURRENT
  ReaderAnalogPin Iin_pin = { .pin_no = A4 };
#endif
  WriterDigitalPin powerIn_pin = { .pin_no = 5 };
  byte wire_on = false;
  byte lcdOkay = false;
  byte jobs_done = false;
  byte measuredValuesAreFresh = false;
  LiquidCrystal_I2C *lcd_handle = nullptr;
  V_t arduino5V = refOf.arduinoRegularV;
#ifndef NOT_CONSIDER_SUPPLY_CURRENT
  A_t Iin = 0.0;
#endif
  V_t cellV[LENGTH_OF(cells)] = { };
public:
  void init(ms_t expected_elapsed_time); // TRUST ME
  void step(ms_t expected_elapsed_time); // FIX ME
private:
  void measure(); // TRUST ME
  void control(); // FIX ME
  bool checkSafety(); // TRUST ME
  void goodbye(); // FIX ME
  void execEmergencyMode(); // FIX ME
  void showValues(); // FIX ME
  void initWire(); // FIX ME
  bool openLCD(int lcd_width, int lcd_height); // TRUST ME
  void hello(); // TRUST ME
} myBMS;

void setup()
{
  myBMS.init(500);
}

void loop()
{
  myBMS.step(3000);
}

void BMS::init(ms_t given_time)
{
  Timer hourglass;

#ifndef NO_DEBUGGING
  Serial.begin(SERIAL_PORT);
#endif
#ifndef NO_DEBUGGING
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
  if (lcdOkay)
  {
    hello();
  }
  else
  {
#ifndef NO_DEBUGGING
    Serial.println("[Warning] No lcd connected.");
#endif
  }
  given_time -= hourglass.getDuration();
  delay(given_time >= 0 ? given_time : 0);
}

void BMS::step(ms_t given_time)
{
  bool system_is_okay = true;
  Timer hourglass;

  measure();
  showValues();
  control();
  system_is_okay = checkSafety();

  if (system_is_okay and jobs_done)
  {
    goodbye();
  }
  else
  {
    for (given_time -= hourglass.getDuration(); given_time > 0; given_time -= hourglass.getDuration())
    {
      if (not system_is_okay)
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
  ms_t const measuring_time_for_one_sensor = 10;
  V_t sensorV = 0.0;
  V_t accumV = 0.0;

#ifndef NOT_CONSIDER_SUPPLY_VOLTAGE
  sensorV = refOf.arduinoRegularV * arduino5V_pin.readSignal(measuring_time_for_one_sensor) / refOf.analogSignalMax;
  arduino5V = refOf.arduinoRegularV * refOf.zenerdiodeVfromRtoA / sensorV;
#endif
  for (int i = 0; i < LENGTH_OF(cells); i++)
  {
    sensorV = arduino5V * cells[i].voltageSensor_pin.readSignal(measuring_time_for_one_sensor) / refOf.analogSignalMax;
    cellV[i] = sensorV - accumV;
    accumV += cellV[i];
  }
#ifndef NOT_CONSIDER_SUPPLY_CURRENT
  sensorV = arduino5V * Iin_pin.readSignal(measuring_time_for_one_sensor) / refOf.analogSignalMax;
  Iin = refOf.conversionRatioForCurrentSensor * (sensorV - arduino5V * 0.5);
#endif

  measuredValuesAreFresh = true;
}

void BMS::control()
{
  V_t const V_wanted = 3.8, overV_wanted = 4.1; // <- How to calculate these voltage?

  while (measuredValuesAreFresh)
  {
    measure();
  }

  jobs_done = true;

  for (int i = 0; i < LENGTH_OF(cellV); i++)
  {
    bool const this_cell_being_charged_now = not cells[i].balanceCircuit_pin.isHigh();
    bool const this_cell_charging_finished = cellV[i] >= (this_cell_being_charged_now ? overV_wanted : V_wanted);

    jobs_done &= this_cell_charging_finished;

    if ((not this_cell_charging_finished) and (not this_cell_being_charged_now))
    {
      cells[i].balanceCircuit_pin.turnOff();
    }

    if ((this_cell_charging_finished) and (this_cell_being_charged_now))
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

#ifndef NOT_CONSIDER_SUPPLY_CURRENT
  if (Iin > allowedA_high)
  {
    isBad = true;
#ifndef NO_DEBUGGING
    Serial.println("[Warning] 'Iin' too high.");
#endif
  }
  else if (Iin < allowedA_low)
  {
    isBad = true;
#ifndef NO_DEBUGGING
    Serial.println("[Warning] 'Iin' too low.");
#endif
  }
#endif

  for (int i = 0; i < LENGTH_OF(cellV); i++)
  {
    if (cellV[i] > allowedV_high)
    {
      isBad = true;
#ifndef NO_DEBUGGING
      Serial.print("[Warning] 'cellV[");
      Serial.print(i);
      Serial.print("]'");
      Serial.println(" too high.");
#endif
    }
    else if (cellV[i] < allowedV_low)
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
  }
#endif
  for (int i = 0; i < LENGTH_OF(cells); i++)
  {
    cells[i].balanceCircuit_pin.turnOn();
  }
  delay(10000);
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
#ifndef NOT_CONSIDER_SUPPLY_VOLTAGE
  Serial.print(">>> ");
  Serial.print("arduino5V");
  Serial.print(" = ");
  Serial.print(arduino5V);
  Serial.println("[V].");
#endif
#ifndef NOT_CONSIDER_SUPPLY_CURRENT
  Serial.print(">>> ");
  Serial.print("Iin");
  Serial.print(" = ");
  Serial.print(Iin);
  Serial.print("[A]");
  Serial.println(".");
#endif
#endif
#ifndef NO_LCD_USE
  if (lcdOkay)
  {
    LcdPrettyPrinter lcd = { .controllerOfLCD = lcd_handle };

    for (int i = 0; i < LENGTH_OF(cellV); i++)
    {
      lcd.print("B");
      lcd.print(i + 1);
      lcd.print("=");
      lcd.print(cellV[i]);
      lcd.println(" ");
    }
#ifndef NOT_CONSIDER_SUPPLY_CURRENT
    lcd.print("I");
    lcd.print("=");
    lcd.print(Iin);
    lcd.println(" ");
#endif
    // Here, the LCD screen will be updated, while the variable lcd being destructed.
    //                              A1234567B1234567
    // A possible screen will be:  ##################
    //                            1#B1=4.25 B2=4.17 #
    //                            2#B3=3.33 I=1.66  #
    //                             ##################
  }
#endif
#ifndef NO_DEBUGGING
  for (int i = 0; i < LENGTH_OF(cellV); i++)
  {
    Serial.print(">>> ");
    Serial.print("cellV[");
    Serial.print(i);
    Serial.print("]");
    Serial.print(" = ");
    Serial.print(cellV[i]);
    Serial.println("[V].");
  }
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
      for (byte adr = 0x01; adr <= 0xFF; adr++)
      {
        byte response = 4;
        Wire.beginTransmission(adr);
        response = Wire.endTransmission(adr);
        if (response == 0)
        {
#ifndef NO_DEBUGGING
          Serial.print("[log] I2C address found: address = ");
          printByteOnSerial(adr);
          Serial.println(".");
#endif
          lcd_handle = new LiquidCrystal_I2C(adr, row_dim, col_dim);
          if (lcd_handle)
          {
#ifndef NO_DEBUGGING
            Serial.print("[log] I2C connected: address = ");
            printByteOnSerial(adr);
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
  else
  {
#ifndef NO_DEBUGGING
    Serial.println("[Warning] Please execute 'this->initWire()' before calling 'BMS::openLCD'.");
#endif
  }

  return isGood;
}

void BMS::hello()
{
#ifndef NO_LCD_USE
  if (lcdOkay)
  {
    lcd_handle->noBacklight();
    lcd_handle->clear();
    lcd_handle->backlight();
    lcd_handle->setCursor(0, 0);
    lcd_handle->print("SYSTEM ONLINE");
    lcd_handle->setCursor(0, 1);
    lcd_handle->print("VERSION = ");
    lcd_handle->print(BMS_VERSION);
  }
#endif
}
