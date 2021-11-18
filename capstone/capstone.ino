/* <CAPSTONE PROJECT>
** ===============================================================================
** MEMBER        | AFFILIATION                                                   |
** ===============================================================================
** Hwan-hee Jeon | School of Meslogical Engineering, Chonnam National University |
** Hak-jung Im   | School of Meslogical Engineering, Chonnam National University |
** Ki-jeong Lim  | School of Meslogical Engineering, Chonnam National University |
** ===============================================================================
*/

#include "capstone.hpp"

static constexpr
V_t V_wanted = 4.00, overV_wanted = 4.20; // FIX ME!

static constexpr
ReferenceCollection refOf =
{ .analogSignalMax            = 1024
, .arduinoRegularV            = 5.00
, .batteryCapacity            = 3317
, .sensitivityOfCurrentSensor = .100 // is the sensitivity of the current sensor `ACS712ELCTR-20A-T`.
, .zenerdiodeVfromRtoA        = 2.48 // is `Vref` of the zener-diode `TL431BVLPRAGOSCT-ND`.
};

static constexpr
V_t allowedV_max = +4.20, allowedV_min = +2.70; // FIX ME!

static constexpr
A_t allowedA_max = +2.00, allowedA_min = -0.10; // FIX ME!

static
PinsOfCell const cells[] =
#if( MODE == 1 )
{ { .voltage_sensor_pin = { .pinId = Apin(0) }, .BalanceCircuit_pin = { .pinId = Dpin(2) } } // B1(3V7)
#else
{ { .voltage_sensor_pin = { .pinId = Apin(0) }, .BalanceCircuit_pin = { .pinId = Dpin(2) } } // B1(3V7)
, { .voltage_sensor_pin = { .pinId = Apin(1) }, .BalanceCircuit_pin = { .pinId = Dpin(3) } } // B2(7V4)
, { .voltage_sensor_pin = { .pinId = Apin(2) }, .BalanceCircuit_pin = { .pinId = Dpin(4) } } // B3(11V1)
#endif
};

class BMS {
#if( MODE == 1 )
  PinReader const arduino5V_pin = { .pinId = Apin(1) };
  PinReader const Iin_pin       = { .pinId = Apin(2) };
  PinSetter const powerIn_pin   = { .pinId = Dpin(5) };
#else
  PinReader const arduino5V_pin = { .pinId = Apin(3) };
  PinReader const Iin_pin       = { .pinId = Apin(6) };
  PinSetter const powerIn_pin   = { .pinId = Dpin(5) };
#endif
  bool jobsDone                 = false;
  bool measuredValuesAreFresh   = false;
  ms_t Qs_lastUpdatedTime       = 0;
  V_t arduino5V                 = refOf.arduinoRegularV;
  A_t Iin                       = 0.00;
  V_t cellVs[LENGTH_OF(cells)]  = { };
  mAh_t Qs[LENGTH_OF(cells)]    = { };
  LiquidCrystal_I2C *lcdHandle  = nullptr;
public:
  void initialize(ms_t timeLimit);
  void progress(ms_t timeLimit);
private:
  void measureValues();
  void initQs();
  void updateQs();
  double checkSocOf(int cell_no) const;
  void printValues() const;
  void startCharging(int cell_no);
  void breakCharging(int cell_no);
  bool checkSafety(bool reportsToSerial);
  void controlSystem();
  void goodbye(int secsLeftToQuit);
} myBMS;

void setup()
{
#if defined(SERIAL_PORT)
  Serial.begin(SERIAL_PORT);
#endif
  myBMS.initialize(3000);
}

void loop()
{
#if defined(SERIAL_PORT)
  Serial.println("=======");
#endif
  myBMS.progress(3000);
}

void BMS::initialize(ms_t const given_time)
{
  Timer hourglass;

  Wire.begin();
  sout << "Run time started.";
  for (int i = 0; i < LENGTH_OF(cells); i++)
  {
    cells[i].BalanceCircuit_pin.initWith(true);
  }
  powerIn_pin.initWith(true);
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
  initQs();
}
void BMS::progress(ms_t const given_time)
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
              breakCharging(i);
            }
          }
          delay(100);
        }
        system_is_okay = checkSafety(false);
      }
    }
  }
  updateQs();
}
void BMS::measureValues()
{
  constexpr ms_t measuring_time = 10;
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
void BMS::initQs()
{
  if (not measuredValuesAreFresh)
  {
    measureValues();
  }
  for (int i = 0; i < LENGTH_OF(Qs); i++)
  {
    Qs[i] = refOf.batteryCapacity * (mySocOcvTable.get_x_from_y(cellVs[i]) / 100);
  }
  Qs_lastUpdatedTime = millis();
}
void BMS::updateQs()
{
  int number_of_batteries_being_charged = 0;
  if (not measuredValuesAreFresh)
  {
    measureValues();
  }
  for (int i = 0; i < LENGTH_OF(cells); i++)
  {
    number_of_batteries_being_charged += not cells[i].BalanceCircuit_pin.isHigh();
  }
  for (int i = 0; i < LENGTH_OF(cells); i++)
  {
    Qs[i] += (Iin / number_of_batteries_being_charged) * (millis() - Qs_lastUpdatedTime) / 3600;
  }
  Qs_lastUpdatedTime = millis();
}
double BMS::checkSocOf(int const cell_no) const
{
#if( 0 )
  return mySocVcellTable.get_x_from_y(cellVs[cell_no]);
#else
  return (Qs[cell_no] / refOf.batteryCapacity) * 100;
#endif
}
void BMS::printValues() const
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
      double const soc = checkSocOf(i);

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
void BMS::startCharging(int const cell_no)
{
  updateQs();
  cells[cell_no].BalanceCircuit_pin.turnOff();
}
void BMS::breakCharging(int const cell_no)
{
  updateQs();
  cells[cell_no].BalanceCircuit_pin.turnOn();
}
bool BMS::checkSafety(bool const reportsToSerial)
{
  bool isBad = false;

  if (not measuredValuesAreFresh)
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
  if (not measuredValuesAreFresh)
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
      startCharging(i);
    }
    if ((is_this_cell_fully_charged_now) and (is_this_cell_being_charged_now))
    {
      breakCharging(i);
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
