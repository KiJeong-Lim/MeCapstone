/** THE THIRD STEP ON OUR CAPSTONE PROJECT
  <Abstract>
  * A battery-cell charging system using the CC-CV method 
  <Members>
  * Hwanhee Jeon [School of Mechanical Engineering, Chonnam National University]
  * Hakjung Im [School of Mechanical Engineering, Chonnam National University]
  * Kijeong Lim [School of Mechanical Engineering, Chonnam National University]
  <Wiring Diagram>
  *                     #===========#
  *                     # LCD (I2C) #
  *                     # ========= #
  * +-------------------< SCL       #
  * |   +---------------< SDA       #
  * |   |       +-------< VCC       #
  * |   |       |   +---< GND       #
  * |   |       |   |   #===========#
  * |   |       |   |
  * |   |       |   |   #==============#
  * |   |       |   |   # Arduino      #
  * |   |       |   |   # ============ #
  * |   |       |   |   * IOREF # AREF *                    #===============#
  * |   |       |   |   * RESET #  GND >----------------+---< (-) POWER (+) >---+
  * |   |       |   |   * 3.3V  #   13 *                |   #===============#   |
  * |   |       +---|---< 5V    #   12 *                |                       |
  * |   |           +---< GND   #  ~11 *                |       #========#      |
  * |   |   +-----------< GND   #  ~10 *                |   +---< R (#1) >------+
  * |   |   |           * Vin   #   ~9 *                |   |   #========#      |
  * |   |   |           #       #    8 *                |   |                   |
  * |   |   |           #       #    7 *                |   |   #=====#         |
  * |   |   |           #       #   ~6 *                |   |   # NPN #         |
  * |   |   |   +-------< A0    #   ~5 *                |   |   # === #         |
  * |   |   |   |   +---< A1    #    4 *   #========#   |   +---< C   #         |
  * |   |   |   |   |   * A2    #   ~3 >---< R (#2) >---|---|---< B   #         |
  * |   |   |   |   |   * A3    #    2 *   #========#   +---|---< E   #         |   #========#
  * |   +---|---|---|---< A4    #    1 *                |   |   #=====#         |   # MOFSET #
  * +-------|---|---|---< A5    #    0 *                |   |                   |   # ====== #
  *         |   |   |   #==============#                |   +-------------------|---< G      #
  *         |   |   |                                   |                       +---< D      #
  *         |   |   |                                   |                   +-------< S      #
  *         |   |   |                                   |                   |       #========#
  *         |   |   |   #============#                  |                   |
  *         |   |   |   # Sensor     #                  |                   |
  *         |   |   |   # ========== #                  |                   |
  *         |   +---|---< VT  #  Vin >------------------|-------------------|------------------+
  *         |       +---< AT  #  GND >------------------+                   |                  |
  *         |           * GND # Vout >-------+          |                   |                  |
  *         +-----------< GND #  GND >---+   |          |   #===========#   |   #==========#   |
  *                     #============#   |   |          +---< + Diode - >---+---< Inductor >---+
  *                                      |   |          |   #===========#       #==========#   |
  *           #======#   #===========#   |   |          |                                      |
  *       +---< Fuse >---< - Diode + >---+   |          |                      #===========#   |
  *       |   #======#   #===========#       |          +----------------------< Capacitor >---+
  *       |                                  |                                 #===========#
  *       |   #==============#               |
  *       +---< (-) Cell (+) >---------------+
  *           #==============#
  <Used Parts>
  * [Arduino] Arduino Uno (R3)
  * [Capacitor] 2200[μF] 25[V]
  * [Cell] NCR18650B Li-ion MH12210
  * [Diode] 1N4001
  * [Fuse] Polyswitch 3[A]/30[V]
  * [Inductor] 100[mH]
  * [LCD (I2C)] Arduino LCD 16x2 display module with 4-pins I2C LCD controller
  * [MOSFET] IRFP260N
  * [NPN] C1815
  * [POWER] ???
  * [R (#1)] 10[kΩ]
  * [R (#2)] 10[kΩ]
  * [Sensor] MAX471 25[V] 3[A] (HAM6703)
  <Dependencies>
  * [LiquidCrystal_I2C] https://codeload.github.com/johnrickman/LiquidCrystal_I2C/zip/refs/tags/1.1.3
  * [ACS712] https://codeload.github.com/rkoptev/ACS712-arduino/zip/refs/tags/1.0.2
  <References>
  * [1] https://m.blog.naver.com/ysahn2k/222074476103
  * [2] https://codingrun.com/119
***/

