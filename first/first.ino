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
  <Setting>
  * [.vscode/arduino.json]
  * ```
  * {
  *   "sketch": "first/first.ino",
  *   "board": "arduino:avr:uno",
  *   "port": "COM3",
  *   "output": "first/build"
  * }
  * ```
***/

// Includes

#include "first.h"

// Module Functions

static ErrorCode_t initializeLCD(int lcd_width, int lcd_height);
static V_t measureVoltage();
static void printMeasuredVoltage(V_t measured_voltage);
static void greeting();

// Module Variables

static LiquidCrystal_I2C *main_lcd_handle = nullptr;
static ErrorCode_t err_code = NO_ERROR;

// Implementations

inline V_t calculateVoltage(const double avgerage_signal)
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

  const V_t V_out = (avgerage_signal / MAX_SIGNAL) * VOLTAGE_FOR_MAX_SIGNAL; // See ref [1]
  const V_t V_in = V_out / (R_2 / (R_1 + R_2)); // See ref [1]

  return V_in;
}

inline void checkOkay()
{
  switch (err_code)
  {
  case NO_ERROR:
    return;

  default:
    free(main_lcd_handle);
    main_lcd_handle = nullptr;
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
  #ifndef NO_DEBUGGING
  Serial.begin(9600);
  Serial.println("log: Runtime started.");
  #endif // ifndef NO_DEBUGGING
  
  Wire.begin();
  err_code = initializeLCD(16, 2);
  checkOkay();

  greeting();
  warmingupAnalogPin(A0);
}

void loop()
{
  const V_t measured_voltage = measureVoltage(); // [V]

  main_lcd_handle->clear();

  // print `measured_voltage`
  main_lcd_handle->setCursor(0, 0);
  printMeasuredVoltage(measured_voltage);
}

static V_t measureVoltage()
{
  const int long long beg_time = millis(); // the time when this function was called
  int long long sum_of_vals = 0; // sum of values obtained from the analog pin `A0`
  int long cnt_of_vals = 0; // number of how many times we call the function `analogRead`

  // keep calling the function `analogRead` for 100 milli seconds
  for (int long long cur_time = beg_time; cur_time - beg_time < 100; cur_time = millis())
  {
    sum_of_vals += analogRead(A0);
    cnt_of_vals++;
  }
  
  // calculate the input voltage from the average value
  return calculateVoltage(((double)sum_of_vals) / ((double)cnt_of_vals));
}

static ErrorCode_t initializeLCD(const int row_dim, const int col_dim)
{
  ErrorCode_t my_err_code = INIT_FAIL; // result of this function

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
      my_err_code = NO_ERROR;
      main_lcd_handle->init();
      main_lcd_handle->backlight();
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
  return my_err_code;
}

static void printMeasuredVoltage(const V_t measured_voltage)
{
  #ifndef NO_DEBUGGIG
  Serial.print("log: measured_voltage = ");
  Serial.print(measured_voltage);
  Serial.println("[V].");
  #endif // ifndef NO_DEBUGGING

  switch (((measured_voltage > 4.99) * 2) + ((measured_voltage < 0.01) * 1))
  {
  case 0: // when (measured_voltage >= 0.01 && measured_voltage <= 4.99)
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

  case 2: // when (measured_voltage > 4.99)
    #ifndef NO_DEBUGGIG
    Serial.println("Warning: the input voltage is too large.");
    #endif // ifndef NO_DEBUGGING

    main_lcd_handle->print("V_in > 4.99V");
    break;
  }
}

static void greeting()
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
  
  main_lcd_handle->setCursor(0, 1);
  main_lcd_handle->print("# VOLTAGE SENSOR");
  delay(1000);
}
