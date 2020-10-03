We use anonymous mode in mqtt broker


HA configuration.yml:

~~~~~~
mqtt:
  discovery: true
  discovery_prefix: homeassistant
~~~~~~

Not needed because activated in integration setup of mqtt


TODO:

* replace MQTT library
* battery level notification
* deep sleep
* real sensor integration
* linear calibration
* sensor power save modes
