/* <CAPSTONE PROJECT>
** ===============================================================================
** MEMBER        | AFFILIATION                                                   |
** ===============================================================================
** Hwan-hee Jeon | School of Mechanical Engineering, Chonnam National University |
** Hak-jung Im   | School of Mechanical Engineering, Chonnam National University |
** Ki-jeong Lim  | School of Mechanical Engineering, Chonnam National University |
** ===============================================================================
*/

#include "capstone.hpp"

enum bms_state_mask_t : int {
  bms_life                = 0,
  jobs_done               = 1,
  every_cells_connected   = 2,
  Iin_recongnized         = 3,
  cells_locked            = 4,
  power_locked            = 5,
  bms_being_operating     = 6,
};

static constexpr
Vol_t wantedV = 4.00, wantedV_overloaded = 4.00;

static constexpr
ReferenceCollection const refOf =
{ .analogSignalMax              = 1024
, .arduinoRegularV              = 5.00
, .batteryCapacity              = 3317
, .sensitivityOfCurrentSensor   = .100
, .zenerdiodeVfromRtoA          = 2.48
};

static constexpr
Vol_t allowedV_max = 4.20, allowedV_min = 2.70;

static constexpr
Amp_t allowedA_max = 2.00, allowedA_min = 0.10;

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
  Timer Qs_lastUpdatedTime      = { .init_time = 0 };
  LcdHandle_t lcd_handle        = nullptr;
  Vol_t arduino5V               = refOf.arduinoRegularV;
  Amp_t Iin                     = 0.00;
  Amp_t Iin_calibration         = 0.00;
  Vol_t cellVs[LENGTH(cells)]   = { };
  mAh_t Qs[LENGTH(cells)]       = { };
  Bits<uint16_t> bms_state      = 0u;
public:
  void setup();
  void loop();
  void routine();
  void printVersion();
  bool checkCellsAttatched();
  void goodbye(char const *bye_message, int seconds_left_to_quit = 10);
  bool checkPowerConnected();
  Amp_t getCalibrationOfIin();
  void lockCells();
  void lockPower();
  void unlockCells();
  void unlockPower();
  void measureValues();
  void findQs_0();
  void updateQs();
  double getSocOf(int cell_no) const;
  void printValues() const;
  void log() const;
  void dormant();
  void recover();
} myBMS;

void setup()
{
  myBMS.setup();
}

void loop()
{
  myBMS.loop();
}

