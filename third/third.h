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

#endif
