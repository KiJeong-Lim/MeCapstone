/* 2021-11-13 <CAPSTONE PROJECT>
** ===============================================================================
** MEMBERS       | AFFILIATION                                                   |
** ===============================================================================
** Hwan-hee Jeon | School of Mechanical Engineering, Chonnam National University |
** Hak-jung Im   | School of Mechanical Engineering, Chonnam National University |
** Ki-jeong Lim  | School of Mechanical Engineering, Chonnam National University |
** ===============================================================================
*/

#define MAJOR_VERSION   0
#define MINOR_VERSION   1
#define REVISION_NUMBER 1
#include "Version.h"
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

ReferenceCollection const refOf =
{ .analogSignalMax = 1024.0
, .arduinoRegularV = 5.00
, .zenerdiodeVfromRtoA = 2.48
, .conversionRatioOfCurrentSensor = 1 / SENSITIVITY_OF_20A_CURRENT_SENSOR
};

CELL cells[] =
  // B1(3V7)
{ { .voltageSensor_pin = { .pin_no = A0 }
  , .balanceCircuit_pin = { .pin_no = 2 }
  }
  // B2(7V4)
, { .voltageSensor_pin = { .pin_no = A1 }
  , .balanceCircuit_pin = { .pin_no = 3 }
  }
  // B3(11V1)
, { .voltageSensor_pin = { .pin_no = A2 }
  , .balanceCircuit_pin = { .pin_no = 4 }
  }
};

class BMS {
#ifndef NOT_CONSIDER_SUPPLY_VOLTAGE
  ReaderAnalogPin arduino5V_pin = { .pin_no = A3 };
#endif
#ifndef NOT_CONSIDER_SUPPLY_CURRENT
  ReaderAnalogPin Iin_pin = { .pin_no = A4 };
#endif
  WriterDigitalPin powerIn_pin = { .pin_no = 5 };
  byte lcdOkay = false;
  byte jobsDone = false;
  byte measuredValuesAreFresh = false;
#ifndef NO_LCD_USE
  LiquidCrystal_I2C *lcdHandle = nullptr;
#endif
  V_t arduino5V = refOf.arduinoRegularV;
#ifndef NOT_CONSIDER_SUPPLY_CURRENT
  A_t Iin = 0.0;
#endif
  V_t cellV[LENGTH_OF(cells)] = { };
public:
  void init(ms_t expected_elapsed_time);
  void play(ms_t expected_elapsed_time);
private:
  void controlSystem();
  void measureValues(bool showValues);
  bool checkSafety();
  void execEmergencyMode();
  void goodbye(int timeLeftToQuit);
#ifndef NO_LCD_USE
  bool openLCD(int lcd_width, int lcd_height);
#endif
#ifndef NO_LCD_USE
  void hello();
#endif
} myBMS;

void setup()
{
  do // NOT change the following block:
  {
#ifndef NO_DEBUGGING
    Serial.begin(SERIAL_PORT);
#endif
#ifndef NO_LCD_USE
    Wire.begin();
#endif
  } while (false);
  myBMS.init(1000);
}

void loop()
{
  myBMS.play(3000);
}

void BMS::init(ms_t const given_time)
{
  Timer hourglass;

#ifndef NO_DEBUGGING
  Serial.print("[log] ");
  Serial.println("Runtime started.");
#endif
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
#ifndef NO_LCD_USE
    hello();
#endif
  }
  else
  {
#ifndef NO_DEBUGGING
    Serial.print("[Warning] ");
    Serial.println("LCD not connected.");
#endif
  }
  hourglass.wait(given_time);
}

void BMS::play(ms_t const given_time)
{
  Timer hourglass;

#ifndef NO_DEBUGGING
  Serial.print("[log] ");
  Serial.println("Turn changed.");
#endif
  measureValues(true);
  controlSystem();
  {
    bool system_is_okay = checkSafety();

    if (system_is_okay and jobsDone)
    {
#ifndef NO_DEBUGGING
      Serial.print("[log] ");
      Serial.println("CHARGING COMPLETED.");
#endif
#ifndef NO_LCD_USE
      if (lcdOkay)
      {
        lcdHandle->clear();
        lcdHandle->setCursor(0, 1);
        lcdHandle->print("JOBS FINISHED");
      }
#endif
      goodbye(10);
    }
    else
    {
      for (ms_t remaining_time = given_time - hourglass.getDuration(); remaining_time > 0; remaining_time -= hourglass.getDuration())
      {
        if (not system_is_okay)
        {
          execEmergencyMode();
        }
        system_is_okay = checkSafety();
      }
    }
  }
}

void BMS::controlSystem()
{
  V_t const V_wanted = 3.7, overV_wanted = 4.1; // <- How to calculate these voltages? We must derive them!!!

  if (measuredValuesAreFresh)
  {
    measureValues(false);
  }

#ifndef NOT_CONTROL_BALANCE_CIRCUIT
  jobsDone = true;

  for (int i = 0; i < LENGTH_OF(cellV); i++)
  {
    bool const this_cell_being_charged_now = not cells[i].balanceCircuit_pin.isHigh();
    bool const this_cell_charging_finished = cellV[i] >= (this_cell_being_charged_now ? overV_wanted : V_wanted);

    jobsDone &= this_cell_charging_finished;

    if ((not this_cell_charging_finished) and (not this_cell_being_charged_now))
    {
      cells[i].balanceCircuit_pin.turnOff();
    }

    if ((this_cell_charging_finished) and (this_cell_being_charged_now))
    {
      cells[i].balanceCircuit_pin.turnOn();
    }
  }
#endif

  measuredValuesAreFresh = false;
}

