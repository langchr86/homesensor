HA configuration.yml:

~~~~~~
mqtt:
  discovery: true
  discovery_prefix: homeassistant

sensor:
  - platform: mqtt
    state_topic: "home/balkon/temperature"
    name: "Balkon Temperatur"
    unique_id: "balkon_temp"
    device_class: "temperature"
    expire_after: 120
    unit_of_measurement: "°C"
~~~~~~
