# My charging system for a battery cell

```c
/**
  <Wiring Diagram>
  *                 #===========#
  *                 # LCD (I2C) #
  *                 # ========= #
  * +---------------< SCL       #
  * |   +-----------< SDA       #
  * |   |   +-------< VCC       #
  * |   |   |   +---< GND       #
  * |   |   |   |   #===========#
  * |   |   |   |
  * |   |   |   |   #==============#
  * |   |   |   |   # Arduino      #
  * |   |   |   |   # ============ #
  * |   |   |   |   * IOREF   AREF *                    #============#
  * |   |   |   |   * RESET    GND >----------------+---< - Source + >---+
  * |   |   |   |   * 3.3V      13 *                |   #============#   |
  * |   |   +---|---< 5V        12 *                |                    |
  * |   |       +---< GND      ~11 *                |       #========#   |
  * |   |   +-------< GND      ~10 *                |   +---< R (#1) >---+
  * |   |   |       * Vin       ~9 *                |   |   #========#   |
  * |   |   |       #            8 *                |   |                |      
  * |   |   |       #            7 *                |   |   #=====#      |
  * |   |   |       #           ~6 *                |   |   # NPN #      |
  * |   |   |   +---< A0        ~5 *                |   |   # === #      |
  * |   |   |   |   * A1         4 *   #========#   |   +---< C   #      |
  * |   |   |   |   * A2        ~3 >---< R (#2) >---|---|---< B   #      |
  * |   |   |   |   * A3         2 *   #========#   +---|---< E   #      |
  * |   +---|---|---< A4         1 *                |   |   #=====#      |
  * +-------|---|---< A5         0 *                |   |                |
  *         |   |   #==============#                |   |   #========#   |
  *         |   |                                   |   |   # MOSFET #   |
  *         |   |              #================#   |   |   # ====== #   |
  *         |   |              # Voltage Sensor #   |   |   #      S >---+
  *         |   |              # ============== #   |   |   #      D >-------+
  *         |   +--------------< S              #   |   |   #      G >---+   |
  *         |                  * +              #   |   |   #========#   |   |
  *         +------------------< -              #   |   |                |   |
  * +--------------------------< VCC            #   |   +----------------+   |
  * |                      +---< GND            #   |                        |
  * |                      |   #================#   |                        |
  * |                      |                        |        #===========#   |
  * |                      +------------------------+--------< + Diode - >---+
  * |                      |                                 #===========#   |
  * |                      |                                                 |
  * |   #==============#   |   #===========#            #================#   |
  * +---< (+) Cell (-) >---+---< Capacitor >---+--------< Power Inductor >---+
  * |   #==============#       #===========#   |        #================#
  * |                                          |
  * +------------------------------------------+
  <Used Parts>
  * [Arduino] Arduino Uno (R3)
  * [Capacitor] Panasonic 100[μF] 16[V] Electrolytic Capacitor
  * [Cell] NCR18650B Li-ion MH12210
  * [Diode] 1N5819 Schottky Diode
  * [LCD (I2C)] Arduino LCD 16x2 display module with 4-pins I2C LCD controller
  * [MOFSET] IRF4905PBF P-channel Power MOFSET
  * [NPN] SS8050DTA Bipolar NPN Transistor
  * [Power Inductor] 150[μH] Power Inductor 2[A] Idc 106[mΩ] Rdc
  * [R (#1)] 1[kΩ]
  * [R (#2)] 1[kΩ]
  * [Source] ???
  * [Voltage Sensor] Arduino DC 0-25V voltage sensor module
  <References>
  * [1] https://samiralavi.github.io/blog/buck_coverter/buck_converter_arduino/
***/
```
