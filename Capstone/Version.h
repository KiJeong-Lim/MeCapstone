/* 2021-11-13 <CAPSTONE PROJECT>
** ===============================================================================
** MEMBERS       | AFFILIATION                                                   |
** ===============================================================================
** Hwan-hee Jeon | School of Mechanical Engineering, Chonnam National University |
** Hak-jung Im   | School of Mechanical Engineering, Chonnam National University |
** Ki-jeong Lim  | School of Mechanical Engineering, Chonnam National University |
** ===============================================================================
*/

#ifndef VERSION
#if defined(MAJOR_VERSION) & defined(MINOR_VERSION) & defined(REVISION_NUMBER)
#define VERSION (1.00 * (MAJOR_VERSION) + 0.10 * (MINOR_VERSION) + 0.01 * (REVISION_NUMBER))
#elif defined(MAJOR_VERSION) & defined(MINOR_VERSION)
#define VERSION (1.00 * (MAJOR_VERSION) + 0.10 * (MINOR_VERSION))
#elif defined(MAJOR_VERSION)
#define VERSION (1.00 * (MAJOR_VERSION))
#else
#define VERSION (0.00)
#endif

#if defined(WITH_NANO)
#define SERIAL_PORT_NUMBER 115200
#elif defined(WITH_UNO)
#define SERIAL_PORT_NUMBER 9600
#endif

#if MAJOR_VERSION == 0
#if MINOR_VERSION == 1
/* <SCHEMATIC>
** {- ToDo: Draw schematic here -}
*/
#define DEBUGGING
#define LCD_USE
#define CONSIDER_SUPPLY_VOLTAGE
#define CONSIDER_SUPPLY_CURRENT
#define CONTROL_BALANCE_CIRCUIT
#endif
#if MINOR_VERSION == 2
/* <SCHEMATIC>
** {- ToDo: Draw schematic here -}
*/
#define DEBUGGING
#define LCD_USE
#define CONSIDER_SUPPLY_VOLTAGE
#define CONSIDER_SUPPLY_CURRENT
#define CONTROL_BALANCE_CIRCUIT
#endif
#endif

#ifndef DEBUGGING
#define NO_DEBUGGING
#undef DEBUGGING
#endif
#ifndef LCD_USE
#define NO_LCD_USE
#undef LCD_USE
#endif
#ifndef CONSIDER_SUPPLY_VOLTAGE
#define NOT_CONSIDER_SUPPLY_VOLTAGE
#undef CONSIDER_SUPPLY_VOLTAGE
#endif
#ifndef CONSIDER_SUPPLY_CURRENT
#define NOT_CONSIDER_SUPPLY_CURRENT
#undef CONSIDER_SUPPLY_CURRENT
#endif
#ifndef CONTROL_BALANCE_CIRCUIT
#define NOT_CONTROL_BALANCE_CIRCUIT
#undef CONTROL_BALANCE_CIRCUIT
#endif

#endif
