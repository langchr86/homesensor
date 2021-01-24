Device production
=================


Bill of Material
----------------

* DFRobot Firebeetle ESP32: [1738-1302-ND](https://www.digikey.ch/product-detail/de/dfrobot/DFR0478/1738-1302-ND/7398878)
* Adafruit breakout board: Sensirion SHTC3: [1528-4636-ND](https://www.digikey.ch/product-detail/de/adafruit-industries-llc/4636/1528-4636-ND/12504001)
* Sensirion SCD30: [1649-1098-ND](https://www.digikey.ch/product-detail/de/sensirion-ag/SCD30/1649-1098-ND/8445334)
* 18650 Battery holder: [BH-18650-PC-ND](https://www.digikey.ch/product-detail/de/mpd-memory-protection-devices/BH-18650-PC/BH-18650-PC-ND/3029216)
* JST 2pin power connector: [1528-1126-ND](https://www.digikey.ch/product-detail/de/adafruit-industries-llc/261/1528-1126-ND/5353586)



Firebeetle preparation
----------------------

First we have to prepare the CPU board.
The main thing we have to do is to enable the input voltage measurement.
For this we have to short two prepared jumbers or place 0Ohm resistors.
Those are the `R10` and `R11` on the board.

<p float="left">
<img src="images/esp32_01_no_adc_bridge.jpg" width="30%">
<img src="images/esp32_02_adc_bridge.jpg" width="30%">
</p>



Indoor sensor
-------------

First correctly wire the sensor to the CPU board.

<p float="left">
<img src="images/scd30_01_esp_wires.jpg" width="30%">
<img src="images/scd30_02_sensor_wires.jpg" width="30%">
<img src="images/scd30_03_complete_wires.jpg" width="30%">
</p>

We use the box in which the ESP32 was delivered as a case.
This has a good size and is good enough for indoor usage.
The electronic parts are assembled to an additional cardboard.

<p float="left">
<img src="images/scd30_04_cardboard_empty.jpg" width="30%">
<img src="images/scd30_05_cardboard_folded.jpg" width="30%">
<img src="images/scd30_06_cardboard_case_test.jpg" width="30%">
</p>

![Case plan](scd30_case.png)

The box need to have air holes on both lids to enable air flow.

<p float="left">
<img src="images/scd30_07_case_marks.jpg" width="30%">
<img src="images/scd30_08_case_cuts.jpg" width="30%">
<img src="images/scd30_09_case_hole.jpg" width="30%">
</p>

The eletronic parts are attached to the cardboard with adhesive tape.

<p float="left">
<img src="images/scd30_10_adhesive.jpg" width="30%">
<img src="images/scd30_11_battery.jpg" width="30%">
<img src="images/scd30_12_placed.jpg" width="30%">
</p>

The cardboard can then be inserted in the box.

<p float="left">
<img src="images/scd30_13_slide_in.jpg" width="30%">
<img src="images/scd30_14_fit_in_case.jpg" width="30%">
<img src="images/scd30_15_case_closed.jpg" width="30%">
</p>



Outdoor sensor
--------------

If you want to have as less power consumption
and most importantly no always glowing LED
you have to remove it from the Adafruit board.

<img src="images/shtc3_led.jpg" width="30%">

**ToDo: cabling/images**

We use a 3D printed case for the outdoor sensor
because of the weather influence.

<p float="left">
<img src="case_outdoor/images/Entwurf-1.1a.png" width="40%">
<img src="case_outdoor/images/Entwurf-1.1c.png" width="15%">
<img src="case_outdoor/images/Entwurf-1.1e.png" width="40%">
</p>

**Not yet printed/tested**

**Images are no longer up-to-date with the real model data. Will be updated after real printing test.**

The corresponding files are located under `case_outdoor/`.
To ensure longevity of the screw holes we use special plastic screws.
More information for those
[SCHRIEVER STS Senkkopf Schrauben T KN 1041 Stahl znb 1227](https://www.sfs.ch/de/Befestigungstechnik/Blech-%2C-Bohr-%2C-Gewindefurchschrauben/Gewindefurchende-und-gewindeschneidende-Schrauben/Gewindefurchschrauben-f%C3%BCr-Kunststoff/SCHRIEVER-STS-Senkkopf-Schrauben-T-KN-1041-Stahl-znb/p/S010603_0880?sourceCategory=su_m_10-15-030-10&q=%3Arelevance%3Asu_c_10-15-030-10_su_21142_facet%3ASenkkopf)
can be found in the datasheet in the top level `documents` folder.

~~~~~~
d1 = 2.2
d2 = 3.8
t = 6
Antrieb = T6
Löcher = 1.85
~~~~~~
