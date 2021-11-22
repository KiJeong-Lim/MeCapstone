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
** 2. repository  = https://github.com/marcoschwartz/LiquidCrystal_I2C.git
** 3. release     = https://github.com/johnrickman/LiquidCrystal_I2C/releases/tag/1.1.3
** 4. interface   = "LiquidCrystal_I2C.h"
*/

/* History
** [2021-11-14]
** 1. Files added `CapstoneStable/version.h`,
**                `CapstoneStable/header.h`,
**                `CapstoneStable/formatter.hpp`,
**                `CapstoneStable/printer.ino`,
**                `CapstoneStable/utility.ino`,
**                `CapstoneStable/CapstoneStable.ino`.
** 2. The class `BufferWithFormat` replaced with `Formatter<LCD_SECTION_LEN>`.
** 3. `VERSION` updated to `0.10`.
** [2021-11-15]
** 1. `Wire.begin();` inserted in the method `BMS::initialize`.
**    - This is a very important change.
**    - If `Wire.begin();` is missed,
**      the function `openLcdI2c` will not work.
** 2. Files moved `CapstoneStable/header.h` -> `CapstoneStable/header.hpp`.
** 3. The method `LcdPrettyPrinter::~LcdPrettyPrinter` corrected.
**    - Code changed,
**      old-version = `mybuf[c][LCD_WIDTH - 1] = '\0';`,
**      new-version = `mybuf[c][LCD_WIDTH] = '\0';`.
** 4. Files added `CapstoneStable/ocv.ino`.
**    - For deriving OCV by looking-up the soc-ocv table.
** 5. `VERSION` updated to `0.20`.
** [2021-11-16]
** 1. The class `SerialPrinter` corrected.
** 2. The function `Formatter::putDouble` improved.
** 3. The class `Timer` corrected.
** 4. Files moved `CapstoneStable/ocv.ino` -> `CapstoneStable/soc.ino`.
** 5. `VERSION` updated to `0.30`.
** [2021-11-17]
** 1. Files moved `CapstoneStable/version.h`          -> `capstone/version.h`,
**                `CapstoneStable/header.hpp`         -> `capstone/header.h`,
**                `CapstoneStable/formatter.hpp`      -> `capstone/formatter.hpp`,
**                `CapstoneStable/printer.ino`        -> `capstone/printer.ino`,
**                `CapstoneStable/utility.ino`        -> `capstone/utility.ino`,
**                `CapstoneStable/soc.ino`            -> `capstone/soc.ino`,
**                `CapstoneStable/CapstoneStable.ino` -> `capstone/capstone.ino`.
** 2. The schematic [Circuit-A] corrected.
** 3. The class `SerialPrinter` improved.
** 4. The class `Formatter` improved.
** 5. `VERSION` updated to `0.40`.
** [2021-11-18]
** 1. Files added `capstone/pinhandlers.ino`.
** 2. Files moved `capstone/utility.ino`   -> `capstone/utilities.ino`,
**                `capstone/formatter.hpp` -> `capstone/formatters.hpp`,
**                `capstone/printer.ino`   -> `capstone/printers.ino`,
**                `capstone/soc.ino`       -> `capstone/data.ino`,
**                `capstone/header.h`      -> `capstone/capstone.h`.
** 3. The method `BMS::checkSocOf` improved.
** [2021-11-19]
** 1. Files eliminated `capstone/formatters.hpp`.
** 2. The method `BMS::getCalibrationOfIin` introduced.
** 3. Files moved `capstone/capstone.h`      -> `capstone/capstone.hpp`,
**                `capstone/utilities.ino`   -> `capstone/utilities.cpp`,
**                `capstone/printers.ino`    -> `capstone/printers.cpp`,
**                `capstone/pinhandlers.ino` -> `capstone/pinhandlers.cpp`,
**                `capstone/data.ino`        -> `capstone/data.cpp`.
** 4. The method `BMS::updateQs` corrected.
**    - Code changed,
**      old-version = `Qs[i] += (Iin / number_of_cell_being_charged) * (millis() - Qs_lastUpdatedTime) / 3600;`,
**      new-version = `Qs[i] += Iin * (millis() - Qs_lastUpdatedTime) / 3600;`.
** 5. The method `BMS::initialize` corrected.
** 6. Dependencies information added.
** [2021-11-20]
** 1. Used schematics uploaded.
** 2. Product name of the DC power supply corrected.
** 3. The class `AscMap` renamed into `AscList`.
**    The method `AscMap::get_x_from_y`         renamed into `AscList::get_x_by_y`,
**               `AscMap::get_x_from_parameter` renamed into `AscList::get_x_by_parameter`.
**    The method `AscList::get_y_by_x` introduced.
** [2021-11-21]
** 1. The schematic [Circuit-B] corrected.
** 2. The class `BMS` improved.
**    -- Fields added `BMS::every_cell_attatched`,
**                    `BMS::is_operating_now`.
**    -- Methods added `BMS::showBmsInfo`,
**                     `BMS::checkCellsAttatched`.
**    -- Methods corrected `BMS::initialize`,
**                         `BMS::progress`,
**                         `BMS::updateQs`.
** 3. Test results
**    [Circuit-B]
**    (1) Not charging -> Okay
**        -- Serial port has been connected, SERIAL_PORT := 9600.
**    (2) Charging -> Okay!!!
**        -- Turn on the power supply before running Arduino.
**        -- Serial port has been connected.
**    (3) Blinking -> Okay!!!
**        -- Turn on the power supply before running Arduino.
**        -- Serial port has been connected.
** [2021-11-22]
** 1. The class `Map2d` added.
** 2. `VERSION` updated to `1.00`.
** 3. File moved `capstone/capstone.ino` -> `capstone/capstone-stable.ino`.
*/

