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
  jobs_finished           = 1,
  every_cells_connected   = 2,
  power_connected         = 3,
  cells_locked            = 4,
  power_locked            = 5,
  bms_being_operating     = 6,
  not_dormant             = 7,
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
  BitArray<byte> bms_state      = 0u;
  int8_t dormant_cnt            = 0;
public:
  void setup();
  void loop();
  bool routine();
  void reboot();
  bool checkCellsAttatched();
  bool checkPowerConnected();
  Amp_t getCalibrationOfIin();
  void measureValues();
  void findQs_0();
  void updateQs();
  double getSocOf(int cell_no) const;
  void printValues() const;
  void lockCells();
  void lockPower();
  void unlockCells();
  void unlockPower();
  void greeting();
  void goodbye(char const *bye_message, int seconds_left_to_quit = 10);
  void report() const;
  void revive();
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
  sout << "Runtime begin.";
  bms_state = 0u;
  bms_state.set(bms_life, true);
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
    this->greeting();
  }
  else
  {
    serr << "LCD not connected.";
  }
  bms_state.set(not_dormant, true);
  hourglass.delay(3000);
  dormant_cnt = 0;
}
void BMS::loop()
{
  Timer hourglass = { };
  bool okay = bms_state.get(bms_life) && bms_state.get(not_dormant);
  this->report();
  if (not bms_state.get(bms_being_operating))
  {
    this->updateQs();
    if (dormant_cnt < 10)
    {
      dormant_cnt++;
    }
    else
    {
      dormant_cnt = 0;
      this->reboot();
    }
  }
  switch (okay)
  {
  default:
    if (bms_state.get(jobs_finished))
    {
      sout << "CHARGING COMPLETED.";
      this->goodbye("JOBS FINISHED");
      break;
    }
    okay &= this->checkCellsAttatched();
    bms_state.set(not_dormant, okay);
    if (okay)
    {
      if (bms_state.get(bms_being_operating))
      {
        sout << "Running.";
        this->routine();
        break;
      }
      if (bms_state.get(power_locked))
      {
        sout << "Booting.";
        if (lcd_handle)
        {
          LcdPrinter lcd = { .lcdHandleRef = lcd_handle };
          lcd.println("BOOTING ");
          lcd.println("BMS...");
        }
        this->unlockCells();
        delay(2000);
        this->getCalibrationOfIin();
        this->checkPowerConnected();
        this->measureValues();
        this->findQs_0();
        this->unlockPower();
        break;
      }
      this->lockCells();
      this->checkPowerConnected();
      if (bms_state.get(power_connected))
      {
        delay(2000);
        this->measureValues();
        this->findQs_0();
        this->unlockCells();
        for (int i = 0; i < LENGTH(cellVs); i++)
        {
          double const soc = getSocOf(i);
          sout << "cellVs[" << i << "] = " << cellVs[i] << "[V].";
          sout << "soc[" << i << "] = " << soc << "%.";
        }
        this->unlockCells();
        bms_state.set(bms_being_operating, true);
        sout << "NOW BMS OPERATING.";
        break;
      }
      this->unlockPower();
      sout << "Waiting.";
      if (lcd_handle)
      {
        LcdPrinter lcd = { .lcdHandleRef = lcd_handle };
        for (int i = 0; i < LENGTH(cellVs); i++)
        {
          double const soc = getSocOf(i);
          sout << "cellVs[" << i << "] = " << cellVs[i] << "[V].";
          sout << "soc[" << i << "] = " << soc << "%.";
          lcd.print("B");
          lcd.print(i + 1);
          lcd.print("=");
          lcd.println(cellVs[i]);
          lcd.print(" ");
          lcd.print(soc);
          lcd.println("%");
        }
        lcd.println("NO POWER");
        lcd.println(" SUPPLY ");
      }
      break;
  case false:
      sout << "Runtime begin.";
      this->revive();
    }
    else
    {
      sout << "Restarting.";
    }
    bms_state.set(bms_being_operating, false);
    this->lockCells();
    this->lockPower();
    Iin_calibration = 0.0;
    this->greeting();
    bms_state.set(not_dormant, true);
  }
  hourglass.delay(3000);
}
bool BMS::routine()
{
  bool pin_state_modified = false, every_cells_fully_charged = true;
  this->measureValues();
  this->updateQs();
  this->printValues();
  if (Iin < Iin_calibration + allowedA_min)
  {
    serr << "`Iin`" << " too LOW.";
    bms_state.set(not_dormant, false);
  }
  if (Iin > allowedA_max)
  {
    serr << "`Iin`" << " too HIGH.";
    pin_state_modified |= powerIn_pin.isHigh();
    this->lockPower();
  }
  for (int cell_no = 0; cell_no < LENGTH(cellVs); cell_no++)
  {
    if (cellVs[cell_no] > allowedV_max)
    {
      serr << "`cellVs[" << cell_no << "]`" << " too HIGH.";
      pin_state_modified |= not cells[cell_no].BalanceCircuit_pin.isHigh();
      cells[cell_no].BalanceCircuit_pin.turnOn();
    }
  }
  if (pin_state_modified)
  {
    delay(3000);
    this->measureValues();
    for (int cell_no = 0; cell_no < LENGTH(cellVs); cell_no++)
    {
      bool const is_this_cell_being_charged_now = not cells[cell_no].BalanceCircuit_pin.isHigh();
      bool const is_this_cell_fully_charged_now = cellVs[cell_no] >= (is_this_cell_being_charged_now ? wantedV_overloaded : wantedV);
      every_cells_fully_charged &= is_this_cell_fully_charged_now;
    }
  }
  else
  {
    for (int cell_no = 0; cell_no < LENGTH(cellVs); cell_no++)
    {
      bool const is_this_cell_being_charged_now = not cells[cell_no].BalanceCircuit_pin.isHigh();
      bool const is_this_cell_fully_charged_now = cellVs[cell_no] >= (is_this_cell_being_charged_now ? wantedV_overloaded : wantedV);
      every_cells_fully_charged &= is_this_cell_fully_charged_now;      
      if ((not is_this_cell_fully_charged_now) and (not is_this_cell_being_charged_now))
      {
        cells[cell_no].BalanceCircuit_pin.turnOff();
      }
      if ((is_this_cell_fully_charged_now) and (is_this_cell_being_charged_now))
      {
        cells[cell_no].BalanceCircuit_pin.turnOn();
      }
    }
  }
  bms_state.set(jobs_finished, every_cells_fully_charged);
  return bms_state.get(not_dormant);
}
void BMS::reboot()
{
  this->lockPower();
  bms_state.set(not_dormant, true);
}
bool BMS::checkPowerConnected()
{
  Vol_t Vref_sensorV = refOf.zenerdiodeVfromRtoA, Vref = refOf.arduinoRegularV, Iin_sensorV = 0.5 * Vref;
  Vref_sensorV = refOf.arduinoRegularV * arduino5V_pin.readSignal(20) / refOf.analogSignalMax;
  Vref = refOf.arduinoRegularV * refOf.zenerdiodeVfromRtoA / Vref_sensorV;
  Iin_sensorV = Vref * Iin_pin.readSignal(20) / refOf.analogSignalMax;
  bms_state.set(power_connected, ((Iin_sensorV - 0.5 * Vref) / refOf.sensitivityOfCurrentSensor) >= allowedA_min);
  return bms_state.get(power_connected);
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
Amp_t BMS::getCalibrationOfIin()
{
  Vol_t Vref_sensorV = refOf.zenerdiodeVfromRtoA, Vref = refOf.arduinoRegularV, Iin_sensorV = 0.5 * Vref;
  if (bms_state.get(power_locked))
  {
    Iin_calibration = 0.0;
  }
  else
  {
    Vref_sensorV = refOf.arduinoRegularV * arduino5V_pin.readSignal(20) / refOf.analogSignalMax;
    Vref = refOf.arduinoRegularV * refOf.zenerdiodeVfromRtoA / Vref_sensorV;
    Iin_sensorV = Vref * Iin_pin.readSignal(20) / refOf.analogSignalMax;
    Iin_calibration = (Iin_sensorV - 0.5 * Vref) / refOf.sensitivityOfCurrentSensor;
    sout << "Iin_calibration = " << Iin_calibration << "[A].";
  }
  return Iin_calibration;
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
void BMS::findQs_0()
{
  for (int cell_no = 0; cell_no < LENGTH(Qs); cell_no++)
  {
    Qs[cell_no] = refOf.batteryCapacity * mySocOcvTable.get_x_by_y(cellVs[cell_no]) / 100.0;
  }
  Qs_lastUpdatedTime.reset();
}
void BMS::updateQs()
{
  for (int cell_no = 0; cell_no < LENGTH(cells); cell_no++)
  {
    if (not cells[cell_no].BalanceCircuit_pin.isHigh())
    {
      Qs[cell_no] += Iin * Qs_lastUpdatedTime.getDuration() / 3600.0;
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
void BMS::lockCells()
{
  if (not bms_state.get(cells_locked))
  {
    for (int i = 0; i < LENGTH(cells); i++)
    {
      cells[i].BalanceCircuit_pin.turnOn();
    }
    bms_state.set(cells_locked, true);
  }
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
  if (not bms_state.get(power_locked))
  {
    powerIn_pin.turnOff();
    bms_state.set(power_locked, true);
  }
}
void BMS::unlockPower()
{
  powerIn_pin.turnOn();
  bms_state.set(power_locked, false);
}
void BMS::greeting()
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
void BMS::goodbye(char const *const msg, int const countDown)
{
  Timer hourglass = { };
  this->lockCells();
  bms_state.set(bms_being_operating, false);
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
  Wire.end();
  Serial.end();
  this->lockPower();
  bms_state.set(bms_life, false);
  abort();
}
void BMS::report() const
{
  drawlineSerial();
  slog << "`powerIn_pin.is_high` = " << powerIn_pin.isHigh() << ".";
  for (int i = 0; i < LENGTH(cells); i++)
  {
    slog << "`cells[" << i << "].BalanceCircuit_pin.is_high` = " << cells[i].BalanceCircuit_pin.isHigh() << ".";
  }
  slog << "`Iin_calibration` = " << Iin_calibration << "[A].";
  slog << "`Iin` = " << Iin << "[A].";
  for (int i = 0; i < LENGTH(Qs); i++)
  {
    slog << "`Qs[" << i << "]` = " << static_cast<double>(Qs[i]) << "[mAh].";
  }
}
void BMS::revive()
{
  bms_state = 0u;
  bms_state.set(bms_life, true);
  if (lcd_handle == nullptr)
  {
    lcd_handle = openLcdI2C(LCD_WIDTH, LCD_HEIGHT);
  }
  if (lcd_handle == nullptr)
  {
    serr << "LCD not connected.";
  }
}
