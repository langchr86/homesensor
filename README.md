We use anonymous mode in mqtt broker


HA configuration.yml:

~~~~~~
mqtt:
  discovery: true
  discovery_prefix: homeassistant
~~~~~~

Not needed because activated in integration setup of mqtt


TODO:

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


Building steps

* remove LED on SHTC3
* enable voltage read (2x0ohm)
* use direct 3.3V in

https://randomnerdtutorials.com/esp32-deep-sleep-arduino-ide-wake-up-sources/