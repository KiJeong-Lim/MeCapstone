#include "third.h"

/**
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
  * |   |       |   |   * IOREF # AREF *                    #============#
  * |   |       |   |   * RESET #  GND >----------------+---< - SOURCE + >---+
  * |   |       |   |   * 3.3V  #   13 *                |   #============#   |
  * |   |       +---|---< 5V    #   12 *                |                    |
  * |   |           +---< GND   #  ~11 *                |       #========#   |
  * |   |   +-----------< GND   #  ~10 *                |   +---< R (#1) >---+
  * |   |   |           * Vin   #   ~9 *                |   |   #========#   |
  * |   |   |           #       #    8 *                |   |                |
  * |   |   |           #       #    7 *                |   |   #=====#      |
  * |   |   |           #       #   ~6 *                |   |   # NPN #      |
  * |   |   |   +-------< A0    #   ~5 *                |   |   # === #      |
  * |   |   |   |   +---< A1    #    4 *   #========#   |   +---< C   #      |
  * |   |   |   |   |   * A2    #   ~3 >---< R (#2) >---|---|---< B   #      |
  * |   |   |   |   |   * A3    #    2 *   #========#   +---|---< E   #      |
  * |   +---|---|---|---< A4    #    1 *                |   |   #=====#      |
  * +-------|---|---|---< A5    #    0 *                |   |                |
  *         |   |   |   #==============#                |   +------------+   |
  *         |   |   |                                   |                |   |
  *         |   |   |                                   |   #========#   |   |
  *         |   |   |                                   |   # MOFSET #   |   |
  *         |   |   |   #==========#                    |   # ====== #   |   |
  *         |   |   |   # Sensor   #                    |   #      G >---+   |
  *         |   |   |   # ======== #                    |   #      D >-------+
  *         |   +---|---< VT   Vin >-----------+        |   #      S >-----------+
  *         |       +---< AT   GND >-----------|--------+   #========#           |
  *         +-----------< GND Vout >-------+   |        |                        |
  *                     * GND  GND >---+   |   |        |   #===========#        |
  *                     #==========#   |   |   |        +---< + Diode - >--------+
  *                                    |   |   |        |   #===========#        |
  *                 #==============#   |   |   |        |                        |
  *             +---< (+) Cell (-) >---+   |   |        |         #==========#   |
  *             |   #==============#       |   +--------|-----+---< Inductor >---+
  *             |                          |            |     |   #==========#
  *             +--------------------------+            |     |
  *                                                     |     |   #===========#
  *                                                     |     +---< Capacitor >---+
  *                                                     |         #===========#   |
  *                                                     |                         |
  *                                                     +-------------------------+
  <Used Parts>
  * [Arduino] Arduino Uno (R3)
  * [Capacitor] 2200[μF] 25[V]
  * [Cell] NCR18650B Li-ion MH12210
  * [Diode] 1N4001
  * [Inductor] 100[mH]
  * [LCD (I2C)] Arduino LCD 16x2 display module with 4-pins I2C LCD controller
  * [MOSFET] IRFP260N
  * [NPN] C1815
  * [R (#1)] 10[kΩ]
  * [R (#2)] 10[kΩ]
  * [Sensor] MAX471 25[V] 3[A] (HAM6703)
  <References>
  [1] https://m.blog.naver.com/ysahn2k/222074476103
***/

State_t system_state = WORKING_STATE;
LiquidCrystal_I2C *main_lcd_handle = nullptr;

boolean initializeLCD(int lcd_width, int lcd_height);
boolean initializePins();
double measureAnalogSignal(uint8_t pin_num, Time_t duration);
Voltage_t measureVoltage();
mA_t measureCurrent();
void setPWM(double duty_ratio);
void showValues(const Voltage_t measured_voltage, const mA_t measured_current);
double calculateDutyRatio(const Voltage_t measured_voltage, const mA_t measured_current);

inline void setState(State_t new_state)
{
  system_state = new_state;

  #ifndef NO_DEBUGGING
  Serial.print("log: system_state = ");
  switch (system_state)
  {
  case BAD_STATE:
    Serial.println("BAD_STATE");
    break;
  case WORKING_STATE:
    Serial.println("WORKING_STATE");
    break;
  case FINISH_STATE:
    Serial.println("FINISH_STATE");
    break;
  }
  #endif // ifndef NO_DEBUGGING

  switch (system_state)
  {
  case FINISH_STATE:
    main_lcd_handle->clear();
    main_lcd_handle->setCursor(0, 0);
    main_lcd_handle->print("FULL CHARGED");
  case BAD_STATE:
    delay(10000);
    abort();
  case WORKING_STATE:
    break;
  }
}

inline Voltage_t calculateVoltage(const double avg_value)
{
  const Voltage_t MAX_INPUT_VOLTAGE = 25.0; // [V]
  const Voltage_t V_out = (avg_value / INPUT_MAX_SIGNAL_VALUE) * VOLTAGE_FOR_MAX_SIGNAL;
  const Voltage_t V_in = V_out * (MAX_INPUT_VOLTAGE / VOLTAGE_FOR_MAX_SIGNAL);

  return V_in;
}

inline mA_t calculateCurrent(const double avg_value)
{
  const Ohm_t voltage_ampere_ratio_of_SENSOR = 1.0; // [V/A]

  return 1000 * ((avg_value / INPUT_MAX_SIGNAL_VALUE) * VOLTAGE_FOR_MAX_SIGNAL / voltage_ampere_ratio_of_SENSOR);
}

