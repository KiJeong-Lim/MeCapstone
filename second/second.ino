#include "second.h"

LiquidCrystal_I2C *main_lcd_handle = nullptr;
ACS712 *current_sensor_handle = nullptr;

boolean initializeLCD(int lcd_width, int lcd_height);
boolean initializePins();
double measureAnalogSignal(uint8_t pin_num, Time_t duration);
V_t measureVoltage();
mA_t measureCurrent();
void showMeasuredValues(const V_t measured_voltage, const mA_t measured_current);

inline V_t calculateVoltage(const double avg_value)
{
/** Note: The principle of voltage divider
  <Voltage Divisor>
  * V_A ===>  +---< V_in  *
  *           |           *
  *           |           *
  *          R_1          *
  *           |           *
  *           |           *
  * V_B ===>  +---< V_out *
  *           |           *
  *           |           *
  *          R_2          *
  *           |           *
  *           |           *
  * V_C ===> GND          *
  <Relationship between `V_in` and `V_out` in the above figure>
  * V_in = V_out / (R_2 / (R_1 + R_2)). See ref [3]
  <In the case of the above wiring>
  * V_A = `(+)` of [Battery] = `VCC` of [Voltage Sensor].
  * V_B = `A0` of [Arduino] = `S` of [Voltage Sensor].
  * V_C = `GND` of [Arduino] = `-` of [Voltage Sensor].
  * R_1 = the resistance between `VCC` of [Voltage Sensor] and `S` of [Voltage Sensor].
  * R_2 = the resistance between `S` of [Voltage Sensor] and `-` of [Voltage Sensor].
  <Core Wiring and Voltage Divison>
  *                        +---< (+)     *
  *                        |      |      *
  *                        |      |      *
  *                        |     V_in    *
  *                        |      |      *
  *            VCC >-------+     \|/     *
  *            GND >-----------< (-)     *
  *              S >-------+   [Battery] *
  *              - >---+   |             *
  * [Voltage Sensor]   |   +---< A0      *
  *                    |          |      *
  *                    |          |      *
  *                    |         V_out   *
  *                    |          |      *
  *                    |         \|/     *
  *                    +-------< GND     *
  *                            [Arduino] *
  <The conclusion>
  * R_1 = 30000.0 [Ohm]
  * R_2 = 7500.0  [Ohm]
  * -------------------
  * V_out = V_in * 0.2
***/

  const Ohm_t R_1 = 30000.0;
  const Ohm_t R_2 = 7500.0;

  const V_t V_out = (avg_value / INPUT_MAX_SIGNAL_VALUE) * VOLTAGE_FOR_MAX_SIGNAL; // See ref [1]
  const V_t V_in = V_out / (R_2 / (R_1 + R_2)); // See ref [1]

  return V_in;
}

inline mA_t calculateCurrent(const double avg_value)
{
  return (avg_value / INPUT_MAX_SIGNAL_VALUE) * VOLTAGE_FOR_MAX_SIGNAL;
}

void setup()
{
  #ifndef NO_DEBUGGING
  Serial.begin(9600);
  Serial.println("log: Runtime started.");
  #endif // ifndef NO_DEBUGGING

  Wire.begin();

  initializeLCD(16, 2);
  initializePins();

  greeting();
}

void loop()
{
  const V_t measured_voltage = measureVoltage(); // [V]
  const mA_t measured_current = measureCurrent(); // [mA]

  #ifndef NO_DEBUGGIG
  Serial.print("log: measured_voltage = ");
  Serial.print(measured_voltage);
  Serial.println("[V].");
  Serial.print("log: measured_current = ");
  Serial.print(measured_current);
  Serial.println("[mA].");
  #endif // ifndef NO_DEBUGGING

  showMeasuredValues(measured_voltage, measured_current);

  delay(300);
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
        #ifndef NO_DEBUGGING // ifndef NO_DEBUGGING
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
  return true;
}

V_t measureVoltage()
{
  const double avg_val = measureAnalogSignal(VoltagePin, 100);

  return (calculateVoltage(avg_val));
}

mA_t measureCurrent()
{
  const double avg_val = measureAnalogSignal(CurrentPin, 100);

  return (calculateCurrent(avg_val));
}

void showMeasuredValues(const V_t measured_voltage, const mA_t measured_current)
{
  main_lcd_handle->clear();

  main_lcd_handle->setCursor(0, 0);
  switch (((measured_voltage > 4.99) * 2) + ((measured_voltage < 0.01) * 1))
  {
  case 0: // when (measured_voltage >= 0.01 && measured_voltage <= 4.99)
    main_lcd_handle->print("V_in = ");
    main_lcd_handle->print(measured_voltage);
    main_lcd_handle->print("V");
    main_lcd_handle->setCursor(0, 1);
    main_lcd_handle->print("I_in = ");
    main_lcd_handle->print(measured_current);
    main_lcd_handle->print("mA");
    break;

  case 1: // when (measured_voltage < 0.01)
    #ifndef NO_DEBUGGIG
    Serial.println("Warning: the input voltage is too small.");
    #endif // ifndef NO_DEBUGGING

    main_lcd_handle->print("V_in < 0.01V");
    break;

  case 2: // when (measured_voltage > 4.99)
    #ifndef NO_DEBUGGIG
    Serial.println("Warning: the input voltage is too large.");
    #endif // ifndef NO_DEBUGGING

    main_lcd_handle->print("V_in > 4.99V");
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