void BMS::measureValues(bool const showValues)
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
  Iin = refOf.conversionRatioOfCurrentSensor * (sensorV - arduino5V * 0.5);
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
    waitForSerial();
#endif
#ifndef NOT_CONSIDER_SUPPLY_CURRENT
    Serial.print(">>> ");
    Serial.print("Iin");
    Serial.print(" = ");
    Serial.print(Iin);
    Serial.println("[A].");
    waitForSerial();
#endif
#endif
#ifndef NO_LCD_USE
    if (lcdOkay)
    {
      LcdPrettyPrinter lcd = { .controllerOfLCD = lcdHandle };

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
      // Here, the variable `lcd` is destructed,
    } //       the LCD screen being updated.
      //                                   A1234567B1234567
      // One of the possible screens is:  ##################
      //                                 1#B1=4.25 B2=4.17 #
      //                                 2#B3=3.33 I=1.66  #
      //                                  ##################
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
      waitForSerial();
    }
#endif
  }
}

bool BMS::checkSafety()
{
  V_t const allowedV_max = 4.20, allowedV_min =  2.70; // CONFIRM US!!!
  A_t const allowedA_max = 2.00, allowedA_min = -0.10; // CONFIRM US!!!
  bool isBad = false;

  if (measuredValuesAreFresh)
  {
    measureValues(false);
  }

  // Check current
#ifndef NOT_CONSIDER_SUPPLY_CURRENT
  if (Iin > allowedA_max)
  {
    isBad = true;
#ifndef NO_DEBUGGING
    Serial.print("[Warning] ");
    Serial.print("`");
    Serial.print("Iin");
    Serial.print("`");
    Serial.print(" too ");
    Serial.println("HIGH.");
#endif
  }
  if (Iin < allowedA_min)
  {
    isBad = true;
#ifndef NO_DEBUGGING
    Serial.print("[Warning] ");
    Serial.print("`");
    Serial.print("Iin");
    Serial.print("`");
    Serial.print(" too ");
    Serial.println("LOW.");
#endif
  }
#endif

  // Check voltages
  for (int i = 0; i < LENGTH_OF(cellV); i++)
  {
    if (cellV[i] > allowedV_max)
    {
      isBad = true;
#ifndef NO_DEBUGGING
      Serial.print("[Warning] ");
      Serial.print("`");
      Serial.print("cellV[");
      Serial.print(i);
      Serial.print("]");
      Serial.print("`");
      Serial.print(" too ");
      Serial.println("HIGH.");
#endif
    }
    if (cellV[i] < allowedV_min)
    {
      isBad = true;
#ifndef NO_DEBUGGING
      Serial.print("[Warning] ");
      Serial.print("`");
      Serial.print("cellV[");
      Serial.print(i);
      Serial.print("]");
      Serial.print("`");
      Serial.print(" too ");
      Serial.println("LOW.");
#endif
    }
  }

#ifndef NO_DEBUGGING
  if (isBad)
  {
    waitForSerial();
  }
#endif
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
  delay(500); // <- This is JUST hiding from the problem.
#ifndef NOT_CONTROL_BALANCE_CIRCUIT
  for (int i = 0; i < LENGTH_OF(cells); i++)
  {
    cells[i].balanceCircuit_pin.turnOff();
  }
#endif
}

void BMS::goodbye(int const countDown)
{
  Timer hourglass;
#ifndef NOT_CONTROL_BALANCE_CIRCUIT
  for (int i = 0; i < LENGTH_OF(cells); i++)
  {
    cells[i].balanceCircuit_pin.turnOn();
  }
#endif
#ifndef NO_LCD_USE
  if (lcdOkay)
  {
    lcdHandle->setCursor(1, 0);
    lcdHandle->print(" SECS LEFT");
  }
#endif
  for (int i = countDown; i > 0; i--)
  {
#ifndef NO_LCD_USE
    if (lcdOkay)
    {
      lcdHandle->setCursor(0, 0);
      lcdHandle->print(i - 1);
    }
#endif
#ifndef NO_DEBUGGING
    Serial.print("[Warning] ");
    Serial.print("Your arduino will abort in ");
    Serial.print(i);
    Serial.println(" seconds.");
#endif
    hourglass.wait(1000);
  }
  powerIn_pin.turnOff();
  abort();
}

#ifndef NO_LCD_USE
bool BMS::openLCD(int const row_dim, int const col_dim)
{
  byte is_good = false;

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
        Serial.print("[log] ");
        Serial.print("I2C ");
        Serial.print("address");
        Serial.print(" found");
        Serial.print(": ");
        Serial.print("address");
        Serial.print(" = ");
        printByteOnSerial(adr);
        Serial.println(".");
#endif
        lcdHandle = new LiquidCrystal_I2C(adr, row_dim, col_dim);
        if (lcdHandle)
        {
#ifndef NO_DEBUGGING
          Serial.print("[log] ");
          Serial.print("I2C ");
          Serial.print("connected");
          Serial.print(": ");
          Serial.print("address");
          Serial.print(" = ");
          printByteOnSerial(adr);
          Serial.println(".");
#endif
          break;
        }
      }
    }

    if (lcdHandle)
    {
      lcdHandle->init();
      lcdHandle->backlight();
      lcdHandle->noCursor();
      is_good = true;
    }
  }

  return is_good;
}
#endif

void BMS::hello()
{
#ifndef NO_LCD_USE
  if (lcdOkay)
  {
    LcdPrettyPrinter lcd = { .controllerOfLCD = lcdHandle };
    
    lcd.println("> SYSTEM");
    lcd.println(" ONLINE");
    lcd.println("VERSION");
    lcd.print("= ");
    lcd.println(VERSION);
  }
#endif
}
