/* 2021-11-14 <CAPSTONE PROJECT>
** ===============================================================================
** MEMBERS       | AFFILIATION                                                   |
** ===============================================================================
** Hwan-hee Jeon | School of Mechanical Engineering, Chonnam National University |
** Hak-jung Im   | School of Mechanical Engineering, Chonnam National University |
** Ki-jeong Lim  | School of Mechanical Engineering, Chonnam National University |
** ===============================================================================
*/

#include "header.h"

SerialPrinter const consoleLog =
{ .prefix = "    [log] "
};

SerialPrinter const alert =
{ .prefix = "[Warning] "
};

SerialPrinter const shell =
{ .prefix = "*Arduino> "
};

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
  ReaderAnalogPin arduino5V_pin = { .pin_no = A1 };
  ReaderAnalogPin Iin_pin = { .pin_no = A2 };
  WriterDigitalPin powerIn_pin = { .pin_no = 5 };
  byte jobsDone = false;
  byte measuredValuesAreFresh = false;
  LiquidCrystal_I2C *lcdHandle = nullptr;
  V_t arduino5V = refOf.arduinoRegularV;
  A_t Iin = 0.0;
  V_t cellV[LENGTH_OF(cells)] = { };
public:
  void initialize(ms_t timeLimit);
  void progressing(ms_t timeLimit);
private:
  void controlSystem();
  void measureValues(bool showValues);
  bool checkSafety(bool reportsToSerial);
  void goodbye(int timeLeftToQuit);
  void hello();
} myBMS;

void setup() {
#if defined(SERIAL_PORT)
  Serial.begin(SERIAL_PORT);
#endif
  myBMS.initialize(1000);
}

void loop() {
#if defined(SERIAL_PORT)
  Serial.println("=========");
#endif
  myBMS.progressing(2000);
}

void BMS::initialize(ms_t const given_time)
{
  Timer hourglass;

  consoleLog << "Run time started.";
  for (int i = 0; i < LENGTH_OF(cells); i++)
  {
    cells[i].balanceCircuit_pin.initWith(true);
  }
  powerIn_pin.initWith(true);
  lcdHandle = openLcdI2C(LCD_WIDTH, LCD_HEIGHT);
  if (lcdHandle)
  {
    hello();
  }
  else
  {
    alert << "LCD not connected.";
  }
  hourglass.wait(given_time);
}

void BMS::progressing(ms_t const given_time)
{
  Timer hourglass;

  measureValues(true);
  controlSystem();
  {
    bool system_is_okay = checkSafety(true);

    if (system_is_okay and jobsDone)
    {
      consoleLog << "CHARGING COMPLETED.";
      if (lcdHandle)
      {
        lcdHandle->clear();
        lcdHandle->noBacklight();
        lcdHandle->backlight();
        lcdHandle->setCursor(0, 1);
        lcdHandle->print("JOBS FINISHED");
      }
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

void BMS::measureValues(bool const showValues) // OKAY
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
  Iin = refOf.conversionRatioOfCurrentSensor * (sensorV - 0.5 * arduino5V) + 0.04;
  measuredValuesAreFresh = true;

  if (showValues)
  {
    shell << "arduino5V = " << Iin << "[V].";
    shell << "Iin = " << Iin << "[A].";
    if (lcdHandle)
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
      shell << "cellV[" << i << "] = " << cellV[i] << "[V].";
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
  if (Iin > allowedA_max)
  {
    isBad = true;
    if (reportsToSerial)
    {
      alert << "`Iin`" << " too " << "HIGH.";
    }
  }
  if (Iin < allowedA_min)
  {
    isBad = true;
    if (reportsToSerial)
    {
      alert << "`Iin` too " << "LOW.";
    }
  }

  // Check voltages
  for (int i = 0; i < LENGTH_OF(cellV); i++)
  {
    if (cellV[i] > allowedV_max)
    {
      isBad = true;
      alert << "`cellV[" << i << "]` too " << "HIGH.";
    }
    if (cellV[i] < allowedV_min)
    {
      isBad = true;
      alert << "`cellV[" << i << "]` too " << "LOW.";
    }
  }

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
  if (lcdHandle)
  {
    lcdHandle->setCursor(1, 0);
    lcdHandle->print(" SECS LEFT");
  }
  for (int i = countDown; i > 0; i--)
  {
    if (lcdHandle)
    {
      lcdHandle->setCursor(0, 0);
      lcdHandle->print(i - 1);
    }
    alert << "Your arduino will abort in " << i << " seconds.";
    hourglass.wait(1000);
  }
  if (lcdHandle)
  {
    lcdHandle->clear();
    lcdHandle->noBacklight();
    delete lcdHandle;
    lcdHandle = nullptr;
  }
  powerIn_pin.turnOff();
  abort();
}

void BMS::hello()
{
  if (lcdHandle)
  {
    LcdPrettyPrinter lcd = { .controllerOfLCD = lcdHandle };
    
    lcd.println("> SYSTEM");
    lcd.println(" ONLINE");
    lcd.println("VERSION");
    lcd.print("= ");
    lcd.println(VERSION);
  }
}
