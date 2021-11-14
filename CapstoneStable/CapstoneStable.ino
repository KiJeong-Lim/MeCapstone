#include "header.h"

class BMS {
public:
  void initialize();
  void progressing();
} myBMS;

SerialPrinter const consoleLog = { .prefix = "    [log] " };
SerialPrinter const alert = { .prefix = "[Warning] " };

void setup() {
#if defined(SERIAL_PORT)
  Serial.begin(SERIAL_PORT);
#endif
  myBMS.initialize();
}

void loop() {
#if defined(SERIAL_PORT)
  Serial.println("=========");
#endif
  myBMS.progressing();
}

void BMS::initialize()
{
  Wire.begin();
}

void BMS::progressing()
{
}
