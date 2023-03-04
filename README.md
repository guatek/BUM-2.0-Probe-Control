# BUM 2.0 Console-Control
![version](https://img.shields.io/badge/version-1.0.0-blue.svg) ![license](https://img.shields.io/badge/license-MIT-blue.svg) 

Console-Control is an Arduino (C++) firmware for controlling the Guatek BUM 2.0 Console system built in 2022, revA. The system is designed around a [LowPowerLabs](https://lowpowerlab.com/) [Moteino M0](https://lowpowerlab.com/category/moteino/moteinom0/), which is a compact and pin-rich breakout board for the Atmel SAMD21.

## Requirements

- [Console-Control RevA](https://github.com/guatek/KiCAD-BUM-2.0/tree/master/Console-Controller) hardware
- [Arduino](https://www.arduino.cc/en/software)
- [LowPowerLab SAMD Release](https://lowpowerlab.com/2020/03/04/moteino-samd-1-5-0-release/)

## Highly Recommended

- [VS Code](https://code.visualstudio.com/)
- [PlatformIO](https://platformio.org/) with [Atmel SAM Platform](https://docs.platformio.org/en/latest/platforms/atmelsam.html) and [Moteino M0 Environment](https://docs.platformio.org/en/latest/boards/atmelsam/moteino_zero.html?highlight=moteino_zero)

## System Archetecture

Details of the archetecture will be described soon. For the time being, the system does the following:

### Setup 

1. Start the debug serial port (USB)
2. Setup pins modes
3. Initialize DS3231 real time clock
4. Start timers
5. Initialize flash
6. Start sensors
7. Add all of the config parameters to the SystemConfig object
8. Configure watchdog
9. Start all of the remaining serial ports
10. Load saved SystemConfig values from flash
11. Load saved Scheduler from flash
12. Setup timers and ISRs for camera and flash trigger signals 

### Loop

1. System Update (read sensors, process data)
2. Check for user input
3. Check input voltage
4. Check environment sensors (temp, humidity, pressure)
5. Check Scheduler events
6. Check status of camera power events
7. Sleep
8. Flash status LED
9. GoTo: 1


## Reporting Issues
We use GitHub Issues as the official bug tracker

## Technical Support or Questions

If you need support beyond github please use our [Contact Page](http://www.guatek.com/contact/) or reach out to us directly via email or slack.

## Licensing

- Copyright 2022 Guatek (http://www.guatek.com)
- Licensed under MIT (https://github.com/guatek/Dual-Mag-Control/blob/master/LICENSE.md)

