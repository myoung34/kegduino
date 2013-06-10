kegduino
========

Arduino Kegerator Controller

## What is it ##

This is code to control a kegerator with an arduino touch screen. Simply press the button and beer flows.

## What else is needed ##

* A solenoid (1/4" NPT Electric Solenoid Valve 12-Volt DC - BBTF-CD-12VDC or similar)
* L shaped hose barbs (2 HHO 3/8" NPT x 1/4")
* [1N4004 - 400V 1A Diode](http://www.jameco.com/webapp/wcs/stores/servlet/Product_10001_10001_35991_-1)
* [TIP120 Transistor](http://www.jameco.com/webapp/wcs/stores/servlet/Product_10001_10001_32993_-1)
* [12V 0.5A DC Cord](http://www.jameco.com/webapp/wcs/stores/servlet/Product_10001_10001_114825_-1)
* [12V to 5V regulator](http://www.jameco.com/webapp/wcs/stores/servlet/Product_10001_10001_114825_-1)
* [A Flow Meter](http://www.adafruit.com/products/828)
* [DHT11 Temperature Sensor](http://www.adafruit.com/products/386)
* [Arduino Uno R3](http://www.adafruit.com/products/50)
* [2.8" Resistive TouchScreen](http://www.adafruit.com/products/376)
* A micro sd card. Any card.

## Setup (in a nutshell) ##

A lot of assembly required. [Follow my tutorial for specifics](http://kegduino.org).

* [Set up the 12V to 5v Step Down](http://arduino.cc/en/Main/Standalone)
* Connect the output from the 5V step down to the Vin on the arduino
* [Connect the solenoid](bildr.org/2011/03/high-power-control-with-arduino-and-tip120/) - This basically acts as a gate. Input allows flow through the 12V line to turn the solenoid on/off.
    > Facing: the left prong of the transistor goes through a 10K resistor to the Digital 2 pin on the arduino
    > The middle pin connects straight to the - side of the Solenoid. The middle pin also connects from 1N4004 diode into the +12V line (striped side in the +12V).
    > The right pin on the transistor goes to ground
    > The + side of the solenoid goes directly into +12V line.
* Connect the flow meter. Yellow goes to Arduino Digital 3, red to +5v, black to common ground.
* Connect the temperature sensor. +5v into pin 1. Analog 4 from the arduino goes into pin 2. +5v through a 10K resistor also goes into pin 2. Pin 3 unused. Pin 4 to common ground.
* Ground the arduino
* Put *data.txt* on the sd card that contains (note that {numberofounces} should be an integer that represents how much beer is in the keg at the time of setup and {nameofbeer} is the string name you want to display for the beer). Note2: Do not use the {}'s. Those are there to see, don't use them! See sdcard/data.txt as an example!
    >{numberofounces}\n
    >{nameofbeer}
* Put beer.bmp on the sd card
* Upload [this file](https://github.com/myoung34/kegduino/blob/master/kegduino.ino) to the arduino
* [Get the Adafruit GFX library](https://github.com/adafruit/Adafruit-GFX-Library)
* [Get the Adafruit TFTLCD library](https://github.com/adafruit/TFTLCD-Library)
* [Get the Adafruit TouchScreen library](https://github.com/adafruit/Touch-Screen-Library)
* Change NUMSAMPLES to 3 in libraries/TouchScreen/TouchScreen.cpp! (This makes the touch screen refresh to a median value, meaning that on each refresh it will not try to detect a new press, but continue to realize a hold)
* If you did everything right, congrats!

## Thanks ##

Thanks go to Jenna Roberts for the image. 
