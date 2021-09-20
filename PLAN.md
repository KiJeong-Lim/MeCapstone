# The charging system for a battery cell

```c
/**
  <Wiring Diagram>
  *                                        #=====#   #==============#                                            
  * +--------------------------------------< SCL #   # Source (DC)  #                                          #==============#
  * |   +----------------------------------< SDA #   # ============ #       +--------------------------+       # IRF4905PBF   #
  * |   |   +------------------------------< VCC #   # V(t) = V_src #       |                          |       # ============ #
  * |   |   |   +--------------------------< GND #   # I(t) = I_src #       |   #==============#       |       # P-channel    #
  * |   |   |   |                          # === #   #          (+) >-------+---< R = 1 [KOhm] >---+   |       # power MOSFET #
  * |   |   |   |                          # I2C #   #          (-) >---+       #==============#   |   +-------< S     55 [V] #
  * |   |   |   |   #==================#   #=====#   #==============#   |                          |       +---< D     74 [A] #
  * |   |   |   |   # Arduino Uno (R3) #                                |      #===============#   +-------|---< G   2 [mOhm] #
  * |   |   |   |   # ================ #                                |      # SS8050DTA     #   |       |   #==============#
  * |   |   |   |   * IOREF       AREF *                                |      # ============= #   |       +------------------+
  * |   |   |   |   * RESET        GND >--------------------------------+      # NPN Bipolar C >---+   #==============#       |
  * |   |   |   |   * 3.3V          13 *                                |      # 1.5 [A]     B >-------< R = 1 [KOhm] >---+   |
  * |   |   +---|---< 5V            12 *                                |      # 25 [V]      E >---+   #==============#   |   |
  * |   |       +---< GND          ~11 *                                |      #===============#   |                      |   |
  * |   |   +-------< GND          ~10 *                                |                          |                      |   |
  * |   |   |       * Vin           ~9 *                                +--------------------------+                      |   |
  * |   |   |       #                8 *                                |                                                 |   |
  * |   |   |       #                7 *                                |                                                 |   |
  * |   |   |       #               ~6 *                                |                                                 |   |
  * |   |   |   +---< A0            ~5 *                                |                                                 |   |
  * |   |   |   |   * A1             4 *                                |                                                 |   |
  * |   |   |   |   * A2            ~3 >--------------------------------|-------------------------------------------------+   |
  * |   |   |   |   * A3             2 *                                |                                                     |
  * |   +---|---|---< A4             1 *                                |   #=========#                                       |
  * +-------|---|---< A5             0 *                                |   # 1N5819  #                                       |
  *         |   |   #==================#                                |   # ======= #                                       |
  *         |   |                          #====================#       +---< (+) (-) >---------------------------------------+
  *         +---|--------------------------< -                  #       |   # diode   #                                       |
  *             |                          * +                  #       |   #=========#                                       |
  *             +--------------------------< S                  #       |                      #==========================#   |
  *                                +-------< VCC                #       |                      # 150 [muH] Power Inductor >---+
  *                                |   +---< GND                #       |                      # 2 [A] Idc                #
  *                                |   |   # V_out = V_in * 0.2 #       |                      # 106 [mOhm] Rdc           >---+
  * #========#                     |   |   # ================== #       |                      #==========================#   |
  * #    (-) >---------------------|---+   # Voltage Divider    #       |                                                     |
  * #        #                     |   |   #====================#       |   #========================#                        |
  * #    (+) >---------------------+   |                                |   # Panasonic              #                        |
  * # ====== #                     |   |                                +---< 100 [muF] 16 [V]       >------------------------+
  * # Target #                     |   |                                |   # Electrolytic Capacitor #                        |
  * #========#                     |   +--------------------------------+   #========================#                        |
  *                                |                                                                                          |
  *                                +------------------------------------------------------------------------------------------+
  <References>
  * [1] https://samiralavi.github.io/blog/buck_coverter/buck_converter_arduino/
***/
```