// Includes

#include "third.h"

// Module Variables

struct MeasuredValueStorage { V_t voltage; mA_t current; }
  measured =
  { .voltage = 0.0 // unit: [V], constraints: OCV_AT_SOC_0 =< voltage =< OCV_AT_SOC_100
  , .current = 0.0 // unit: [mA], constraints: 0.0 =< current =< BATTERY_CAPACITY * CHARGING_COEFFICIENT
  }
;

struct BatteryChargingSystem { LiquidCrystal_I2C *lcd_handle; int PWM_val; }
  charger =
  { .lcd_handle = nullptr
  , .PWM_val    = 0 // 0 ~ 255
  }
;

struct HAM6703_t { V_t MAX_INPUT_VOLTAGE; Ohm_t voltage_ampere_ratio; } const
  HAM6703 =
  { .MAX_INPUT_VOLTAGE    = 25.0  // [V]
  , .voltage_ampere_ratio = 1.0   // [V/A]
  }
;

struct TIMER_t { Time_t initial_time; Time_t current_time; }
  my_timer =
  { .initial_time = 0
  , .current_time = 0
  }
;

// Entry points

void setup()
{
  #ifndef NO_DEBUGGING
  Serial.begin(9600);
  Serial.println("log: Runtime started.");
  #endif // ifndef NO_DEBUGGING

  Wire.begin();

  my_timer.initial_time = millis();

  greeting();
  analyzeCell();
}

void loop()
{
  readSensorFor100ms();

  if (measured.current < 65.0)
  {
    setState(FINISH_STATE);
  }

  if (measured.current >= CUTOFF_AMPERE)
  {
    setPWM(OCV_AT_SOC_0);

    #ifndef NO_DEBUGGING
    Serial.println("ERROR: current over the limit!");
    #endif // ifndef NO_DEBUGGING

    charger.lcd_handle->clear();
    charger.lcd_handle->setCursor(0, 0);
    charger.lcd_handle->print("CURRENT");
    charger.lcd_handle->setCursor(0, 1);
    charger.lcd_handle->print("OVERLOADED");
    delay(1000);

    setState(BAD_STATE);
  }

  if (measured.voltage >= CUTOFF_VOLTAGE)
  {
    setPWM(OCV_AT_SOC_0);

    #ifndef NO_DEBUGGING
    Serial.println("ERROR: voltage over the limit!");
    #endif // ifndef NO_DEBUGGING

    charger.lcd_handle->clear();
    charger.lcd_handle->setCursor(0, 0);
    charger.lcd_handle->print("VOLTAGE");
    charger.lcd_handle->setCursor(0, 1);
    charger.lcd_handle->print("OVERLOADED");
    delay(1000);

    setState(BAD_STATE);
  }

  showMeasuredValues();

  my_timer.current_time = millis();

  V_t wanted_voltage = findVoltage(BATTERY_CAPACITY * CHARGING_COEFFICIENT);

  if (wanted_voltage >= OCV_AT_SOC_100)
  {
    setState(CV_STATE);

    wanted_voltage = OCV_AT_SOC_100;
  }
  else
  {
    setState(CC_STATE);
  }

  setPWM(wanted_voltage);
  delay(400);
}

// Core

V_t findVoltage(mA_t wanted_current)
{
  return lookupOcvTable(my_timer.current_time - my_timer.initial_time);
}

void analyzeCell()
{
  mA_t const wanted_current = BATTERY_CAPACITY * CHARGING_COEFFICIENT;

  while (1)
  {
    readSensorFor100ms();
    showMeasuredValues();

    if (measured.current - wanted_current < 65.0)
    {
      break;
    }

    setPWM(measured.voltage - 0.1);
  }
}

// Setters

