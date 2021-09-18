# My Capstone Project

## first

- Measuring the voltage of a given battery and printing it.

- `arduino.json`:
```json
{
    "sketch": "first/first.ino",
    "board": "arduino:avr:uno",
    "port": "COM3",
    "output": "first/build"
}
```

- `c_cpp_properties.json`:
```json
{
    "version": 4,
    "configurations": [
        {
            "name": "Arduino",
            "compilerPath": "C:\\Users\\user\\AppData\\Local\\Arduino15\\packages\\arduino\\tools\\avr-gcc\\7.3.0-atmel3.6.1-arduino5\\bin\\avr-g++",
            "compilerArgs": [
                "-std=gnu++11",
                "-fpermissive",
                "-fno-exceptions",
                "-ffunction-sections",
                "-fdata-sections",
                "-fno-threadsafe-statics",
                "-Wno-error=narrowing"
            ],
            "intelliSenseMode": "gcc-x64",
            "includePath": [
                "C:\\Users\\user\\AppData\\Local\\Arduino15\\packages\\arduino\\hardware\\avr\\1.8.2\\cores\\arduino",
                "C:\\Users\\user\\AppData\\Local\\Arduino15\\packages\\arduino\\hardware\\avr\\1.8.2\\variants\\standard",
                "C:\\Users\\user\\AppData\\Local\\Arduino15\\packages\\arduino\\hardware\\avr\\1.8.2\\libraries\\Wire\\src",
                "C:\\Program Files (x86)\\Arduino\\libraries\\LiquidCrystal_I2C"
            ],
            "forcedInclude": [
                "C:\\Users\\user\\AppData\\Local\\Arduino15\\packages\\arduino\\hardware\\avr\\1.8.2\\cores\\arduino\\Arduino.h"
            ],
            "cStandard": "c11",
            "cppStandard": "c++11",
            "defines": [
                "F_CPU=16000000L",
                "ARDUINO=10816",
                "ARDUINO_AVR_UNO",
                "ARDUINO_ARCH_AVR",
                "USBCON"
            ]
        }
    ]
}
```
