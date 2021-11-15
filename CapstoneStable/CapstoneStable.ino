/* <CAPSTONE PROJECT>
** ===============================================================================
** MEMBERS       | AFFILIATION                                                   |
** ===============================================================================
** Hwan-hee Jeon | School of Mechanical Engineering, Chonnam National University |
** Hak-jung Im   | School of Mechanical Engineering, Chonnam National University |
** Ki-jeong Lim  | School of Mechanical Engineering, Chonnam National University |
** ===============================================================================
*/

#include "header.h"

constexpr
ReferenceCollection refOf =
{ .analogSignalMax = 1024.0
, .arduinoRegularV = 5.00
, .zenerdiodeVfromRtoA = 2.48
, .conversionRatioOfCurrentSensor = 1 / SENSITIVITY_OF_20A_CURRENT_SENSOR
};

static
CELL const cells[] =
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
  ReaderAnalogPin const arduino5V_pin = { .pin_no = A3 };
  ReaderAnalogPin const Iin_pin = { .pin_no = A6 };
  WriterDigitalPin const powerIn_pin = { .pin_no = 5 };
  bool jobsDone = false;
  bool measuredValuesAreFresh = false;
  LiquidCrystal_I2C *lcdHandle = nullptr;
  V_t arduino5V = refOf.arduinoRegularV;
  A_t Iin = 0.0;
  V_t cellV[LENGTH_OF(cells)] = { };
public:
  void initialize(ms_t timeLimit);
  void progress(ms_t timeLimit);
private:
  void controlSystem();
  void measureValues(bool showValues);
  bool checkSafety(bool reportsToSerial);
  void goodbye(int timeLeftToQuit);
  void hello();
} myBMS;

void setup()
{
#if defined(SERIAL_PORT)
  Serial.begin(SERIAL_PORT);
#endif
  myBMS.initialize(2000);
}

void loop()
{
#if defined(SERIAL_PORT)
  Serial.println("=======");
#endif
  myBMS.progress(2000);
}

void BMS::initialize(ms_t const given_time)
{
  Timer hourglass;

  cout << "Run time started.";
  for (int i = 0; i < LENGTH_OF(cells); i++)
  {
    cells[i].balanceCircuit_pin.initWith(true);
  }
  powerIn_pin.initWith(true);
  lcdHandle = openLcdI2C();
  if (lcdHandle)
  {
    hello();
  }
  else
  {
    cerr << "LCD not connected.";
  }
  hourglass.wait(given_time);
}
void BMS::progress(ms_t const given_time)
{
  Timer hourglass;

  measureValues(true);
  controlSystem();
  {
    bool system_is_okay = checkSafety(true);

    if (system_is_okay and jobsDone)
    {
      cout << "CHARGING COMPLETED.";
      if (lcdHandle)
      {
        lcdHandle->clear();
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
  constexpr V_t V_wanted = 3.6, overV_wanted = 3.6; // <- How to calculate these voltages? We must derive them!!!

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
  constexpr ms_t measuring_time_for_one_sensor = 10;
  V_t sensorV = 0.0;
  V_t accumV = 0.0;
  // Calculate the voltage of the pin `5V`
  sensorV = refOf.arduinoRegularV * arduino5V_pin.readSignal(measuring_time_for_one_sensor) / refOf.analogSignalMax;
  arduino5V = refOf.arduinoRegularV * refOf.zenerdiodeVfromRtoA / sensorV;
  // Calculate the voltage of the cells
  for (int i = 0; i < LENGTH_OF(cells); i++)
  {
    constexpr Ohm_t R1 = 18000.0, R2 = 2000.0;
    sensorV = arduino5V * cells[i].voltageSensor_pin.readSignal(measuring_time_for_one_sensor) / refOf.analogSignalMax;
    cellV[i] = (sensorV / (R2 / (R1 + R2))) - accumV;
    accumV += cellV[i];
  }
  // Calculate the main current
  sensorV = arduino5V * Iin_pin.readSignal(measuring_time_for_one_sensor) / refOf.analogSignalMax;
  Iin = refOf.conversionRatioOfCurrentSensor * (sensorV - 0.5 * arduino5V) + 0.04; // `0.04` is a calibration.
  // Guarantee the above values are fresh
  measuredValuesAreFresh = true;

  if (showValues)
  {
    chan << "arduino5V = " << arduino5V << "[V].";
    chan << "Iin = " << Iin << "[A].";
    if (lcdHandle)
    {
      LcdPrettyPrinter lcd = { .controllerOfLCD = lcdHandle };

      for (int i = 0; i < LENGTH_OF(cellV); i++)
      {
        lcd.print("B");
        lcd.print(i + 1);
        lcd.print("=");
        lcd.println(cellV[i]);
      }
      lcd.print("I");
      lcd.print("=");
      lcd.println(Iin);
      // Here, the variable `lcd` is destructed,
    } //       the LCD screen being updated.
      //                                   A1234567B1234567
      // One of the possible screens is:  ##################
      //                                 0#B1=4.25 B2=4.17 #
      //                                 1#B3=3.33 I=1.66  #
      //                                  ##################
    for (int i = 0; i < LENGTH_OF(cellV); i++)
    {
      chan << "cellV[" << i << "] = " << cellV[i] << "[V].";
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
      cerr << "`Iin`" << " too HIGH.";
    }
  }
  if (Iin < allowedA_min)
  {
    isBad = true;
    if (reportsToSerial)
    {
      cerr << "`Iin`" << " too LOW.";
    }
  }

  // Check voltages
  for (int i = 0; i < LENGTH_OF(cellV); i++)
  {
    if (cellV[i] > allowedV_max)
    {
      isBad = true;
      cerr << "`cellV[" << i << "]`" << " too HIGH.";
    }
    if (cellV[i] < allowedV_min)
    {
      isBad = true;
      cerr << "`cellV[" << i << "]`" << " too LOW.";
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
    cerr << "Your arduino will abort in " << i << " seconds.";
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

SerialPrinter const cout = { .prefix = "       > " };

SerialPrinter const cerr = { .prefix = "WARNING> " };

SerialPrinter const chan = { .prefix = "Arduino> " };
