/** THE THIRD STEP ON OUR CAPSTONE PROJECT
  <Abstract>
  * A battery cell charging system using the CC-CV method 
  <Members>
  * Hwanhee Jeon [School of Mechanical Engineering, Chonnam National University]
  * Hakjung Im [School of Mechanical Engineering, Chonnam National University]
  * Kijeong Lim [School of Mechanical Engineering, Chonnam National University]
  <Wiring Diagram>
  *                     #===========#
  *                     # LCD (I2C) #
  *                     # ========= #
  * +-------------------< SCL       #
  * |   +---------------< SDA       #
  * |   |       +-------< VCC       #
  * |   |       |   +---< GND       #
  * |   |       |   |   #===========#
  * |   |       |   |
  * |   |       |   |   #==============#
  * |   |       |   |   # Arduino      #
  * |   |       |   |   # ============ #
  * |   |       |   |   * IOREF # AREF *                    #===============#
  * |   |       |   |   * RESET #  GND >----------------+---< (-) POWER (+) >---+
  * |   |       |   |   * 3.3V  #   13 *                |   #===============#   |
  * |   |       +---|---< 5V    #   12 *                |                       |
  * |   |           +---< GND   #  ~11 *                |       #========#      |
  * |   |   +-----------< GND   #  ~10 *                |   +---< R (#1) >------+
  * |   |   |           * Vin   #   ~9 *                |   |   #========#      |
  * |   |   |           #       #    8 *                |   |                   |
  * |   |   |           #       #    7 *                |   |   #=====#         |
  * |   |   |           #       #   ~6 *                |   |   # NPN #         |
  * |   |   |   +-------< A0    #   ~5 *                |   |   # === #         |
  * |   |   |   |   +---< A1    #    4 *   #========#   |   +---< C   #         |
  * |   |   |   |   |   * A2    #   ~3 >---< R (#2) >---|---|---< B   #         |
  * |   |   |   |   |   * A3    #    2 *   #========#   +---|---< E   #         |   #========#
  * |   +---|---|---|---< A4    #    1 *                |   |   #=====#         |   # MOFSET #
  * +-------|---|---|---< A5    #    0 *                |   |                   |   # ====== #
  *         |   |   |   #==============#                |   +-------------------|---< G      #
  *         |   |   |                                   |                       +---< D      #
  *         |   |   |                                   |                   +-------< S      #
  *         |   |   |                                   |                   |       #========#
  *         |   |   |   #============#                  |                   |
  *         |   |   |   # Sensor     #                  |                   |
  *         |   |   |   # ========== #                  |   #========#      |
  *         |   +---|---< VT  #  Vin >------------------|---< R (#3) >------|------------------+
  *         |       +---< AT  #  GND >------------------+   #========#      |                  |
  *         |           * GND # Vout >-------+          |                   |                  |
  *         +-----------< GND #  GND >---+   |          |   #===========#   |   #==========#   |
  *                     #============#   |   |          +---< + Diode - >---+---< Inductor >---+
  *                                      |   |          |   #===========#       #==========#   |
  *                       #======#       |   |          |                                      |
  *                   +---< Fuse >-------+   |          |                      #===========#   |
  *                   |   #======#           |          +----------------------< Capacitor >---+
  *                   |                      |                                 #===========#
  *                   |   #==============#   |
  *                   +---< (-) Cell (+) >---+
  *                       #==============#
  <Used Parts>
  * [Arduino] Arduino Uno (R3)
  * [Capacitor] 2200[μF] 25[V]
  * [Cell] NCR18650B Li-ion MH12210
  * [Diode] 1N4001
  * [Fuse] Polyswitch 3[A]/30[V]
  * [Inductor] 100[mH]
  * [LCD (I2C)] Arduino LCD 16x2 display module with 4-pins I2C LCD controller
  * [MOSFET] IRFP260N
  * [NPN] C1815
  * [POWER] ???
  * [R (#1)] 10[kΩ]
  * [R (#2)] 10[kΩ]
  * [R (#3)] 5[Ω]
  * [Sensor] MAX471 25[V] 3[A] (HAM6703)
  <Dependencies>
  * [LiquidCrystal_I2C] https://codeload.github.com/johnrickman/LiquidCrystal_I2C/zip/refs/tags/1.1.3
  * [ACS712] https://codeload.github.com/rkoptev/ACS712-arduino/zip/refs/tags/1.0.2
  <References>
  * [1] https://m.blog.naver.com/ysahn2k/222074476103
  * [2] https://codingrun.com/119
***/

// Includes

#include "third.h"

void setup()
{
  // my_printer.printf("Analyzing Battery ...\n");
  // my_controller.setPWM(wanted_voltage = 4.2[V], due_time = 10[ms]);
  // cur = my_sensor.getCurrent();
  // my_controller.lockCurrent(wanted_current = 0[mA], tolerance = 100[mA], due_time = 1000[ms], duration = 5000[ms]);
  // battery_model.ocv = my_sensor.getVoltage();
  // battery_model.computeSOC();
  // soc = battery_model.getSOC();
  // my_printer.printf("soc = %lf\n", soc);
  // battery_model.resistance = (4.2[V] - ocv) / cur
  // mode = CC;
  // my_controller.setPWM(wanted_voltage = ocv + res * CC_cur, due_time = 60000[ms]);
}

void loop()
{
  // vol = my_sensor.getVoltage();
  // cur = my_sensor.getVoltage();
  // my_printer.printf("vol = %lf\n", vol);
  // my_printer.printf("cur = %lf\n", cur);
  // if (mode == CC)
  //   my_controller.lockCurrent(wanted_current = CC_cur, tolerance = 100[mA], due_time = 1000[ms], duration = 1000[ms]);
  //   if (vol >= 4.2[V])
  //     mode = CV;
  // else
  //   my_controller.lockVoltage(wanted_voltage = 4.2[V], tolerance = 0.1[V], due_time = 1000[ms], duration = 1000[ms]);
  //   if (cur < 100[mA])
  //     print("FULL CHARGED");
}
