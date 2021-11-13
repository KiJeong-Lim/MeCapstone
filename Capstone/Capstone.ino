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
#define WITH_UNO
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
};

class BMS {
  ReaderAnalogPin arduino5V_pin = { .pin_no = A1 };
  ReaderAnalogPin Iin_pin = { .pin_no = A2 };
  WriterDigitalPin powerIn_pin = { .pin_no = 5 };
  byte lcdOkay = false;
  byte jobsDone = false;
  byte measuredValuesAreFresh = false;
#ifndef NO_LCD_USE
  LiquidCrystal_I2C *lcdHandle = nullptr;
#endif
  V_t arduino5V = refOf.arduinoRegularV;
  A_t Iin = 0.0;
  V_t cellV[LENGTH_OF(cells)] = { };
public:
  void init(ms_t timeLimit);
  void play(ms_t timeLimit);
private:
  void controlSystem();
  void measureValues(bool showValues);
  bool checkSafety(bool reportsToSerial);
  void goodbye(int timeLeftToQuit);
#ifndef NO_LCD_USE
  bool openLCD(int lcdWidth, int lcdHeight);
#endif
  void hello();
} myBMS;

void setup()
{
  do // NOT change the following block:
  {
#ifndef NO_DEBUGGING
    Serial.begin(SERIAL_PORT_NUMBER);
#endif
#ifndef NO_LCD_USE
    Wire.begin();
#endif
  } while (false);
  myBMS.init(1000);
}

void loop()
{
  myBMS.play(2000);
}

void BMS::init(ms_t const given_time)
{
  Timer hourglass;

#ifndef NO_DEBUGGING
  Serial.print("    [log] ");
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
  Serial.println("=========");
#endif
  measureValues(true);
  controlSystem();
  {
    bool system_is_okay = checkSafety(true);

    if (system_is_okay and jobsDone)
    {
#ifndef NO_DEBUGGING
      Serial.print("    [log] ");
      Serial.println("CHARGING COMPLETED.");
#endif
#ifndef NO_LCD_USE
      if (lcdOkay)
      {
        lcdHandle->clear();
        lcdHandle->noBacklight();
        lcdHandle->backlight();
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
          delay(100);
        }
        system_is_okay = checkSafety(false);
      }
    }
  }
}

void BMS::controlSystem()
{
  V_t const V_wanted = 3.6, overV_wanted = 3.6; // <- How to calculate these voltages? We must derive them!!!

  if (measuredValuesAreFresh)
  {
    measureValues(false);
  }

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
  measuredValuesAreFresh = false;
}

void BMS::measureValues(bool const showValues)
{
  ms_t const measuring_time_for_one_sensor = 10;
  V_t sensorV = 0.0;
  V_t accumV = 0.0;

  sensorV = refOf.arduinoRegularV * arduino5V_pin.readSignal(measuring_time_for_one_sensor) / refOf.analogSignalMax;
  arduino5V = refOf.arduinoRegularV * refOf.zenerdiodeVfromRtoA / sensorV;
  for (int i = 0; i < LENGTH_OF(cells); i++)
  {
    Ohm_t const R1 = 18000.0, R2 = 2000.0;
    sensorV = arduino5V * cells[i].voltageSensor_pin.readSignal(measuring_time_for_one_sensor) / refOf.analogSignalMax;
    cellV[i] = (sensorV / (R2 / (R1 + R2))) - accumV;
    accumV += cellV[i];
  }
  sensorV = arduino5V * Iin_pin.readSignal(measuring_time_for_one_sensor) / refOf.analogSignalMax;
  Iin = refOf.conversionRatioOfCurrentSensor * (sensorV - 0.5 * arduino5V);
  measuredValuesAreFresh = true;

  if (showValues)
  {
    Serial.print("*Arduino> ");
    Serial.print("arduino5V");
    Serial.print(" = ");
    Serial.print(arduino5V);
    Serial.println("[V].");
    waitForSerial();
    Serial.print("*Arduino> ");
    Serial.print("Iin");
    Serial.print(" = ");
    Serial.print(Iin);
    Serial.println("[A].");
    waitForSerial();
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
      lcd.print("I");
      lcd.print("=");
      lcd.print(Iin);
      lcd.println(" ");
      // Here, the variable `lcd` is destructed,
    } //       the LCD screen being updated.
      //                                   A1234567B1234567
      // One of the possible screens is:  ##################
      //                                 1#B1=4.25 B2=4.17 #
      //                                 2#B3=3.33 I=1.66  #
      //                                  ##################
    for (int i = 0; i < LENGTH_OF(cellV); i++)
    {
      Serial.print("*Arduino> ");
      Serial.print("cellV[");
      Serial.print(i);
      Serial.print("]");
      Serial.print(" = ");
      Serial.print(cellV[i]);
      Serial.println("[V].");
      waitForSerial();
    }
  }
}

bool BMS::checkSafety(bool const reportsToSerial)
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
    if (reportsToSerial)
    {
      Serial.print("[Warning] ");
      Serial.print("`");
      Serial.print("Iin");
      Serial.print("`");
      Serial.print(" too ");
      Serial.println("HIGH.");
    }
#endif
  }
  if (Iin < allowedA_min)
  {
    isBad = true;
#ifndef NO_DEBUGGING
    if (reportsToSerial)
    {
      Serial.print("[Warning] ");
      Serial.print("`");
      Serial.print("Iin");
      Serial.print("`");
      Serial.print(" too ");
      Serial.println("LOW.");
    }
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
      if (reportsToSerial)
      {
        Serial.print("[Warning] ");
        Serial.print("`");
        Serial.print("cellV[");
        Serial.print(i);
        Serial.print("]");
        Serial.print("`");
        Serial.print(" too ");
        Serial.println("HIGH.");
      }
#endif
    }
    if (cellV[i] < allowedV_min)
    {
      isBad = true;
#ifndef NO_DEBUGGING
      if (reportsToSerial)
      {
        Serial.print("[Warning] ");
        Serial.print("`");
        Serial.print("cellV[");
        Serial.print(i);
        Serial.print("]");
        Serial.print("`");
        Serial.print(" too ");
        Serial.println("LOW.");
      }
#endif
    }
  }

#ifndef NO_DEBUGGING
  if (isBad)
  {
    if (reportsToSerial)
    {
      waitForSerial();
    }
  }
#endif
  measuredValuesAreFresh = false;
  return isBad;
}

void BMS::goodbye(int const countDown)
{
  Timer hourglass;
  for (int i = 0; i < LENGTH_OF(cells); i++)
  {
    cells[i].balanceCircuit_pin.turnOn();
  }
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
  bool is_good = false;

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
#ifndef NO_DEBUGGING
        Serial.print("    [log] ");
        Serial.print("I2C ");
        Serial.print("address");
        Serial.print(" found ");
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
          Serial.print("    [log] ");
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
      adr--;
    } while (adr != 0x00);

    if (lcdHandle)
    {
      lcdHandle->init();
      lcdHandle->backlight();
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
