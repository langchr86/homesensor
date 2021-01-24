Software
========



Getting started
---------------

* vscode
* credentials
* build & deploy



Debugging
---------

* #define LOG_DISABLE
* logger.SetMaxLevel(LOG_INFO);



Known issues / future features
------------------------------

### Sensor

* split sensor read out and data sending
* linear calibration for individual sensor values
* sensor power save modes
* long time stability
* do not use heap (String etc.)
* handling of secrets
* merge repos
* ansible for real hardware and vargant
* readmes (how to, images, description of hardware)
* 3D printed case
* Battery life calculator: http://www.of-things.de/battery-life-calculator.php
* SHTC3 Breakout-board without any level converters



License
-------

See top level [README.md](../README.md).

The integrated and improved libraries are the following:

* [scd30 / SoftWire](https://github.com/paulvha/scd30) -> MIT
* [sht30](https://github.com/sparkfun/SparkFun_SHTC3_Arduino_Library) -> GPL v3
