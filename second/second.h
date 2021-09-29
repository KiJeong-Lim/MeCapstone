#ifndef SECOND_H
#define SECOND_H

// Includes

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
#define VOLTAGE_FOR_MAX_SIGNAL      5.0
#define INPUT_MAX_SIGNAL_VALUE      1024

#endif
