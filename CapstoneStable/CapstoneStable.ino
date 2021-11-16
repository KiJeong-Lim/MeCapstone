/* <CAPSTONE PROJECT>
** ===============================================================================
** MEMBER        | AFFILIATION                                                   |
** ===============================================================================
** Hwan-hee Jeon | School of Mechanical Engineering, Chonnam National University |
** Hak-jung Im   | School of Mechanical Engineering, Chonnam National University |
** Ki-jeong Lim  | School of Mechanical Engineering, Chonnam National University |
** ===============================================================================
*/

#include "header.hpp"

constexpr V_t V_wanted = 3.60, overV_wanted = 4.20; // FIX ME!
constexpr ReferenceCollection refOf =
{ .analogSignalMax                  = 1024
, .arduinoRegularV                  = 5.00
, .zenerdiodeVfromRtoA              = 2.48 // is `Vref` of the zener-diode `TL431BVLPRAGOSCT-ND`.
, .sensitivityOfCurrentSensor       = 0.100 // is the sensitivity of the current sensor `ACS712ELCTR-20A-T`.
};
constexpr V_t allowedV_max = 4.20, allowedV_min =  2.70; // FIX ME!
constexpr A_t allowedA_max = 2.00, allowedA_min = -0.10; // FIX ME!

static
CELL const cells[] =
#if MODE == 1
{ { .voltageSensor_pin = { .pin_no = A0 }, .balanceCircuit_pin = { .pin_no = 2 } } // B1(3V7)
};
#else
{ { .voltageSensor_pin = { .pin_no = A0 }, .balanceCircuit_pin = { .pin_no = 2 } } // B1(3V7)
, { .voltageSensor_pin = { .pin_no = A1 }, .balanceCircuit_pin = { .pin_no = 3 } } // B2(7V4)
, { .voltageSensor_pin = { .pin_no = A2 }, .balanceCircuit_pin = { .pin_no = 4 } } // B3(11V1)
};
#endif

