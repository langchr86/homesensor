We use anonymous mode in mqtt broker


HA configuration.yml:

~~~~~~
mqtt:
  discovery: true
  discovery_prefix: homeassistant
~~~~~~

Not needed because activated in integration setup of mqtt


TODO:

* add sensor base class
* allow to disable serial debugging?
* battery level notification
* deep sleep
* boot count
* count missed sensor reads and go into deep sleep if not working
* remove led of SHTC3
* real sensor integration
* linear calibration
* sensor power save modes
* long time stability
* do not use heap (String etc.)
* handling of secrets
* merge repos
* ansible for real hardware and vargant
* readmes (how to, images, description of hardware)
* 3D printed case


Building steps

* remove LED on SHTC3
* enable voltage read

https://randomnerdtutorials.com/esp32-deep-sleep-arduino-ide-wake-up-sources/