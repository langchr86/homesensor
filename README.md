We use anonymous mode in mqtt broker


HA configuration.yml:

~~~~~~
mqtt:
  discovery: true
  discovery_prefix: homeassistant
~~~~~~

Not needed because activated in integration setup of mqtt


TODO:

* allow to disable serial debugging?
* battery level notification
* deep sleep
* real sensor integration
* linear calibration
* sensor power save modes
* do not use heap (String etc.)
* handling of secrets
* merge repos
* ansible for real hardware and vargant
* readmes (how to, images, description of hardware)
* 3D printed case