class BMS {
#if MODE == 1
  ReaderAnalogPin const arduino5V_pin = { .pin_no = A1 };
  ReaderAnalogPin const Iin_pin       = { .pin_no = A2 };
  WriterDigitalPin const powerIn_pin  = { .pin_no = 5 };
#else
  ReaderAnalogPin const arduino5V_pin = { .pin_no = A3 };
  ReaderAnalogPin const Iin_pin       = { .pin_no = A6 };
  WriterDigitalPin const powerIn_pin  = { .pin_no = 5 };
#endif
  bool jobsDone                       = false;
  bool measuredValuesAreFresh         = false;
  LiquidCrystal_I2C *lcdHandle        = nullptr;
  V_t arduino5V                       = refOf.arduinoRegularV;
  A_t Iin                             = 0.00;
  V_t cellV[LENGTH_OF(cells)]         = { };
public:
  void initialize(millis_t timeLimit);
  void progress(millis_t timeLimit);
private:
  void measureValues(bool showValues);
  bool checkSafety(bool reportsToSerial);
  void controlSystem();
  V_t calcOCV(int cell_no) const; // FIX ME!
  void goodbye(int timeLeftToQuit);
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

void BMS::initialize(millis_t const given_time)
{
  Timer hourglass;

  Wire.begin();
  cout << "Run time started.";
  for (int i = 0; i < LENGTH_OF(cells); i++)
  {
    cells[i].balanceCircuit_pin.initWith(true);
  }
  powerIn_pin.initWith(true);
  lcdHandle = openLcdI2C();
  if (lcdHandle)
  {
    LcdPrettyPrinter lcd = { .controllerOfLCD = lcdHandle };

    lcd.println("> SYSTEM");
    lcd.println(" ONLINE");
    lcd.println("VERSION");
    lcd.print("= ");
    lcd.println(VERSION);
  }
  else
  {
    cerr << "LCD not connected.";
  }
  hourglass.wait(given_time);
}
void BMS::progress(millis_t const given_time)
{
  Timer hourglass;

  measureValues(true);
  {
    bool system_is_okay = checkSafety(true);

    controlSystem();
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
      while (hourglass.getDuration() < given_time)
      {
        if (not system_is_okay)
        {
          for (int i = 0; i < LENGTH_OF(cells); i++)
          {
            if (cellV[i] > allowedV_max)
            {
              cells[i].balanceCircuit_pin.turnOn();
            }
          }
          delay(500);
        }
        system_is_okay = checkSafety(false);
      }
    }
  }
}
void BMS::measureValues(bool const showValues)
{
  constexpr millis_t measuring_time_for_one_sensor = 10;
  V_t sensorV = 0.00;
  V_t accumV = 0.00;
  // Calculate the voltage of the pin `5V`
  sensorV = refOf.arduinoRegularV * arduino5V_pin.readSignal(measuring_time_for_one_sensor) / refOf.analogSignalMax;
  arduino5V = refOf.arduinoRegularV * refOf.zenerdiodeVfromRtoA / sensorV;
  // Calculate the voltages of every cell
  for (int i = 0; i < LENGTH_OF(cells); i++)
  {
    constexpr Ohm_t R1 = 18000.0, R2 = 2000.0;
    sensorV = arduino5V * cells[i].voltageSensor_pin.readSignal(measuring_time_for_one_sensor) / refOf.analogSignalMax;
    cellV[i] = (sensorV / (R2 / (R1 + R2))) - accumV;
    accumV += cellV[i];
  }
  // Calculate the main current
  sensorV = arduino5V * Iin_pin.readSignal(measuring_time_for_one_sensor) / refOf.analogSignalMax;
  Iin = ((sensorV - 0.5 * arduino5V) / refOf.sensitivityOfCurrentSensor) + 0.04; // `0.04` is a calibration.
  // Guarantee that the above values are fresh
  measuredValuesAreFresh = true;

  if (showValues)
  {
    chan << "arduino5V = " << arduino5V << "[V].";
    chan << "Iin = " << Iin << "[A].";
    for (int i = 0; i < LENGTH_OF(cellV); i++)
    {
      chan << "cellV[" << i << "] = " << cellV[i] << "[V].";
    }
    if (lcdHandle)
    {
      LcdPrettyPrinter lcd = { .controllerOfLCD = lcdHandle };

      for (int cell_no = 1; cell_no <= LENGTH_OF(cellV); cell_no++)
      {
        V_t const ocv = calcOCV(cell_no);
        double const soc = mySocOcvTable.get_x_from_y(ocv); // 0.00 ~ 100.00

        lcd.print("B");
        lcd.print(cell_no);
        lcd.print("=");
        lcd.println(cellV[cell_no - 1]);
        lcd.print(" ");
        lcd.print(soc);
        lcd.println("%");
      }
      lcd.print("I");
      lcd.print("=");
      lcd.println(Iin);
    }
  }
}
bool BMS::checkSafety(bool const reportsToSerial)
{
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
      if (reportsToSerial)
      {
        cerr << "`cellV[" << i << "]`" << " too HIGH.";
      }
    }
    if (cellV[i] < allowedV_min)
    {
      isBad = true;
      if (reportsToSerial)
      {
        cerr << "`cellV[" << i << "]`" << " too LOW.";
      }
    }
  }

  measuredValuesAreFresh = false;
  return isBad;
}
void BMS::controlSystem()
{
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
V_t BMS::calcOCV(int const cell_no) const
{
  V_t const focused_cellV = cellV[cell_no - 1];
  bool const focused_cell_is_being_charged = not cells[cell_no - 1].balanceCircuit_pin.isHigh();

  return focused_cellV;
}
void BMS::goodbye(int const countDown)
{
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
    Timer hourglass;
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

SerialPrinter cout = { .prefix = "message> " };
SerialPrinter cerr = { .prefix = "WARNING> " };
SerialPrinter chan = { .prefix = "Arduino> " };