void setPWM(V_t const wanted_voltage)
{
  double duty_ratio = wanted_voltage / VOLTAGE_OUTPUT_AT_255;
  
  if (duty_ratio > 1.0)
  {
    #ifndef NO_DEBUGGING
    Serial.print("Warning: duty_ratio = ");
    Serial.print(duty_ratio);
    Serial.println(".");
    #endif

    duty_ratio = 1.0;
  }

  if (duty_ratio < 0.0)
  {
    #ifndef NO_DEBUGGING
    Serial.print("Warning: duty_ratio = ");
    Serial.print(duty_ratio);
    Serial.println(".");
    #endif

    duty_ratio = 0.0;
  }

  charger.PWM_val = round(duty_ratio * 255);

  #ifndef NO_DEBUGGING
  Serial.print("log: PWM_val = ");
  Serial.print(charger.PWM_val);
  Serial.println(".");
  #endif

  analogWrite(PwmPin, charger.PWM_val);
}

void setState(State_t const charging_state)
{
  #ifndef NO_DEBUGGING
  Serial.print("log: charging_state = ");
  switch (charging_state)
  {
  case BAD_STATE:
    Serial.println("BAD_STATE");
    break;
  case CC_STATE:
    Serial.println("CC_STATE");
    break;
  case CV_STATE:
    Serial.println("CV_STATE");
  case FINISH_STATE:
    Serial.println("FINISH_STATE");
    break;
  }
  #endif // ifndef NO_DEBUGGING

  switch (charging_state)
  {
  case FINISH_STATE:
    goodbye();
  case BAD_STATE:
    delay(10000);
    free(charger.lcd_handle);
    charger.lcd_handle = nullptr;
    abort();
  default:
    break;
  }
}

// Readers

double readAnalogSignal(uint8_t const pin_num, Time_t const duration)
{
  int long long const beg_time = millis();
  int long long sum_of_vals = 0;
  int long cnt_of_vals = 0;

  for (int long long cur_time = beg_time; cur_time - beg_time < duration; cur_time = millis())
  {
    sum_of_vals += analogRead(pin_num);
    cnt_of_vals++;
  }

  return (((double)sum_of_vals) / ((double)cnt_of_vals));
}

void readSensorFor100ms()
{
  double const avg_A0_val = readAnalogSignal(A0, 20);

  delay(20);

  double const avg_A1_val = readAnalogSignal(A1, 60);

  V_t const V_out = (avg_A0_val / INPUT_MAX_SIGNAL_VALUE) * VOLTAGE_FOR_MAX_SIGNAL;
  V_t const V_in  = V_out * (HAM6703.MAX_INPUT_VOLTAGE / VOLTAGE_FOR_MAX_SIGNAL);
  
  measured.voltage = V_in;
  measured.current = 1000 * ((avg_A1_val / INPUT_MAX_SIGNAL_VALUE) * VOLTAGE_FOR_MAX_SIGNAL / HAM6703.voltage_ampere_ratio);

  #ifndef NO_DEBUGGIG
  Serial.print("log: measured.voltage = ");
  Serial.print(measured.voltage);
  Serial.println("[V].");
  Serial.print("log: measured.current = ");
  Serial.print(measured.current);
  Serial.println("[mA].");
  #endif // ifndef NO_DEBUGGING
}

// Printers

void showMeasuredValues()
{
  int const measured_current_int = round(measured.current);

  charger.lcd_handle->clear();

  // print voltage
  charger.lcd_handle->setCursor(0, 0);
  switch (((measured.voltage > 24.99) * 2) + ((measured.voltage < 0.01) * 1))
  {
  case 0: // when (measured.voltage >= 0.01 && measured.voltage <= 24.99)
    charger.lcd_handle->print("V = ");
    charger.lcd_handle->print(measured.voltage);
    charger.lcd_handle->print("V");
    break;

  case 1: // when (measured.voltage < 0.01)
    #ifndef NO_DEBUGGIG
    Serial.println("Warning: the loaded voltage is too small.");
    #endif // ifndef NO_DEBUGGING

    charger.lcd_handle->print("V < 0.01V");
    break;

  case 2: // when (measured.voltage > 24.99)
    #ifndef NO_DEBUGGIG
    Serial.println("Warning: the loaded voltage is too large.");
    #endif // ifndef NO_DEBUGGING

    charger.lcd_handle->print("V > 24.99V");
    break;
  }

  // print current
  charger.lcd_handle->setCursor(0, 1);
  switch (((measured_current_int > 2950) * 2) + ((measured_current_int < 50) * 1))
  {
  case 0: // when (measured.current >= 2950 && measured.current < 50)
    charger.lcd_handle->print("I = ");
    charger.lcd_handle->print(measured_current_int);
    charger.lcd_handle->print("mA");
    break;

  case 1: // when (measured.current < 50)
    #ifndef NO_DEBUGGIG
    Serial.println("Warning: the loaded current is too small.");
    #endif // ifndef NO_DEBUGGING

    charger.lcd_handle->print("I_in < 50mA");
    break;

  case 2: // when (measured.current > 2950)
    #ifndef NO_DEBUGGIG
    Serial.println("Warning: the loaded current is too large.");
    #endif // ifndef NO_DEBUGGING

    charger.lcd_handle->print("I_in > 2950mA");
    break;
  }
}

