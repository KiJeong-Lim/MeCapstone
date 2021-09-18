/** THE FIRST STEP ON OUR CAPSTONE PROJECT
  <Abstract>
  * Measuring the voltage of a given battery and printing it.
  <Members>
  * Hwanhee Jeon [School of Mechanical Engineering, Chonnam National University]
  * Hakjung Im [School of Mechanical Engineering, Chonnam National University]
  * Kijeong Lim [School of Mechanical Engineering, Chonnam National University]
  <Wiring Diagram>
  *   IOREF *           +---------------< -              *
  *   RESET *           |               * +              *
  *    3.3V *           |   +-----------< S              *
  *      5V >-------+   |   |   +-------< GND            *
  *     GND >-------|---+   |   |   +---< VCC            *
  *     GND >---+   |       |   |   |   [Voltage Sensor] *
  *     Vin *   |   |       |   |   |                    *
  *      A0 >---|---|-------+   |   |                    *
  *      A1 *   |   |           |   |                    *
  *      A2 *   +---|---< GND   |   |                    *
  *      A3 *       +---< VCC   |   |                    *
  *      A4 >-----------< SDA   |   +---< (+)            *
  *      A5 >-----------< SCL   +-------< (-)            *
  * [Arduino]           [I2C]           [Battery]        *
  <Used Parts>
  * [Arduino]        Arduino Uno compatible board
  * [Battery]        NCR18650B Li-ion MH12210
  * [I2C]            Arduino LCD 16x2 display module with 4-pins I2C LCD controller
  * [Voltage Sensor] Arduino DC 0-25V voltage sensor module
  <References>
  * [1] https://www.youtube.com/watch?v=psNAeHoZv0A
  * [2] https://codingrun.com/119
  * [3] https://en.wikipedia.org/wiki/Voltage_divider
***/

// Includes

#include "first.h"

static LiquidCrystal_I2C *lcd_handle = nullptr;

// Implementations

inline Voltage_t calculateVoltage(const double avgerage_signal)
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
  *                        +---< GND     *
  *                        |     /|\     *
  *                        |      |      *
  *                        |     V_out   *
  *                        |      |      *
  *              - >-------+      |      *
  *              S >-----------< A0      *
  *            GND >-------+   [Arduino] *
  *            VCC >---+   |             *
  * [Voltage Sensor]   |   +---< (-)     *
  *                    |         /|\     *
  *                    |          |      *
  *                    |         V_in    *
  *                    |          |      *
  *                    |          |      *
  *                    +-------< (+)     *
  *                            [Battery] *
  <The conclusion>
  * R_1 = 30000.0 [Ohm]
  * R_2 = 7500.0  [Ohm]
  * -------------------
  * V_out = V_in * 0.2
***/
  const Voltage_t V_out = (avgerage_signal / MAX_SIGNAL) * VOLTAGE_FOR_MAX_SIGNAL; // See ref [1]
  const Voltage_t V_in = V_out / (R_2 / (R_1 + R_2)); // See ref [1]
  return V_in;
}

inline void abortWithErrorCode(const ErrorCode_t err_code)
{
  switch (err_code)
  {
  case NO_ERROR:
    return;
  default:
    free(lcd_handle);
    lcd_handle = nullptr;
    abort();
  }
}

inline void warmingupAnalogPin(const int pin_num)
{
  for (int cnt = 0; cnt < 100; cnt++) // call the function `analogRead` 100 times
  {
    analogRead(pin_num);
  }
}

void setup()
{
  ErrorCode_t err_code = NO_ERROR;
#ifndef NO_DEBUGGING
  Serial.begin(9600);
  Serial.println("log: Runtime started.");
#endif
  Wire.begin();
  err_code = initializeLCD(16, 2);
  if (err_code == NO_ERROR)
  {
    greeting();
    warmingupAnalogPin(A0);
  }
  else
  {
    abortWithErrorCode(err_code);
  }
}

