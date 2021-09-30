#ifndef THIRD_H
#define THIRD_H

#include <Arduino.h>
#include <ACS712.h>
#include <LiquidCrystal_I2C.h>

// Types

typedef double mA_t;
typedef double V_t;
typedef double Ohm_t;
typedef unsigned int long Time_t;
enum State_t { BAD_STATE = 0, CC_STATE, CV_STATE, FINISH_STATE };

// Macros

#define VoltagePin                  A0
#define CurrentPin                  A1
#define PwmPin                      3
#define VOLTAGE_FOR_MAX_SIGNAL      5.0
#define INPUT_MAX_SIGNAL_VALUE      1024
#define OCV_AT_SOC_100              4.2
#define OCV_AT_SOC_0                3.0
#define BATTERY_CAPACITY            3400.0
#define CHARGING_COEFFICIENT        0.5
#define CUTOFF_AMPERE               2000.0
#define CUTOFF_VOLTAGE              5.0
#define VOLTAGE_OUTPUT_AT_255       6.0

// Declarations

/* "table.cpp" */
V_t lookupOcvTable(Time_t run_time);

/* Core */
V_t findVoltage(mA_t wanted_current);
void analyzeCell();

/* Setters */
void setState(State_t new_state);
void setPWM(V_t wanted_voltage);

/* Readers */
double readAnalogSignal(uint8_t pin_num, Time_t duration);
void readSensorFor100ms();

/* Printers */
void greeting();
void goodbye();
void showMeasuredValues();

/* Initializers */
void initializePins();
void initializeLCD(int row_dim, int col_dim);

#endif
