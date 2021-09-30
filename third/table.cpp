#include "third.h"

#ifndef NO_OCV_LOOKUP_TABLE

// OCV_table.length = (OCV_AT_SOC_100 - OCV_AT_SOC_0) / DECREASING_VOLTAGE_SCALE + 1
Time_t OCV_table[25] =
{ 0 // running_minutes = ? for V_cell = 3.00 [V]
, 0 // running_minutes = ? for V_cell = 3.05 [V]
, 0 // running_minutes = ? for V_cell = 3.10 [V]
, 0 // running_minutes = ? for V_cell = 3.15 [V]
, 0 // running_minutes = ? for V_cell = 3.20 [V]
, 0 // running_minutes = ? for V_cell = 3.25 [V]
, 0 // running_minutes = ? for V_cell = 3.30 [V]
, 0 // running_minutes = ? for V_cell = 3.35 [V]
, 0 // running_minutes = ? for V_cell = 3.40 [V]
, 0 // running_minutes = ? for V_cell = 3.45 [V]
, 0 // running_minutes = ? for V_cell = 3.50 [V]
, 0 // running_minutes = ? for V_cell = 3.55 [V]
, 0 // running_minutes = ? for V_cell = 3.60 [V]
, 0 // running_minutes = ? for V_cell = 3.65 [V]
, 0 // running_minutes = ? for V_cell = 3.70 [V]
, 0 // running_minutes = ? for V_cell = 3.75 [V]
, 0 // running_minutes = ? for V_cell = 3.80 [V]
, 0 // running_minutes = ? for V_cell = 3.85 [V]
, 0 // running_minutes = ? for V_cell = 3.90 [V]
, 0 // running_minutes = ? for V_cell = 3.95 [V]
, 0 // running_minutes = ? for V_cell = 4.00 [V]
, 0 // running_minutes = ? for V_cell = 4.05 [V]
, 0 // running_minutes = ? for V_cell = 4.10 [V]
, 0 // running_minutes = ? for V_cell = 4.15 [V]
, 0 // running_minutes = ? for V_cell = 4.20 [V]
};

V_t lookupOcvTable(Time_t const running_millseconds)
{
  Time_t const running_minutes = running_millseconds / 6000;

  int left_idx = 0;
  int right_idx = sizeof(OCV_table) / sizeof(OCV_table[0]) - 1;
  int mid_idx = (left_idx + right_idx) / 2;
  int dir = 0;
  V_t result_Voltage = 0.0;

  while (left_idx <= right_idx)
  {
    if (OCV_table[mid_idx] < running_millseconds)
    {
      dir = 1;
      left_idx = mid_idx + dir;
    }
    else if (OCV_table[mid_idx] > running_millseconds)
    {
      dir = -1;
      right_idx = mid_idx + dir;
    }
    else
    {
      dir = 0;
      break;
    }

    mid_idx = (left_idx + right_idx) / 2;
  }

  if (dir)
  {
    Time_t T_left = OCV_table[right_idx], T_right = OCV_table[left_idx];

    result_Voltage = (OCV_AT_SOC_0 + right_idx * DECREASING_VOLTAGE_SCALE) + (DECREASING_VOLTAGE_SCALE / (T_right - T_left)) * (running_minutes - T_left);
  }
  else
  {
    result_Voltage = OCV_AT_SOC_0 + mid_idx * DECREASING_VOLTAGE_SCALE;
  }

  return result_Voltage;
}

#endif // ifndef NO_OCV_LOOKUP_TABLE
