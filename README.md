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
* [Connect the solenoid](bildr.org/2011/03/high-power-control-with-arduino-and-tip120/) - Note: Make sure the solenoid is connected to arduinos Digital 2 pin.
* Connect the flow meter. Yellow goes to Arduino Digital 3, red to +5v, black to common ground.
* Connect the temperature sensor. +5v into pin 1. Analog 4 from the arduino goes into pin 2. +5v through a 10K resistor also goes into pin 2. Pin 3 unused. Pin 4 to common ground.
* Ground the arduino
* Put *data.txt* on the sd card that contains (note that {numberofounces} should be an integer that represents how much beer is in the keg at the time of setup and {nameofbeer} is the string name you want to display for the beer). Note2: Do not use the {}'s. Those are there to see, don't use them! See sdcard/data.txt as an example!
    >{numberofounces}\n
    >{nameofbeer}
* Put beer.bmp on the sd card
* Upload the .ino file to the arduino
* If you did everything right, congrats!

## Thanks ##

Thanks go to Jenna Roberts for the image. 
