/* <CAPSTONE PROJECT>
** ===============================================================================
** MEMBER        | AFFILIATION                                                   |
** ===============================================================================
** Hwan-hee Jeon | School of Meslogical Engineering, Chonnam National University |
** Hak-jung Im   | School of Meslogical Engineering, Chonnam National University |
** Ki-jeong Lim  | School of Meslogical Engineering, Chonnam National University |
** ===============================================================================
*/

#include "capstone.h"

static constexpr
V_t V_wanted = 4.00, overV_wanted = 4.20; // FIX ME!

static constexpr
ReferenceCollection refOf =
{ .analogSignalMax            = 1024
, .arduinoRegularV            = 5.00
, .batteryCapacity            = 3317
, .zenerdiodeVfromRtoA        = 2.48 // is `Vref` of the zener-diode `TL431BVLPRAGOSCT-ND`.
, .sensitivityOfCurrentSensor = 0.100 // is the sensitivity of the current sensor `ACS712ELCTR-20A-T`.
};

static constexpr
V_t allowedV_max = 4.20, allowedV_min =  2.70; // FIX ME!

static constexpr
A_t allowedA_max = 2.00, allowedA_min = -0.10; // FIX ME!

static
PinsOfCell const cells[] =
#if MODE == 1
{ { .voltage_sensor_pin = { .pinId = A0 }, .BalanceCircuit_pin = { .pinId = 2 } } // B1(3V7)
};
#else
{ { .voltage_sensor_pin = { .pinId = A0 }, .BalanceCircuit_pin = { .pinId = 2 } } // B1(3V7)
, { .voltage_sensor_pin = { .pinId = A1 }, .BalanceCircuit_pin = { .pinId = 3 } } // B2(7V4)
, { .voltage_sensor_pin = { .pinId = A2 }, .BalanceCircuit_pin = { .pinId = 4 } } // B3(11V1)
};
#endif

