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
Vol_t V_wanted = 4.00, overV_wanted = 4.10; // FIX ME!

static constexpr
ReferenceCollection refOf =
{ .analogSignalMax            = 1024
, .arduinoRegularV            = 5.00
, .batteryCapacity            = 3317
, .sensitivityOfCurrentSensor = .100 // is the sensitivity of the current sensor `ACS712ELCTR-20A-T`.
, .zenerdiodeVfromRtoA        = 2.48 // is `Vref` of the zener-diode `TL431BVLPRAGOSCT-ND`.
};

static constexpr
Vol_t allowedV_max = +4.20, allowedV_min = +2.70; // FIX ME!

static constexpr
Amp_t allowedA_max = +2.00, allowedA_min = -0.10; // FIX ME!

static
PinsOfCell const cells[] =
#if( OPERATING_MODE == 1 )
{ { .voltage_sensor_pin = { .pinId = Apin(0) }, .BalanceCircuit_pin = { .pinId = Dpin(2) } } // B1(3V7)
#else
{ { .voltage_sensor_pin = { .pinId = Apin(0) }, .BalanceCircuit_pin = { .pinId = Dpin(2) } } // B1(3V7)
, { .voltage_sensor_pin = { .pinId = Apin(1) }, .BalanceCircuit_pin = { .pinId = Dpin(3) } } // B2(7V4)
, { .voltage_sensor_pin = { .pinId = Apin(2) }, .BalanceCircuit_pin = { .pinId = Dpin(4) } } // B3(11V1)
#endif
};

class BMS {
#if( OPERATING_MODE == 1 )
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
  Timer Qs_lastUpdatedTime      = { .init_time = 0 };
  LcdHandle_t lcd_handle        = nullptr;
  Vol_t arduino5V               = refOf.arduinoRegularV;
  Amp_t Iin                     = 0.00;
  Amp_t Iin_calibration         = 0.00;
  Vol_t cellVs[LENGTH(cells)]   = { };
  mAh_t Qs[LENGTH(cells)]       = { };
public:
  auto initialize(ms_t time_limit) -> void;
  auto progress(ms_t time_limit) -> void;
  auto getCalibrationOfIin() const -> Amp_t;
  auto measureValues() -> void;
  auto findQs_0() -> void;
  auto updateQs() -> void;
  auto getSocOf(int cell_no) const -> double;
  auto printValues() const -> void;
  auto startCharging(int cell_no) -> void;
  auto breakCharging(int cell_no) -> void;
  auto checkSafety(bool reports_to_serial) -> bool;
  auto controlSystem() -> void;
  auto goodbye(int seconds_left_to_quit) -> void;
} myBMS;

void setup()
{
  myBMS.initialize(3000);
}

void loop()
{
  myBMS.progress(3000);
}

void BMS::initialize(ms_t const given_time)
{
  Timer hourglass = { };

  invokingSerial();
  sout << "Run time started.";
  Wire.begin();
  for (int i = 0; i < LENGTH(cells); i++)
  {
    cells[i].BalanceCircuit_pin.initWith(true);
  }
  powerIn_pin.initWith(false);
  delay(5);
  Iin_calibration = getCalibrationOfIin();
  sout << "Iin_calibration = " << Iin_calibration << "[A].";
  lcd_handle = openLcdI2C(LCD_WIDTH, LCD_HEIGHT);
  if (lcd_handle)
  {
    LcdPrinter lcd = { .lcdHandleRef = lcd_handle };

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
  findQs_0();
  powerIn_pin.turnOn();
}
void BMS::progress(ms_t const given_time)
{
  Timer hourglass = { };

  drawlineSerial();
  measureValues();
  printValues();
  {
    bool system_is_okay = checkSafety(true);

    controlSystem();
    if (system_is_okay and jobsDone)
    {
      sout << "CHARGING COMPLETED.";
      if (lcd_handle)
      {
        lcd_handle->clear();
        lcd_handle->setCursor(0, 1);
        lcd_handle->print("JOBS FINISHED");
      }
      goodbye(10);
    }
    else
    {
      while (hourglass.time() < given_time)
      {
        if (not system_is_okay)
        {
          for (int i = 0; i < LENGTH(cells); i++)
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
Amp_t BMS::getCalibrationOfIin() const
{
  Vol_t Vref_sensorV = refOf.zenerdiodeVfromRtoA, Vref = refOf.arduinoRegularV, Iin_sensorV = 0.5 * Vref;

  Vref_sensorV = refOf.arduinoRegularV * arduino5V_pin.readSignal(10) / refOf.analogSignalMax;
  Vref = refOf.arduinoRegularV * refOf.zenerdiodeVfromRtoA / Vref_sensorV;
  Iin_sensorV = Vref * Iin_pin.readSignal(10) / refOf.analogSignalMax;

  return ((Iin_sensorV - 0.5 * Vref) / refOf.sensitivityOfCurrentSensor);
}
void BMS::measureValues()
{
  constexpr ms_t measuring_time = 10;
  Vol_t sensorV = 0.00, accumV = 0.00;
  // Calculate the voltage of the pin `5V`
  sensorV = refOf.arduinoRegularV * arduino5V_pin.readSignal(measuring_time) / refOf.analogSignalMax;
  arduino5V = refOf.arduinoRegularV * refOf.zenerdiodeVfromRtoA / sensorV;
  // Calculate the voltages of every cell
  for (int i = 0; i < LENGTH(cells); i++)
  {
    constexpr Ohm_t R1 = 18000.0, R2 = 2000.0;
    sensorV = arduino5V * cells[i].voltage_sensor_pin.readSignal(measuring_time) / refOf.analogSignalMax;
    cellVs[i] = (sensorV / (R2 / (R1 + R2))) - accumV;
    accumV += cellVs[i];
  }
  // Calculate the main current
  sensorV = arduino5V * Iin_pin.readSignal(measuring_time) / refOf.analogSignalMax;
  Iin = ((sensorV - 0.5 * arduino5V) / refOf.sensitivityOfCurrentSensor) - Iin_calibration;
  // Guarantee that the above values are fresh
  measuredValuesAreFresh = true;
}
void BMS::findQs_0()
{
  if (not measuredValuesAreFresh)
  {
    measureValues();
  }
  for (int i = 0; i < LENGTH(Qs); i++)
  {
    Qs[i] = refOf.batteryCapacity * (mySocOcvTable.get_x_from_y(cellVs[i]) / 100);
  }
  Qs_lastUpdatedTime.reset();
}
void BMS::updateQs()
{
  if (not measuredValuesAreFresh)
  {
    measureValues();
  }
  for (int i = 0; i < LENGTH(cells); i++)
  {
    Qs[i] += Iin * (Qs_lastUpdatedTime.getDuration() / 3600);
  }
  Qs_lastUpdatedTime.reset();
}
double BMS::getSocOf(int const cell_no) const
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
  for (int i = 0; i < LENGTH(cellVs); i++)
  {
    slog << "cellVs[" << i << "] = " << cellVs[i] << "[V].";
  }
  if (lcd_handle)
  {
    LcdPrinter lcd = { .lcdHandleRef = lcd_handle };

    for (int i = 0; i < LENGTH(cellVs); i++)
    {
      double const soc = getSocOf(i);

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
#if( 0 )
  updateQs();
#endif
  cells[cell_no].BalanceCircuit_pin.turnOff();
}
void BMS::breakCharging(int const cell_no)
{
#if( 0 )
  updateQs();
#endif
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
  for (int i = 0; i < LENGTH(cellVs); i++)
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
  for (int i = 0; i < LENGTH(cellVs); i++)
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
  Timer hourglass = { };

  for (int i = 0; i < LENGTH(cells); i++)
  {
    cells[i].BalanceCircuit_pin.turnOn();
  }
  if (lcd_handle)
  {
    lcd_handle->setCursor(1, 0);
    lcd_handle->print(" SECS LEFT");
  }
  for (int i = countDown; i > 0; i--)
  {
    if (lcd_handle)
    {
      lcd_handle->setCursor(0, 0);
      lcd_handle->print(i - 1);
    }
    serr << "Your arduino will abort in " << i << " seconds.";
    hourglass.delay(1000);
    hourglass.reset();
  }
  if (lcd_handle)
  {
    lcd_handle->clear();
    lcd_handle->noBacklight();
    delete lcd_handle;
    lcd_handle = nullptr;
  }
  powerIn_pin.turnOff();
  abort();
}
