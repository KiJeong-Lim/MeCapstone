/* <CAPSTONE PROJECT>
** ===============================================================================
** MEMBERS       | AFFILIATION                                                   |
** ===============================================================================
** Hwan-hee Jeon | School of Mechanical Engineering, Chonnam National University |
** Hak-jung Im   | School of Mechanical Engineering, Chonnam National University |
** Ki-jeong Lim  | School of Mechanical Engineering, Chonnam National University |
** ===============================================================================
*/

#ifndef VERSION
#define VERSION 0.10

#define SERIAL_PORT         115200
#define LCD_WIDTH           16
#define LCD_HEIGHT          2
#define LCD_SECTION_EA      2

/* History
** [2021-11-14]
** Make new files `CapstoneStable/version.h`,
**                `CapstoneStable/header.h`,
**                `CapstoneStable/formatter.hpp`,
**                `CapstoneStable/printer.ino`,
**                `CapstoneStable/utility.ino`,
**                `CapstoneStable/CapstoneStable.ino`.
** Relace `BufferWithFormat`
**   with `Formatter<LCD_SECTION_LEN>`.
** Set `VERSION` to be `0.10`.
** [2021-11-15]
** Insert `Wire.begin();` in `BMS::initialize`. -- This is a very important change.
**                                              -- If `Wire.begin();` is missed,
**                                                 the function `openLcdI2c` will not work.
** Move `CapstoneStable/header.h` to
**      `CapstoneStable/header.hpp`.
** Fix `LcdPrettyPrinter::~LcdPrettyPrinter`. -- Relace `mybuf[c][LCD_WIDTH - 1] = '\0';`
**                                                 with `mybuf[c][LCD_WIDTH] = '\0';`
*/

#endif
