Backend
=======

Home Assistant is used to collect the measured data of the sensors.

The easiest and most future prove way is to follow one of the tutorials provided by Home Assistant itself:
[Home Assistant - Installation](https://www.home-assistant.io/installation/).

Because the sensor is using MQTT as data protocol
we need to install the MQTT Integration in Home Assistant: [](https://www.home-assistant.io/integrations/mqtt/).
If Home Assistant was installed with HA-OS we also need to install the MQTT Broker
as Home Assistant add-on: [Mosquitto broker](https://github.com/home-assistant/addons/blob/master/mosquitto/DOCS.md)


Using Ansible
-------------

All information below can be used to setup docker and HA by using Ansible.
But this is only recommended for advanced users!

The ansible playbook can install the needed docker containers on any linux system.
In addition a `Vagrantfile` is provided that setups a virtualbox VM for development purposes.



Setup on existing server
------------------------

First we have to ensure that ansible is installed.

~~~~~~
apt install ansible

# or
apt install software-properties-common
apt-add-repository ppa:ansible/ansible
apt update
apt install git ansible

# or
apt install python3-pip
pip3 install ansible
~~~~~~

For the real installation you need a linux server that has a running `docker-compose` installation.
You can either use the existing role to install `dockerce` / `dockerio`
or install with your package manager e.g. `apt install docker.io docker-compose`
or use some other way e.g. [github.com/geerlingguy/ansible-role-docker](https://github.com/geerlingguy/ansible-role-docker).

You then can clone this repo on your server and run the script `run_local.sh`.
Or you can include the `homeassistant` role in your own ansible playbook for your server.

Now you can configure Home Assistant.



Setup VM for development purposes
---------------------------------

The vagrant configuration is ready to be used to setup a complete VM with Home Assistant.
In addition all ports needed are already forwarded and can be accessed on the local machine.

Install vagrant and virtualbox and start the VM creation with:

~~~~~~
cd homesensor/backend
vagrant up
~~~~~~

Because the sensors are external devices the MQTT port `1883` needs to be reachable from outside the machine.
Add a corresponding incoming rule in the firewall.

The Web-GUI is usually accessed in a browser on the local machine.
If this needs to be accessible from another machine the port `8123` needs to be allowed in the firewall.

If you need later to manipulate the vagrant VM the following commands are useful:

~~~~~~
vagrant halt
vagrant destroy -f
vagrant provision
vagrant up --provision
~~~~~~

Now you can configure Home Assistant.



Configure Home Assistant
------------------------

You can now configure Home Assistant
after installing the `homeassistant` and `eclipse-mosquitto` (MQTT) container on your server host.

* Login to: `<YOUR_HOMEASSISTANT_IP>:8123` with your web browser.
* Create a user account.
* Name your Home Assistant installation. E.g: `homesensor-backend`. And define your location.
* At the next page your asked to select integration services to install. Search and install the `MQTT` integration.
  Use `localhost` for server IP and no user and no password. The port should be kept at the default `1883`.
  Enable the flag for the search function.
* Finish the configuration and re-login at the same URL again with your defined user and password.

> Note: We do not define any MQTT credentials to work in anonymous mode.
> You can define credentials but then also need to configure those in the sensors used.

Now you can flash your sensor boards.
They should automatically appear as device/entity in your Home Assistant lovelace board.



Additional Home Assistant configuration
---------------------------------------

As defined in the `docker-compose.yml` the Home Assistant configuration is mounted to the host file system.
To configure Home Assistant manipulate the automatically created `configuration.yaml`.
E.g. by changing the history duration from 10 (default) to 28 days:

~~~~~~
recorder:
  purge_keep_days: 28
~~~~~~
