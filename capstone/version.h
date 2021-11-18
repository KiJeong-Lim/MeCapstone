/* <CAPSTONE PROJECT>
** ===============================================================================
** MEMBER        | AFFILIATION                                                   |
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

#define SERIAL_PORT         9600
#define MODE                1
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
** Move `CapstoneStable/header.h` -> `CapstoneStable/header.hpp`.
** Fix `LcdPrettyPrinter::~LcdPrettyPrinter`. -- Relace `mybuf[c][LCD_WIDTH - 1] = '\0';`
**                                                 with `mybuf[c][LCD_WIDTH] = '\0';`.
** Make new file `CapstoneStable/ocv.ino`. -- For deriving OCV by looking-up the soc-ocv table.
** Set `VERSION` to be `0.20`.
** [2021-11-16]
** Fix the class `SerialPrinter`.
** Improve the function `Formatter::putDouble`.
** Fix the class `Timer`.
** Move `CapstoneStable/ocv.ino` -> `CapstoneStable/soc.ino`.
** Set `VERSION` to be `0.30`.
** [2021-11-17]
** Move `CapstoneStable/version.h`          -> `capstone/version.h`,
**      `CapstoneStable/header.hpp`         -> `capstone/header.h`,
**      `CapstoneStable/formatter.hpp`      -> `capstone/formatter.hpp`,
**      `CapstoneStable/printer.ino`        -> `capstone/printer.ino`,
**      `CapstoneStable/utility.ino`        -> `capstone/utility.ino`,
**      `CapstoneStable/soc.ino`            -> `capstone/soc.ino`,
**      `CapstoneStable/CapstoneStable.ino` -> `capstone/capstone.ino`.
** Fix the schematic of [2021-11-09].
** Improve the class `SerialPrinter`.
** Improve the class `Formatter`.
** Set `VERSION` to be `0.40`.
** [2021-11-18]
** Make new file `capstone/pinhandlers.ino`.
** Move `capstone/utility.ino`   -> `capstone/utilities.ino`,
**      `capstone/formatter.hpp` -> `capstone/formatters.hpp`,
**      `capstone/printer.ino`   -> `capstone/printers.ino`,
**      `capstone/soc.ino`       -> `capstone/data.ino`,
**      `capstone/header.h`      -> `capstone/capstone.h`.
** Improve the method to check SoC of cells.
*/

/* Schematics
** [2021-11-09]
**                             +---< R
**             +---------------|---< A
**      5V >---|---< 1kOhm >---+---< K
**     GND >---+               |   [TL431]
**      A0 >---|---+           |
**      A1 >---|---|-----------+
** [Arudino]   |   |
**             |   +-----------+
**             |               |
**             +---< 2kOhm >---+---< 18kOhm >---+
**             |                                |
**             +---< (-) Cell (+) >-------------+
*/

#endif