void loop()
{
  const Voltage_t input_voltage = measureVoltage(); // [V]
  lcd_handle->clear();
  lcd_handle->setCursor(0, 0);
  printMeasuredVoltage(input_voltage);
  lcd_handle->setCursor(0, 1);
}

Voltage_t measureVoltage()
{
  const int long long beg_time = millis(); // the time when this function was called
  int long long sum_of_vals = 0; // sum of values obtained from the analog pin `A0`
  int long cnt_of_vals = 0; // number of how many times we call the function `analogRead`
  for (int long long cur_time = beg_time; cur_time - beg_time < 100; cur_time = millis())
  { // keep calling the function `analogRead` for 100 milli seconds
    sum_of_vals += analogRead(A0);
    cnt_of_vals++;
  }
  return calculateVoltage(((double)sum_of_vals) / ((double)cnt_of_vals)); // derive the input voltage from the average value
}

ErrorCode_t initializeLCD(const int row_dim, const int col_dim)
{
  ErrorCode_t err_code = INIT_FAIL;
  if (row_dim > 0 && col_dim > 0)
  {
    for (byte adr = 0x01; adr < 0xFF; adr++) // See ref [2]
    {
      byte response = 4;
      Wire.beginTransmission(adr);
      response = Wire.endTransmission(adr);
      if (response == 0)
      {
#ifndef NO_DEBUGGIG
        Serial.print("log: address found: address = 0x");
        if (adr < 16)
        {
          Serial.print("0");
        }
        Serial.print(adr, HEX);
        Serial.println(".");
#endif
        lcd_handle = new LiquidCrystal_I2C(adr, row_dim, col_dim);
        if (lcd_handle)
        {
#ifndef NO_DEBUGGIG
          Serial.print("log: I2C connected: address = 0x");
          if (adr < 16)
          {
            Serial.print("0");
          }
          Serial.print(adr, HEX);
          Serial.println(".");
#endif
          break;
        }
      }
      else if (response == 0)
      {
#ifndef NO_DEBUGGING
        Serial.print("Warning: unknown error at 0x");
        if (adr < 16)
        {
          Serial.print("0");
        }
        Serial.print(adr, HEX);
        Serial.println(".");
#endif
      }
    }

    if (lcd_handle)
    {
      err_code = NO_ERROR;
      lcd_handle->begin();
      lcd_handle->backlight();
    }
    else
    {
#ifndef NO_DEBUGGIG
      Serial.println("ERROR: initializing LCD failed.");
#endif
    }
  }
  else
  {
#ifndef NO_DEBUGGIG
    Serial.println("ERROR: initializing LCD failed.");
#endif
  }
  return err_code;
}

void printMeasuredVoltage(const Voltage_t measured_voltage)
{
#ifndef NO_DEBUGGIG
  Serial.print("log: measured_voltage = ");
  Serial.print(measured_voltage);
  Serial.println("[V].");
#endif
  switch (((measured_voltage > 4.99) * 2) + ((measured_voltage < 0.01) * 1))
  {
  case 0: // when (measured_voltage <= 0.01 && measured_voltage <= 4.99)
    lcd_handle->print("V_in = ");
    lcd_handle->print(measured_voltage);
    lcd_handle->print("V");
    break;
  case 1: // when (measured_voltage < 0.01)
#ifndef NO_DEBUGGIG
    Serial.println("Warning: the input voltage is too small.");
#endif
    lcd_handle->print("V_in < 0.01V");
    break;
  case 2: // when (measured_voltage > 4.99)
#ifndef NO_DEBUGGIG
    Serial.println("Warning: the input voltage is too large.");
#endif
    lcd_handle->print("V_in > 4.99V");
    break;
  }
}

void greeting()
{
  lcd_handle->clear();
  lcd_handle->setCursor(0, 0);
  lcd_handle->print("SYSTEM ONLINE");
  for (int cnt_dot = 0; cnt_dot < 3; cnt_dot++)
  {
    delay(500);
    lcd_handle->print(".");
  }
  delay(500);
  lcd_handle->setCursor(0, 1);
  lcd_handle->print("# VOLTAGE SENSOR");
  delay(1000);
}
