#ifndef THIRD_H
#define THIRD_H

#include <Arduino.h>
#include <ACS712.h>
#include <LiquidCrystal_I2C.h>

// Types

typedef double mA_t;
typedef double Voltage_t;
typedef double Ohm_t;
typedef unsigned int long Time_t;
enum State_t { BAD_STATE = 0, WORKING_STATE, FINISH_STATE };

// Macros

#define VoltagePin                  A0
#define CurrentPin                  A1
#define PwmPin                      3
#define VOLTAGE_FOR_MAX_SIGNAL      5.0
#define INPUT_MAX_SIGNAL_VALUE      1024

#endif