void BMS::setup()
{
  Timer hourglass = { };
  invokingSerial();
  sout << "Run time started.";
  Wire.begin();
  for (int i = 0; i < LENGTH(cells); i++)
  {
    cells[i].BalanceCircuit_pin.initWith(true);
    Qs[i] = 0.0;
  }
  bms_state.set(cells_locked, true);
  powerIn_pin.initWith(false);  
  bms_state.set(power_locked, true);
  lcd_handle = openLcdI2C(LCD_WIDTH, LCD_HEIGHT);
  if (lcd_handle)
  {
    this->printVersion();
  }
  else
  {
    serr << "LCD not connected.";
  }
  bms_state.set(bms_life, true);
  hourglass.delay(3000);
}
void BMS::loop()
{
  Timer hourglass = { };
  this->log();
  switch (bms_state.get(bms_life))
  {
  default:
    this->checkCellsAttatched();
    if (bms_state.get(every_cells_connected))
    {
      if (bms_state.get(jobs_done))
      {
        sout << "CHARGING COMPLETED.";
        this->goodbye("JOBS FINISHED");
        break;
      }
      if (bms_state.get(bms_being_operating))
      {
        sout << "Perform charging.";
        this->routine();
      }
      if (bms_state.get(power_locked))
      {
        sout << "Booting BMS.";
        if (lcd_handle)
        {
          LcdPrinter lcd = { .lcdHandleRef = lcd_handle };
          lcd.println("BOOTING ");
          lcd.println("BMS...");
        }
        delay(2000);
        this->getCalibrationOfIin();
        this->unlockPower();
        break;
      }
      this->checkPowerConnected();
      if (bms_state.get(Iin_recongnized))
      {
        this->lockCells();
        delay(2000);
        this->measureValues();
        this->findQs_0();
        this->unlockCells();
        bms_state.set(bms_being_operating, true);
        sout << "NOW BMS OPERATING.";
        break;
      }
      sout << "Waiting for the power supply to turn on."; 
      this->lockCells();
      if (lcd_handle)
      {
        LcdPrinter lcd = { .lcdHandleRef = lcd_handle };
        lcd.println("PLZ CONN");
        lcd.println("ECT YOUR");
        lcd.println("POWER SU");
        lcd.println("PPLY ...");
      }
      break;
  case false:
      this->recover();
    }
    else
    {
      sout << "Waiting for every cell being connected."; 
    }
    this->dormant();
    break;
  }
  hourglass.delay(3000);
}
void BMS::routine()
{
  bool isGood = true, jobsDone = true;
  this->measureValues();
  this->updateQs();
  this->printValues();
  // Check current
  if (Iin > allowedA_max)
  {
    isGood = false;
    serr << "`Iin`" << " too HIGH.";
  }
  // Check voltages
  for (int i = 0; i < LENGTH(cellVs); i++)
  {
    if (cellVs[i] > allowedV_max)
    {
      isGood = false;
      serr << "`cellVs[" << i << "]`" << " too HIGH.";
    }
  }
  if (isGood)
  {
    for (int cell_no = 0; cell_no < LENGTH(cellVs); cell_no++)
    {
      bool const is_this_cell_being_charged_now = not cells[cell_no].BalanceCircuit_pin.isHigh();
      bool const is_this_cell_fully_charged_now = cellVs[cell_no] >= (is_this_cell_being_charged_now ? wantedV_overloaded : wantedV);
      jobsDone &= is_this_cell_fully_charged_now;
      if ((not is_this_cell_fully_charged_now) and (not is_this_cell_being_charged_now))
      {
        cells[cell_no].BalanceCircuit_pin.turnOff();
      }
      if ((is_this_cell_fully_charged_now) and (is_this_cell_being_charged_now))
      {
        cells[cell_no].BalanceCircuit_pin.turnOn();
      }
    }
    bms_state.set(jobs_done, jobsDone);
  }
  else
  {
    this->lockCells();
    this->lockPower();
    if (lcd_handle)
    {
      lcd_handle->clear();
      lcd_handle->setCursor(0, 0);
      lcd_handle->print("EMERGENCY");
    }
    bms_state.set(bms_life, false);
  }
}
void BMS::printVersion()
{
  if (lcd_handle)
  {
    LcdPrinter lcd = { .lcdHandleRef = lcd_handle };
    lcd.println("> SYSTEM");
    lcd.println(" ONLINE");
    lcd.println("VERSION");
    lcd.print("= ");
    lcd.println(VERSION);
  }
}
bool BMS::checkCellsAttatched()
{
  bool every_cell_being_attatched = true;
  Vol_t sensorV = 0.0, accumV = 0.00;
  for (int i = 0; i < LENGTH(cells); i++)
  {
    constexpr Ohm_t R1 = 18000.0, R2 = 2000.0;
    sensorV = refOf.arduinoRegularV * cells[i].voltage_sensor_pin.readSignal(20) / refOf.analogSignalMax;
    cellVs[i] = (sensorV / (R2 / (R1 + R2))) - accumV;
    accumV += cellVs[i];
  }
  for (int i = 0; i < LENGTH(cellVs); i++)
  {
    every_cell_being_attatched &= cellVs[i] >= allowedV_min;
  }
  bms_state.set(every_cells_connected, every_cell_being_attatched);
  return every_cell_being_attatched;
}
void BMS::goodbye(char const *const msg, int const countDown)
{
  Timer hourglass = { };
  this->lockCells();
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
  this->lockPower();
  bms_state.set(bms_being_operating, false);
  bms_state.set(bms_life, false);
  abort();
}
void BMS::lockCells()
{
  for (int i = 0; i < LENGTH(cells); i++)
  {
    cells[i].BalanceCircuit_pin.turnOn();
  }
  bms_state.set(cells_locked, true);
}
void BMS::unlockCells()
{
  for (int i = 0; i < LENGTH(cells); i++)
  {
    cells[i].BalanceCircuit_pin.turnOff();
  }
  bms_state.set(cells_locked, false);
}
void BMS::lockPower()
{
  powerIn_pin.turnOff();
  bms_state.set(power_locked, true);
}
void BMS::unlockPower()
{
  powerIn_pin.turnOn();
  bms_state.set(power_locked, false);
}
bool BMS::checkPowerConnected()
{
  Vol_t Vref_sensorV = refOf.zenerdiodeVfromRtoA, Vref = refOf.arduinoRegularV, Iin_sensorV = 0.5 * Vref;
  Vref_sensorV = refOf.arduinoRegularV * arduino5V_pin.readSignal(20) / refOf.analogSignalMax;
  Vref = refOf.arduinoRegularV * refOf.zenerdiodeVfromRtoA / Vref_sensorV;
  Iin_sensorV = Vref * Iin_pin.readSignal(20) / refOf.analogSignalMax;
  bms_state.set(Iin_recongnized, ((Iin_sensorV - 0.5 * Vref) / refOf.sensitivityOfCurrentSensor) >= allowedA_min);
  return bms_state.get(Iin_recongnized);
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
}
Amp_t BMS::getCalibrationOfIin()
{
  Vol_t Vref_sensorV = refOf.zenerdiodeVfromRtoA, Vref = refOf.arduinoRegularV, Iin_sensorV = 0.5 * Vref;
  Vref_sensorV = refOf.arduinoRegularV * arduino5V_pin.readSignal(20) / refOf.analogSignalMax;
  Vref = refOf.arduinoRegularV * refOf.zenerdiodeVfromRtoA / Vref_sensorV;
  Iin_sensorV = Vref * Iin_pin.readSignal(20) / refOf.analogSignalMax;
  Iin_calibration = (Iin_sensorV - 0.5 * Vref) / refOf.sensitivityOfCurrentSensor;
  return Iin_calibration;
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
  return 100.0 * Qs[cell_no] / refOf.batteryCapacity;
}
void BMS::printValues() const
{
  sout << "arduino5V = " << arduino5V << "[V].";
  sout << "Iin = " << Iin << "[A].";
  for (int i = 0; i < LENGTH(cellVs); i++)
  {
    sout << "cellVs[" << i << "] = " << cellVs[i] << "[V].";
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
void BMS::log() const
{
  drawlineSerial();
  slog << "`powerIn_pin.is_high` = " << powerIn_pin.isHigh() << ".";
  for (int i = 0; i < LENGTH(cells); i++)
  {
    slog << "`cells[" << i << "].BalanceCircuit_pin.is_high` = " << cells[i].BalanceCircuit_pin.isHigh() << ".";
  }
  slog << "`Iin_calibration` = " << Iin_calibration << "[A].";
  for (int i = 0; i < LENGTH(Qs); i++)
  {
    slog << "`Qs[" << i << "]` = " << static_cast<double>(Qs[i]) << "[mAh].";
  }
}
void BMS::dormant()
{
  this->printVersion();
}
void BMS::recover()
{
}
