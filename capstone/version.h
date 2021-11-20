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
#define VERSION           (1.00 * (MAJOR_VERSION) + 0.10 * (MINOR_VERSION) + 0.01 * (REVISION_NUMBER))
#elif defined(MAJOR_VERSION) & defined(MINOR_VERSION)
#define VERSION           (1.00 * (MAJOR_VERSION) + 0.10 * (MINOR_VERSION))
#elif defined(MAJOR_VERSION)
#define VERSION           (1.00 * (MAJOR_VERSION))
#else
#define VERSION           (0.00)
#endif

// Please do NOT manipulate any macros other than these:
#define SERIAL_PORT       9600
#define OPERATING_MODE    1
#define LCD_WIDTH         16
#define LCD_HEIGHT        2
#define LCD_SECTION_EA    2

/* Dependencies
** [LiquidCrystal_I2C]
** 1. description = "A library for DFRobot I2C LCD displays"
** 2. repository  = "https://github.com/marcoschwartz/LiquidCrystal_I2C.git"
** 3. release.url = "https://github.com/johnrickman/LiquidCrystal_I2C/releases/tag/1.1.3"
** 4. interface   = "LiquidCrystal_I2C.h"
*/

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
**    - Code changed,
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
**    - Code changed,
**      old-version = `Qs[i] += (Iin / number_of_cell_being_charged) * (millis() - Qs_lastUpdatedTime) / 3600;`,
**      new-version = `Qs[i] += Iin * (millis() - Qs_lastUpdatedTime) / 3600;`.
** 5. Fixed the method `BMS::initialize`.
** 6. Added information about dependencies.
** [2021-11-20]
** 1. Uploaded schematics.
** 2. Correct the product name of the DC power supply.
*/

/* Schematics
** [Circuit-A]
** - Operating dates
**   [#1] 2021.11.09
** - Parts list   Product name                     Quantity
**   [Arduino]    Arduino Uno Rev3                 x 1
**   [Cell]       Panasonic 18650 Li-ion NCR18650G x 1
**   [Resistor]   1kOhm                            x 1
**                2kOhm                            x 1
**                18kOhm                           x 1
**   [ZenerDiode] TL431BVLPRAGOSCT-ND              x 1
** - Diagram
** >                             +---< R
** >             +---------------|---< A
** >      5V >---|---< 1kOhm >---+---< K
** >     GND >---+               |   [ZenerDiode]
** >      A0 >---|---+           |
** >      A1 >---|---|-----------+
** > [Arduino]   |   |
** >             |   +-----------+
** >             |               |
** >             +---< 2kOhm >---+---< 18kOhm >---+
** >             |                                |
** >             +---< - [Cell] + >---------------+
** - Notes
**   1. This circuit will not work if `VERSION` >= 0.4.0.
** [Circuit-B]
** - Operating dates
**   [#1] 2021.11.16
** - Parts list     Product name                           Quantity
**   [AmpereSensor] ACS712ELCTR-20A-T                      x 1
**   [Arduino]      Arduino Uno Rev3                       x 1
**   [Cell]         Panasonic 18650 Li-ion NCR18650G       x 1
**   [LCDI2C]       Arduino I2C 1602 LCD module SZH-EK101  x 1
**   [N-MOSFET]     RFP12N10L N-Channel Power MOSFET       x 1
**   [NPN]          2N2222 NPN Transistor                  x 2
**   [P-MOSFET]     IRF4905PBF P-Channel Power MOSFET      x 1
**   [PNP]          2N3906 PNP Transistor                  x 1
**   [PowerSupply]  SMART RDP-305 DC Power Supply          x 1
**   [RedLED]       microtivity IL011 5mm Diffused Red LED x 1
**   [Resistor]     120Ohm                                 x 1
**                  220Ohm                                 x 1
**                  330Ohm                                 x 2
**                  1kOhm                                  x 3
**                  2kOhm                                  x 1
**                  18kOhm                                 x 1
**                  100kOhm                                x 2
**   [WCR5OhmJ5W]   Wire Wound Cement Resistor EPX7RBL3    x 1
**   [ZenerDiode]   TL431BVLPRAGOSCT-ND                    x 1
** - Diagram
** >             +---------------< GND
** >             |   +-----------< VCC
** >             |   |   +-------< SDA
** >             |   |   |   +---< SCL
** >             |   |   |   |   [LCDI2C]
** >      5V >---|---+---|---|---------------+-------< Vcc    In1 >-----------------------------------+
** >     GND >---+       |   |               |   +---< Out                                            |
** >     GND >-----------|---|-----------+---|---|---< Gnd    In2 >---+                               |
** >      A0 >-----------|---|-------+   |   |   |   [AmpereSensor]   |                               |
** >      A1 >-----------|---|---+   |   |   |   |                    |                               |
** >      A2 >-----------|---|---|---|---|---|---+                    |                               |
** >      A4 >-----------+   |   |   |   |   |                        +---< (+)                       |
** >      A5 >---------------+   |   |   +---|----------------------------< (-)                       |
** >       2 >-------+           |   |   |   |                            [PowerSupply]               |
** >      ~5 >---+   |           |   |   |   |                                                        |
** > [Arduino]   |   |           |   |   |   +---< 1kOhm >---+---< K                                  |
** >             |   |           |   |   +-------------------|---< A                                  |
** >             |   |           +---|---|-------------------+---< R                                  |
** >             |   |               |   |                       [ZenerDiode]                         |
** >             |   |               |   |                                                            |
** >             |   |               |   |   +---------------------< E                                |
** >             +---|---------------|---|---|--------< 330Ohm >---< B                                |
** >                 |               |   |   |                 +---< C                                |
** >                 |   +-----------+   |   |                 |   [NPN]        +-----------------+   |
** >                 |   |               |   |                 |                |                 |   |
** >                 |   |               |   |                 +---< 330Ohm >---+---< 100kOhm >---|---+---< S
** >                 |   |               |   |                                                    +-------< G
** >                 |   |               |   |                                                        +---< D
** >                 |   |               |   +----------------------------------------------< E       |   [P-MOSFET]
** >                 +---|---------------|---|----------------------------------< 1kOhm >---< B       |
** >                     |               |   |                              +---------------< C       |
** >    +---< 18kOhm >---+---< 2kOhm >---+---+                              |               [NPN]     |
** >    |                                |                                  |                         |
** >    |                                |                 +---< 120Ohm >---|---------------< C       |
** >    |                                |                 |                +---< 1kOhm >---< B       |
** >    |                                +-----------------|---+                        +---< E       |
** >    |                                |                 |   |                        |   [PNP]     |
** >    |                                |                 |   +---< S                  |             |
** >    +---< + [Cell] - >---------------+---< 100kOhm >---+-------< G                  |             |
** >    |                                                      +---< D                  |             |
** >    |                                                      |   [N-MOSFET]           |             |
** >    +---< 5Ohm >-------------------------------------------+                        |             |
** >    |                                                      |                        |             |
** >    +---< + [RedLED] - >---< 220Ohm >----------------------+                        |             |
** >    |                                                                               |             |
** >    +-------------------------------------------------------------------------------+-------------+
** - Notes
**   1. If 0.4.0 =< `VERSION` < 0.5.0, this circuit will work.
**   2. The macro `OPERATING_MODE` must be defined by `1`.
**   3. If `SERIAL_PORT` is defined as macro, then it must be expanded as `9600`.
*/

#endif