/* Circuit Archive
** [Circuit-A]
** - List of operating dates
**   [#1] 2021.11.09
** - Parts list   Product name                     Quantity
**   [Arduino]    Arduino Uno Rev3                 x 1
**   [Cell]       Panasonic 18650 Li-ion NCR18650G x 1
**   [Resistor]   1kOhm                            x 1
**                2kOhm                            x 1
**                18kOhm                           x 1
**   [ZenerDiode] TL431BVLPRAGOSCT-ND              x 1
** - Schematic
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
** - List of operating dates
**   [#1] 2021.11.16
**   [#2] 2021.11.21
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
**   [Resistor]     5Ohm                                   x 1
**                  120Ohm                                 x 1
**                  220Ohm                                 x 1
**                  330Ohm                                 x 2
**                  1kOhm                                  x 3
**                  2kOhm                                  x 1
**                  18kOhm                                 x 1
**                  100kOhm                                x 2
**   [WCR5OhmJ5W]   Wire Wound Cement Resistor EPX7RBL3    x 1
**   [ZenerDiode]   TL431BVLPRAGOSCT-ND                    x 1
** - Schematic
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
** >                 |   |               |   |                 +---< 330Ohm >---+---< 100kOhm >---|---+-------< S
** >                 |   |               |   |                                                    |       +---< G
** >                 |   |               |   |                                                    +-------|---< D
** >                 |   |               |   +----------------------------------------------< E           |   [P-MOSFET]
** >                 +---|---------------|---|----------------------------------< 1kOhm >---< B           |
** >                     |               |   |                              +---------------< C           |
** >    +---< 18kOhm >---+---< 2kOhm >---+---+                              |               [NPN]         |
** >    |                                |                                  |                             |
** >    |                                |                 +---< 120Ohm >---|---------------< C           |
** >    |                                |                 |                +---< 1kOhm >---< B           |
** >    |                                +-----------------|---+                        +---< E           |
** >    |                                |                 |   |                        |   [PNP]         |
** >    |                                |                 |   +---< S                  |                 |
** >    +---< + [Cell] - >---------------+---< 100kOhm >---+-------< G                  |                 |
** >    |                                                      +---< D                  |                 |
** >    |                                                      |   [N-MOSFET]           |                 |
** >    +---< 5Ohm >-------------------------------------------+                        |                 |
** >    |                                                      |                        |                 |
** >    +---< + [RedLED] - >---< 220Ohm >----------------------+                        |                 |
** >    |                                                                               |                 |
** >    +-------------------------------------------------------------------------------+-----------------+
** - Notes
**   1. If 0.4.0 =< `VERSION` < 0.5.0, this circuit will work.
**   2. The macro `OPERATING_MODE` must be defined by `1`.
**   3. The macro `SERIAL_PORT` must be expanded to `9600` if it defined.
*/

#endif
