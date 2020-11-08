Backend
=======

Home assistant is used to collect the measured data of the sensors.
The ansible playbook can install the needed docker containers on any linux system.

In addition a `Vagrantfile` is provided that setups a virtualbox VM for development purposes.



Setup on existing server
------------------------

For the real installation you need a linux server that has a running `docker-compose` installation.
You can either use the existing role to install `dockerce` / `dockerio`
or install with your package manager e.g. `apt install docker.io docker-compose`
or use some other way e.g. [github.com/geerlingguy/ansible-role-docker](https://github.com/geerlingguy/ansible-role-docker).

You then can clone this repo on your server and run the script `run_local.sh`.
Or you can include the `homeassistant` role in your own ansible playbook for your server.

Now you can configure home assistant.



Setup VM for development purposes
---------------------------------

The vagrant configuration is ready to be used to setup a complete VM with home assistant.
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

Now you can configure home assistant.



Configure home assistant
------------------------

You can now configure home assistant
after installing the `homeassistant` and `eclipse-mosquitto` (MQTT) container on your server host.

* Login to: `<YOUR_HOMEASSISTANT_IP>:8123` with your web browser.
* Create a user account.
* Name your home assistant installation. E.g: `homesensor-backend`. And define your location.
* At the next page your asked to select integration services to install. Search and install the `MQTT` integration.
  Use `localhost` for server IP and no user and no password. The port should be kept at the default `1883`.
  Enable the flag for the search function.
* Finish the configuration and re-login at the same URL again with your defined user and password.

> Note: We do not define any MQTT credentials to work in anonymous mode.
> You can define credentials but then also need to configure those in the sensors used.

Now you can flash your sensor boards.
They should automatically appear as device/entity in your home assistant lovelace board.