void setup()
{
  boolean is_good = true;

  #ifndef NO_DEBUGGING
  Serial.begin(9600);
  Serial.println("log: Runtime started.");
  #endif // ifndef NO_DEBUGGING

  Wire.begin();

  is_good &= initializeLCD(16, 2);
  is_good &= initializePins();

  if (is_good)
  {
    greeting();
  }
  else
  {
    setState(BAD_STATE);
  }
}

void loop()
{
  const Voltage_t measured_voltage = measureVoltage(); // [V]
  const mA_t measured_current = measureCurrent(); // [mA]

  #ifndef NO_DEBUGGIG
  Serial.print("log: measured_voltage = ");
  Serial.print(measured_voltage);
  Serial.println("[V].");
  Serial.print("log: measured_current = ");
  Serial.print(measured_current);
  Serial.println("[mA].");
  #endif // ifndef NO_DEBUGGING

  showValues(measured_voltage, measured_current);

  const double duty_ratio = calculateDutyRatio(measured_voltage, measured_current); // 0.0 ~ 1.0

  #ifndef NO_DEBUGGIG
  Serial.print("log: duty_ratio = ");
  Serial.print(duty_ratio);
  Serial.println(".");
  #endif // ifndef NO_DEBUGGING

  setPWM(duty_ratio);

  delay(300);
}

double calculateDutyRatio(const Voltage_t measured_voltage, const mA_t measured_current)
{

  // TO DO: implement this function!

}

void setPWM(double duty_ratio)
{
  if (duty_ratio >= 0 && duty_ratio <= 1)
  {
    const int duty_val = 255.0 * duty_ratio;

    analogWrite(PwmPin, duty_val);
  }
}

double measureAnalogSignal(uint8_t pin_num, Time_t duration)
{
  const int long long beg_time = millis();
  int long long sum_of_vals = 0;
  int long cnt_of_vals = 0;

  for (int long long cur_time = beg_time; cur_time - beg_time < duration; cur_time = millis())
  {
    sum_of_vals += analogRead(pin_num);
    cnt_of_vals++;
  }

  return (double)sum_of_vals / (double)cnt_of_vals;
}

boolean initializeLCD(const int row_dim, const int col_dim)
{
  boolean successed = false; // result of this function

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
        main_lcd_handle = new LiquidCrystal_I2C(adr, row_dim, col_dim);

        if (main_lcd_handle) // when the LCD handle is good
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
    if (main_lcd_handle) // if we have a good LCD handle
    {
      successed = true;
      main_lcd_handle->init();
      main_lcd_handle->backlight();
      main_lcd_handle->clear();
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

  return successed;
}

boolean initializePins()
{
  pinMode(VoltagePin, INPUT);
  pinMode(CurrentPin, INPUT);
  pinMode(PwmPin, OUTPUT);

  return true;
}

Voltage_t measureVoltage()
{
  const double avg_val = measureAnalogSignal(VoltagePin, 100);

  return (calculateVoltage(avg_val));
}

mA_t measureCurrent()
{
  const double avg_val = measureAnalogSignal(CurrentPin, 100);

  return (calculateCurrent(avg_val));
}

void showValues(const Voltage_t measured_voltage, const mA_t measured_current)
{
  main_lcd_handle->clear();

  main_lcd_handle->setCursor(0, 0);
  switch (((measured_voltage > 24.99) * 2) + ((measured_voltage < 0.01) * 1))
  {
  case 0: // when (measured_voltage >= 0.01 && measured_voltage <= 24.99)
    main_lcd_handle->print("V_in = ");
    main_lcd_handle->print(measured_voltage);
    main_lcd_handle->print("V");
    break;

  case 1: // when (measured_voltage < 0.01)
    #ifndef NO_DEBUGGIG
    Serial.println("Warning: the input voltage is too small.");
    #endif // ifndef NO_DEBUGGING

    main_lcd_handle->print("V_in < 0.01V");
    break;

  case 2: // when (measured_voltage > 24.99)
    #ifndef NO_DEBUGGIG
    Serial.println("Warning: the input voltage is too large.");
    #endif // ifndef NO_DEBUGGING

    main_lcd_handle->print("V_in > 24.99V");
    break;
  }

  main_lcd_handle->setCursor(0, 1);
  switch (((measured_current > 2950.0) * 2) + ((measured_current < 50.0) * 1))
  {
  case 0: // when (measured_current >= 2950 && measured_current < 50)
    main_lcd_handle->print("I_in = ");
    main_lcd_handle->print(measured_current);
    main_lcd_handle->print("mA");
    break;

  case 1: // when (measured_current < 50)
    #ifndef NO_DEBUGGIG
    Serial.println("Warning: the input current is too small.");
    #endif // ifndef NO_DEBUGGING

    main_lcd_handle->print("I_in < 50mA");
    break;

  case 2: // when (measured_current > 2950)
    #ifndef NO_DEBUGGIG
    Serial.println("Warning: the input current is too large.");
    #endif // ifndef NO_DEBUGGING

    main_lcd_handle->print("I_in > 2950mA");
    break;
  }
}

void greeting()
{
  main_lcd_handle->clear();

  main_lcd_handle->setCursor(0, 0);
  main_lcd_handle->print("SYSTEM ONLINE");
  for (int cnt_dot = 0; cnt_dot < 3; cnt_dot++)
  {
    delay(500);
    main_lcd_handle->print(".");
  }
  delay(500);
}