class BMS {
#if MODE == 1
  PinReader const arduino5V_pin = { .pinId = A1 };
  PinReader const Iin_pin       = { .pinId = A2 };
  PinSetter const powerIn_pin   = { .pinId = 5 };
#else
  PinReader const arduino5V_pin = { .pinId = A3 };
  PinReader const Iin_pin       = { .pinId = A6 };
  PinSetter const powerIn_pin   = { .pinId = 5 };
#endif
  bool jobsDone                 = false;
  bool measuredValuesAreFresh   = false;
  LiquidCrystal_I2C *lcdHandle  = nullptr;
  V_t arduino5V                 = refOf.arduinoRegularV;
  A_t Iin                       = 0.00;
  V_t cellVs[LENGTH_OF(cells)]  = { };
  class CellManager {
    millis_t lastUpdateTime         = 0;
    V_t const *cellVs_ref           = nullptr;
    A_t const *Iin_ref              = nullptr;
    mAh_t charges[LENGTH_OF(cells)] = { };
  public:
    CellManager()
    {
    }
    ~CellManager()
    {
    }
    void init(V_t (*const cellVs_adr)[LENGTH_OF(cellVs)], A_t const *const Iin_adr)
    {
      cellVs_ref = *cellVs_adr;
      Iin_ref = Iin_ref;
      for (int i = 0; i < LENGTH_OF(charges); i++)
      {
        charges[i] = refOf.batteryCapacity * mySocOcvTable.get_x_from_y(cellVs_ref[i]) / 100;
      }
      lastUpdateTime = millis();
    }
    void startCharging(int const cell_no)
    {
      int count_of_batteries_being_charged = 0;
      for (int i = 0; i < LENGTH_OF(cells); i++)
      {
        count_of_batteries_being_charged += not cells[i].BalanceCircuit_pin.isHigh();
      }
      for (int i = 0; i < LENGTH_OF(cells); i++)
      {
        charges[i] += (*Iin_ref / count_of_batteries_being_charged) * (millis() - lastUpdateTime) / 3600;
      }
      lastUpdateTime = millis();
      cells[cell_no].BalanceCircuit_pin.turnOff();
    }
    void breakCharging(int const cell_no)
    {
      int count_of_batteries_being_charged = 0;
      for (int i = 0; i < LENGTH_OF(cells); i++)
      {
        count_of_batteries_being_charged += not cells[i].BalanceCircuit_pin.isHigh();
      }
      for (int i = 0; i < LENGTH_OF(cells); i++)
      {
        charges[i] += (*Iin_ref / count_of_batteries_being_charged) * (millis() - lastUpdateTime) / 3600;
      }
      lastUpdateTime = millis();
      cells[cell_no].BalanceCircuit_pin.turnOn();
    }
    double getSocOf(int const i)
    {
#if 0
      return mySocVcellTable.get_x_from_y(cellVs_ref[i]);
#else
      return (charges[i] / refOf.batteryCapacity) * 100;
#endif
    }
  } manager;
public:
  void initialize(millis_t timeLimit);
  void progress(millis_t timeLimit);
private:
  void measureValues();
  bool checkSafety(bool reportsToSerial);
  void controlSystem();
  void printValues();
  void goodbye(int secsLeftToQuit);
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
  sout << "Run time started.";
  for (int i = 0; i < LENGTH_OF(cells); i++)
  {
    cells[i].BalanceCircuit_pin.initWith(true);
  }
  powerIn_pin.initWith(true);
  manager.init(&cellVs, &Iin);
  lcdHandle = openLcdI2C(LCD_WIDTH, LCD_HEIGHT);
  if (lcdHandle)
  {
    LcdPrinter lcd = { .addressOfLcdI2C = lcdHandle };

    lcd.println("> SYSTEM");
    lcd.println(" ONLINE");
    lcd.println("VERSION");
    lcd.print("= ");
    lcd.println(VERSION);
  }
  else
  {
    serr << "LCD not connected.";
  }
  hourglass.delay(given_time);
}
void BMS::progress(millis_t const given_time)
{
  Timer hourglass;

  measureValues();
  printValues();
  {
    bool system_is_okay = checkSafety(true);

    controlSystem();
    if (system_is_okay and jobsDone)
    {
      sout << "CHARGING COMPLETED.";
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
      while (hourglass.time() < given_time)
      {
        if (not system_is_okay)
        {
          for (int i = 0; i < LENGTH_OF(cells); i++)
          {
            if (cellVs[i] > allowedV_max)
            {
              manager.breakCharging(i);
            }
          }
          delay(500);
        }
        system_is_okay = checkSafety(false);
      }
    }
  }
}
void BMS::measureValues()
{
  constexpr millis_t measuring_time = 10;
  V_t sensorV = 0.00;
  V_t accumV = 0.00;
  // Calculate the voltage of the pin `5V`
  sensorV = refOf.arduinoRegularV * arduino5V_pin.readSignal(measuring_time) / refOf.analogSignalMax;
  arduino5V = refOf.arduinoRegularV * refOf.zenerdiodeVfromRtoA / sensorV;
  // Calculate the voltages of every cell
  for (int i = 0; i < LENGTH_OF(cells); i++)
  {
    constexpr Ohm_t R1 = 18000.0, R2 = 2000.0;
    sensorV = arduino5V * cells[i].voltage_sensor_pin.readSignal(measuring_time) / refOf.analogSignalMax;
    cellVs[i] = (sensorV / (R2 / (R1 + R2))) - accumV;
    accumV += cellVs[i];
  }
  // Calculate the main current
  sensorV = arduino5V * Iin_pin.readSignal(measuring_time) / refOf.analogSignalMax;
  Iin = ((sensorV - 0.5 * arduino5V) / refOf.sensitivityOfCurrentSensor) + 0.04; // `0.04` is a calibration.
  // Guarantee that the above values are fresh
  measuredValuesAreFresh = true;
}
void BMS::printValues()
{
  slog << "arduino5V = " << arduino5V << "[V].";
  slog << "Iin = " << Iin << "[A].";
  for (int i = 0; i < LENGTH_OF(cellVs); i++)
  {
    slog << "cellVs[" << i << "] = " << cellVs[i] << "[V].";
  }
  if (lcdHandle)
  {
    LcdPrinter lcd = { .addressOfLcdI2C = lcdHandle };

    for (int i = 0; i < LENGTH_OF(cellVs); i++)
    {
      double const soc = manager.getSocOf(i);

      lcd.print("B");
      lcd.print(i + 1);
      lcd.print("=");
      lcd.println(cellVs[i]);
      lcd.print(" ");
      lcd.print(soc);
      lcd.println("%");
    }
    lcd.print("I");
    lcd.print("=");
    lcd.println(Iin);
  }
}
bool BMS::checkSafety(bool const reportsToSerial)
{
  bool isBad = false;

  if (measuredValuesAreFresh)
  {
    measureValues();
  }

  // Check current
  if (Iin > allowedA_max)
  {
    isBad = true;
    if (reportsToSerial)
    {
      serr << "`Iin`" << " too HIGH.";
    }
  }
  if (Iin < allowedA_min)
  {
    isBad = true;
    if (reportsToSerial)
    {
      serr << "`Iin`" << " too LOW.";
    }
  }

  // Check voltages
  for (int i = 0; i < LENGTH_OF(cellVs); i++)
  {
    if (cellVs[i] > allowedV_max)
    {
      isBad = true;
      if (reportsToSerial)
      {
        serr << "`cellVs[" << i << "]`" << " too HIGH.";
      }
    }
    if (cellVs[i] < allowedV_min)
    {
      isBad = true;
      if (reportsToSerial)
      {
        serr << "`cellVs[" << i << "]`" << " too LOW.";
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
    measureValues();
  }

  jobsDone = true;
  for (int i = 0; i < LENGTH_OF(cellVs); i++)
  {
    bool const is_this_cell_being_charged_now = not cells[i].BalanceCircuit_pin.isHigh();
    bool const is_this_cell_fully_charged_now = cellVs[i] >= (is_this_cell_being_charged_now ? overV_wanted : V_wanted);

    jobsDone &= is_this_cell_fully_charged_now;

    if ((not is_this_cell_fully_charged_now) and (not is_this_cell_being_charged_now))
    {
      manager.startCharging(i);
    }
    if ((is_this_cell_fully_charged_now) and (is_this_cell_being_charged_now))
    {
      manager.breakCharging(i);
    }
  }

  measuredValuesAreFresh = false;
}
void BMS::goodbye(int const countDown)
{
  Timer hourglass;

  for (int i = 0; i < LENGTH_OF(cells); i++)
  {
    cells[i].BalanceCircuit_pin.turnOn();
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
    serr << "Your arduino will abort in " << i << " seconds.";
    hourglass.delay(1000);
    hourglass.reset();
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

SerialPrinter sout = { .prefix = "arduino> " };
SerialPrinter serr = { .prefix = "WARNING> " };
SerialPrinter slog = { .prefix = "       > " };
