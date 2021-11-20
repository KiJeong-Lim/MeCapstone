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

/* Dependencies
** [LiquidCrystal_I2C]
** 1. description = "A library for DFRobot I2C LCD displays"
** 2. repository  = "https://github.com/marcoschwartz/LiquidCrystal_I2C.git"
** 3. release-url = "https://github.com/johnrickman/LiquidCrystal_I2C/releases/tag/1.1.3"
** 4. interface   = "LiquidCrystal_I2C.h"
*/

// Please do NOT manipulate anything other than these macros:
#define SERIAL_PORT         9600
#define OPERATING_MODE      1
#define LCD_WIDTH           16
#define LCD_HEIGHT          2
#define LCD_SECTION_EA      2

/* History
** [2021-11-14]
** 1. Made new files `CapstoneStable/version.h`,
**                   `CapstoneStable/header.h`,
**                   `CapstoneStable/formatter.hpp`,
**                   `CapstoneStable/printer.ino`,
**                   `CapstoneStable/utility.ino`,
**                   `CapstoneStable/CapstoneStable.ino`.
** 2. Relaced `BufferWithFormat` with `Formatter<LCD_SECTION_LEN>`.
** 3. Set `VERSION` to be `0.10`.
** [2021-11-15]
** 1. Inserted `Wire.begin();` in the method `BMS::initialize`.
**    - This is a very important change.
**    - If `Wire.begin();` is missed,
**      the function `openLcdI2c` will not work.
** 2. Moved `CapstoneStable/header.h` -> `CapstoneStable/header.hpp`.
** 3. Fixed the method `LcdPrettyPrinter::~LcdPrettyPrinter`.
**    - Logic changed,
**      old-version = `mybuf[c][LCD_WIDTH - 1] = '\0';`,
**      new-version = `mybuf[c][LCD_WIDTH] = '\0';`.
** 4. Made new file `CapstoneStable/ocv.ino`.
**    - For deriving OCV by looking-up the soc-ocv table.
** 5. Set `VERSION` to be `0.20`.
** [2021-11-16]
** 1. Fixed the class `SerialPrinter`.
** 2. Improved the function `Formatter::putDouble`.
** 3. Fixed the class `Timer`.
** 4. Moved `CapstoneStable/ocv.ino` -> `CapstoneStable/soc.ino`.
** 5. Set `VERSION` to be `0.30`.
** [2021-11-17]
** 1. Moved `CapstoneStable/version.h`          -> `capstone/version.h`,
**          `CapstoneStable/header.hpp`         -> `capstone/header.h`,
**          `CapstoneStable/formatter.hpp`      -> `capstone/formatter.hpp`,
**          `CapstoneStable/printer.ino`        -> `capstone/printer.ino`,
**          `CapstoneStable/utility.ino`        -> `capstone/utility.ino`,
**          `CapstoneStable/soc.ino`            -> `capstone/soc.ino`,
**          `CapstoneStable/CapstoneStable.ino` -> `capstone/capstone.ino`.
** 2. Fixed the schematic of [2021-11-09].
** 3. Improved the class `SerialPrinter`.
** 4. Improved the class `Formatter`.
** 5. Set `VERSION` to be `0.40`.
** [2021-11-18]
** 1. Made new file `capstone/pinhandlers.ino`.
** 2. Moved `capstone/utility.ino`   -> `capstone/utilities.ino`,
**          `capstone/formatter.hpp` -> `capstone/formatters.hpp`,
**          `capstone/printer.ino`   -> `capstone/printers.ino`,
**          `capstone/soc.ino`       -> `capstone/data.ino`,
**          `capstone/header.h`      -> `capstone/capstone.h`.
** 3. Improved the method `BMS::checkSocOf`.
** [2021-11-19]
** 1. Removed file `capstone/formatters.hpp`.
** 2. Made new method `BMS::getCalibrationOfIin`.
** 3. Moved `capstone/capstone.h`      -> `capstone/capstone.hpp`,
**          `capstone/utilities.ino`   -> `capstone/utilities.cpp`,
**          `capstone/printers.ino`    -> `capstone/printers.cpp`,
**          `capstone/pinhandlers.ino` -> `capstone/pinhandlers.cpp`,
**          `capstone/data.ino`        -> `capstone/data.cpp`.
** 4. Fixed the method `BMS::updateQs`.
**    - Logic changed,
**      old-version = `Qs[i] += (Iin / number_of_cell_being_charged) * (millis() - Qs_lastUpdatedTime) / 3600;`,
**      new-version = `Qs[i] += Iin * (millis() - Qs_lastUpdatedTime) / 3600;`.
** 5. Fixed the method `BMS::initialize`.
** 6. Added information about dependencies.
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