void greeting()
{
  charger.lcd_handle->clear();

  charger.lcd_handle->setCursor(0, 0);
  charger.lcd_handle->print("SYSTEM ONLINE");
  for (int cnt_dot = 0; cnt_dot < 3; cnt_dot++)
  {
    delay(500);
    charger.lcd_handle->print(".");
  }
  delay(500);
}

void goodbye()
{
  charger.lcd_handle->clear();

  for (int i = 0; i < 3; i++)
  {
    charger.lcd_handle->noBacklight();
    delay(100);

    charger.lcd_handle->backlight();
    delay(100);
  }

  charger.lcd_handle->setCursor(0, 0);
  charger.lcd_handle->print("FULL CHARGED");
}

// Initializers

void initializePins()
{
  pinMode(VoltagePin, INPUT);
  pinMode(CurrentPin, INPUT);
  pinMode(PwmPin, OUTPUT);
  setPWM(OCV_AT_SOC_100);
}

void initializeLCD(int const row_dim, int const col_dim)
{
  boolean failed = true; // result of this function

  if (row_dim > 0 && col_dim > 0)
  {
    for (byte adr = 0x01; adr < 0xFF; adr++) // See ref [2]
    {
      byte response = 4;

      Wire.beginTransmission(adr);
      response = Wire.endTransmission(adr);

      if (response == 0) // when `adr` is the address of a device
      {
        #ifndef NO_DEBUGGIG
        Serial.print("log: address found: address = 0x");
        if (adr < 16)
        {
          Serial.print("0");
        }
        Serial.print(adr, HEX);
        Serial.println(".");
        #endif // ifndef NO_DEBUGGING

        // make an LCD handle
        charger.lcd_handle = new LiquidCrystal_I2C(adr, row_dim, col_dim);

        if (charger.lcd_handle) // when the LCD handle is good
        {
          #ifndef NO_DEBUGGIG
          Serial.print("log: I2C connected: address = 0x");
          if (adr < 16)
          {
            Serial.print("0");
          }
          Serial.print(adr, HEX);
          Serial.println(".");
          #endif // ifndef NO_DEBUGGING

          break; // break the loop `for (byte adr = 0x01; adr < 0xFF; adr++)`
        }
      }
      else if (response == 0) // when an error occurs in the address `adr`
      {
        #ifndef NO_DEBUGGING
        Serial.print("Warning: unknown error at 0x");
        if (adr < 16)
        {
          Serial.print("0");
        }
        Serial.print(adr, HEX);
        Serial.println(".");
        #endif // ifndef NO_DEBUGGING
      }
    }

    if (charger.lcd_handle) // if we have a good LCD handle
    {
      failed = false;
      charger.lcd_handle->init();
      charger.lcd_handle->backlight();
      charger.lcd_handle->clear();
    }
    else
    {
      #ifndef NO_DEBUGGIG
      Serial.println("ERROR: initializing LCD failed.");
      #endif // ifndef NO_DEBUGGING
    }
  }
  else
  {
    #ifndef NO_DEBUGGIG
    Serial.println("ERROR: initializing LCD failed.");
    #endif // ifndef NO_DEBUGGING
  }
  
  if (failed)
  {
    setState(BAD_STATE);
  }
}
