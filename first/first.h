#ifndef FIRST_H
#define FIRST_H

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

#endif
