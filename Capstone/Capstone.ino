/* 2021-11-12 <CAPSTONE PROJECT>
** ===============================================================================
** MEMBERS       | AFFILIATION                                                   |
** ===============================================================================
** Hwan-hee Jeon | School of Mechanical Engineering, Chonnam National University |
** Hak-jung Im   | School of Mechanical Engineering, Chonnam National University |
** Ki-jeong Lim  | School of Mechanical Engineering, Chonnam National University |
** ===============================================================================
*/

#define BMS_VERSION 1.00
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
#ifndef NOT_CONTROL_BALANCE_CIRCUIT
  void control(); // FIX ME
#endif
  bool checkSafety(); // FIX ME
  void execEmergencyMode(); // FIX ME
  void goodbye(int timeLeftToQuit); // TRUST ME
  void measure(bool showValues); // TRUST ME
  void initWire(); // TRUST ME
  bool openLCD(int lcd_width, int lcd_height); // TRUST ME
  void hello(); // TRUST ME
} myBMS;

void setup()
{
#ifndef NO_DEBUGGING
  Serial.begin(SERIAL_PORT);
#endif
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
  Timer hourglass;

#ifndef NO_DEBUGGING
  Serial.println("[log] Turn changed.");
#endif
  measure(true);
#ifndef NO_DEBUGGING
  delay(10);
#endif
#ifndef NOT_CONTROL_BALANCE_CIRCUIT
  control();
#endif
  {
    bool system_is_okay = checkSafety();

    if (system_is_okay and jobs_done)
    {
#ifndef NO_DEBUGGING
      Serial.println("[log] CHARGING COMPLETED.");
#endif
      for (int i = 0; i < LENGTH_OF(cells); i++)
      {
        cells[i].balanceCircuit_pin.turnOn();
      }
#ifndef NO_LCD_USE
      if (lcdOkay)
      {
        lcd_handle->clear();
        lcd_handle->setCursor(0, 1);
        lcd_handle->print("JOBS FINISHED");
      }
#endif
      goodbye(10);
    }
    else
    {
      for (given_time -= hourglass.getDuration(); given_time > 0; given_time -= hourglass.getDuration())
      {
        if (not system_is_okay)
        {
          execEmergencyMode();
        }
        delay(10);
        system_is_okay = checkSafety();
      }
    }
  }
}

#ifndef NOT_CONTROL_BALANCE_CIRCUIT
void BMS::control()
{
  V_t const V_wanted = 3.8, overV_wanted = 4.1; // <- How to calculate these voltages?

  if (measuredValuesAreFresh)
  {
    delay(10);
    measure(false);
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
#endif

bool BMS::checkSafety()
{
  V_t const allowedV_max = 4.20, allowedV_min =  2.70; // CONFIRM US
  A_t const allowedA_max = 2.00, allowedA_min = -0.10; // CONFIRM US
  bool isBad = false;

  if (measuredValuesAreFresh)
  {
    delay(10);
    measure(false);
  }

  // check current
#ifndef NOT_CONSIDER_SUPPLY_CURRENT
  if (Iin > allowedA_max)
  {
    isBad = true;
#ifndef NO_DEBUGGING
    Serial.println("[Warning] 'Iin' too high.");
#endif
  }
  if (Iin < allowedA_min)
  {
    isBad = true;
#ifndef NO_DEBUGGING
    Serial.println("[Warning] 'Iin' too low.");
#endif
  }
#endif

  // check voltage
  for (int i = 0; i < LENGTH_OF(cellV); i++)
  {
    if (cellV[i] > allowedV_max)
    {
      isBad = true;
#ifndef NO_DEBUGGING
      Serial.print("[Warning] 'cellV[");
      Serial.print(i);
      Serial.print("]'");
      Serial.println(" too high.");
#endif
    }
    if (cellV[i] < allowedV_min)
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

void BMS::execEmergencyMode()
{
#ifndef NOT_CONTROL_BALANCE_CIRCUIT
  for (int i = 0; i < LENGTH_OF(cells); i++)
  {
    cells[i].balanceCircuit_pin.turnOn();
  }
#endif
  delay(500);
#ifndef NOT_CONTROL_BALANCE_CIRCUIT
  for (int i = 0; i < LENGTH_OF(cells); i++)
  {
    cells[i].balanceCircuit_pin.turnOff();
  }
#endif
}

void BMS::goodbye(int const countDown)
{
#ifndef NO_LCD_USE
  if (lcdOkay)
  {
    lcd_handle->setCursor(1, 0);
    lcd_handle->print(" SECS LEFT");
  }
#endif
  for (int i = countDown; i > 0; i--)
  {
#ifndef NO_LCD_USE
    if (lcdOkay)
    {
      lcd_handle->setCursor(0, 0);
      lcd_handle->print(i - 1);
    }
#endif
#ifndef NO_DEBUGGING
    Serial.print("[Warning] Your arduino will abort in ");
    Serial.print(i);
    Serial.println(" seconds.");
#endif
    delay(toMilliSeconds(1));
  }
  powerIn_pin.turnOff();
  abort();
}

void BMS::measure(bool const showValues)
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

  if (showValues)
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
    LcdPrettyPrinter lcd = { .controllerOfLCD = lcd_handle };
    
    lcd.println("> SYSTEM");
    lcd.println(" ONLINE");
    lcd.println("VERSION");
    lcd.print("= ");
    lcd.println(BMS_VERSION);
  }
#endif
}
