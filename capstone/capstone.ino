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
Vol_t V_wanted = 4.00, overV_wanted = 4.00; // FIX ME!

static constexpr
ReferenceCollection const refOf =
{ .analogSignalMax              = 1024
, .arduinoRegularV              = 5.00
, .batteryCapacity              = 3317
, .sensitivityOfCurrentSensor   = .100 // is the sensitivity of the current sensor `ACS712ELCTR-20A-T`.
, .zenerdiodeVfromRtoA          = 2.48 // is `Vref` of the zener-diode `TL431BVLPRAGOSCT-ND`.
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
  bool every_cell_attatched     = false;
  bool is_operating_now         = false;
  bool power_connected          = false;
public:
  void initialize(ms_t time_limit);
  void progress(ms_t time_limit);
  void checkCellsAttatched();
  void getCalibrationOfIin();
  void measureValues();
  void findQs_0();
  void updateQs();
  double getSocOf(int cell_no) const;
  void printValues() const;
  void startCharging(int cell_no);
  void breakCharging(int cell_no);
  bool checkSafety(bool reports_to_serial);
  void controlSystem();
  void showBmsInfo();
  void goodbye(char const *bye_message, int seconds_left_to_quit = 10);
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
}
void BMS::progress(ms_t const given_time)
{
  Timer hourglass = { };

  checkCellsAttatched();
  switch (every_cell_attatched)
  {
  case true:
    if (is_operating_now)
    {
      drawlineSerial();
      measureValues();
      printValues();
      showBmsInfo();
      {
        bool system_is_okay = checkSafety(true);

        controlSystem();
        if (system_is_okay and jobsDone)
        {
          sout << "CHARGING COMPLETED.";
          goodbye("JOBS FINISHED");
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
      hourglass.delay(given_time);
      if (not power_connected)
      {
        is_operating_now = false;
      }
      break;
    }
    else
    {
      bool rebooted = false;

      do
      {
        switch (rebooted)
        {
        case true:
          checkCellsAttatched();
          if (every_cell_attatched)
          {
        case false:
            if (rebooted)
            {
              measureValues();
              printValues();
              showBmsInfo();
            }
            else
            {
              if (lcd_handle)
              {
                LcdPrinter lcd = { .lcdHandleRef = lcd_handle };
                lcd.println("CELLS AR");
                lcd.println("E");
                lcd.println("RECOGNIZ");
                lcd.println("ED.");
              }
            }
            powerIn_pin.turnOff();
            delay(2000);
            getCalibrationOfIin();
            sout << "Iin_calibration = " << Iin_calibration << "[A].";
            if (lcd_handle)
            {
              LcdPrinter lcd = { .lcdHandleRef = lcd_handle };
              if (rebooted)
              {
                lcd.println("REBOOTIN");
                lcd.println("G BMS...");
              }
              else
              {
                lcd.println("WAIT FOR");
                lcd.println(" CIRCUIT");
                lcd.println("BEING ST");
                lcd.println("ABLIZED.");
              }
            }
            delay(3000);
            powerIn_pin.turnOn();
            delay(2000);
            measureValues();
            rebooted = true;
          }
        }
      } while (not power_connected);
      findQs_0();
      for (int cell_no = 0; cell_no < LENGTH(cells); cell_no++)
      {
        startCharging(cell_no);
      }
      slog << "OPERATING NOW!";
      is_operating_now = true;
      showBmsInfo();
      hourglass.delay(given_time);
    }
    break;
  case false:
    if (not is_operating_now)
    {
      sout << "Arduino sleep!";
    }
    is_operating_now = false;
    for (int cell_no = 0; cell_no < LENGTH(cells); cell_no++)
    {
      breakCharging(cell_no);
    }
    powerIn_pin.turnOff();
    if (lcd_handle)
    {
      LcdPrinter lcd = { .lcdHandleRef = lcd_handle };
      lcd.println("> SYSTEM");
      lcd.println(" ONLINE");
      lcd.println("VERSION");
      lcd.print("= ");
      lcd.println(VERSION);
    }
    hourglass.delay(given_time);
    Qs_lastUpdatedTime.reset();
    break;
  }
}
void BMS::checkCellsAttatched()
{
  Vol_t sensorV = 0.0, accumV = 0.00;
  for (int i = 0; i < LENGTH(cells); i++)
  {
    constexpr Ohm_t R1 = 18000.0, R2 = 2000.0;
    sensorV = refOf.arduinoRegularV * cells[i].voltage_sensor_pin.readSignal(20) / refOf.analogSignalMax;
    cellVs[i] = (sensorV / (R2 / (R1 + R2))) - accumV;
    accumV += cellVs[i];
  }
  every_cell_attatched = true;
  for (int i = 0; i < LENGTH(cellVs); i++)
  {
    every_cell_attatched &= cellVs[i] >= allowedV_min;
  }
}
void BMS::getCalibrationOfIin()
{
  Vol_t Vref_sensorV = refOf.zenerdiodeVfromRtoA, Vref = refOf.arduinoRegularV, Iin_sensorV = 0.5 * Vref;

  Vref_sensorV = refOf.arduinoRegularV * arduino5V_pin.readSignal(20) / refOf.analogSignalMax;
  Vref = refOf.arduinoRegularV * refOf.zenerdiodeVfromRtoA / Vref_sensorV;
  Iin_sensorV = Vref * Iin_pin.readSignal(20) / refOf.analogSignalMax;
  Iin_calibration = (Iin_sensorV - 0.5 * Vref) / refOf.sensitivityOfCurrentSensor;
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
  power_connected = Iin >= 0.05;
  // Guarantee that the above values are fresh
  measuredValuesAreFresh = true;
}
void BMS::findQs_0()
{
  for (int i = 0; i < LENGTH(Qs); i++)
  {
    Qs[i] = refOf.batteryCapacity * mySocOcvTable.get_x_by_y(cellVs[i]) / 100.0;
  }
  Qs_lastUpdatedTime.reset();
}
void BMS::updateQs()
{
  for (int i = 0; i < LENGTH(cells); i++)
  {
    bool const balance_circuit_on = cells[i].BalanceCircuit_pin.isHigh();
    if (not balance_circuit_on)
    {
      Qs[i] += Iin * Qs_lastUpdatedTime.getDuration() / 3600.0;
    }
  }
  Qs_lastUpdatedTime.reset();
}
double BMS::getSocOf(int const cell_no) const
{
  return Qs[cell_no] / refOf.batteryCapacity * 100.0;
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
  for (int cell_no = 0; cell_no < LENGTH(cellVs); cell_no++)
  {
    bool const is_this_cell_being_charged_now = not cells[cell_no].BalanceCircuit_pin.isHigh();
    bool const is_this_cell_fully_charged_now = cellVs[cell_no] >= (is_this_cell_being_charged_now ? overV_wanted : V_wanted);

    jobsDone &= is_this_cell_fully_charged_now;
    if ((not is_this_cell_fully_charged_now) and (not is_this_cell_being_charged_now))
    {
      startCharging(cell_no);
    }
    if ((is_this_cell_fully_charged_now) and (is_this_cell_being_charged_now))
    {
      breakCharging(cell_no);
    }
  }
  measuredValuesAreFresh = false;
}
void BMS::showBmsInfo()
{
  sout << "powerIn_pin.is_high = " << powerIn_pin.isHigh() << ".";
  for (int i = 0; i < LENGTH(cells); i++)
  {
    sout << "cells[" << i << "].BalanceCircuit_pin.is_high = " << cells[i].BalanceCircuit_pin.isHigh() << ".";
  }
  slog << "Iin_calibration = " << Iin_calibration << "[A].";
  for (int i = 0; i < LENGTH(Qs); i++)
  {
    slog << "Qs[" << i << "] = " << static_cast<double>(Qs[i]) << "[mAh].";
  }
}
void BMS::goodbye(char const *const msg, int const countDown)
{
  Timer hourglass = { };

  for (int i = 0; i < LENGTH(cells); i++)
  {
    cells[i].BalanceCircuit_pin.turnOn();
  }
  if (lcd_handle)
  {
    lcd_handle->clear();
    lcd_handle->setCursor(0, 1);
    lcd_handle->print(msg);
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
