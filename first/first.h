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

#ifndef FIRST_H
#define FIRST_H

// Includes

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Macros

#define VOLTAGE_FOR_MAX_SIGNAL      5.0
#define MAX_SIGNAL                  1024.0
#define R_1                         30000.0
#define R_2                         7500.0

// Types

enum ErrorCode_t {
  NO_ERROR = 0,
  INIT_FAIL = 1,
};

typedef double Voltage_t;

// Declarations

ErrorCode_t initializeLCD(int lcd_width, int lcd_height);
Voltage_t measureVoltage();
void printMeasuredVoltage(Voltage_t measured_voltage);
void greeting();

#endif // ifndef FIRST_H
