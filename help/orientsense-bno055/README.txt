Some of the code in this repository references a bno055 orientation sensor. This low-cost sensor is both inexpensive and reasonably easy to build (perhaps some soldering required). Total cost should be around $60.

=== Parts list ===

Arudino - Although any that has SDA and SCL pins (often labeled A4 and A5 on Arduinos), I used the Arduino Mini Pro (5V, 16Mhz, ATmetga328 variety):
~ $2 on ebay - https://www.arduino.cc/en/Main/ArduinoBoardProMini

Many Arduinos (such as the Arduino Mini Pro) requires that you provide a USB->serial adapter. You'll also need the right USB cable (which you may already have!):
~$15 - Adafruit FTDI Friend - https://www.adafruit.com/product/284
~$4 - USB cable (A male to MiniB) - https://www.adafruit.com/products/260

Adafruit 9DOF Absolute Orientation IMU Fusion Breakout - BNO055. Unlike many other low-cost orientation sensors, this one fuses the accelerometers, gyroscopes, and magnetometers together and makes it easy to simply output the orientation that you want.
~$35 - https://www.adafruit.com/products/2472


=== Wiring: ===

You need to connect 4 wires between the Arduino and the orientation sensor. I used a breadboard and jumper wires to connect GND and 5V pins. I soldered a pair of female jumpers on the A5 (SCL) and A4 (SDA) pins on the Arduino and also used jumper wires to connect those too.

Aruidno GND pin
to
BNO055 GND pin

Arduino VCC
to
BNO055 Vin  (the Adafruit board includes a voltage regulator which accepts 3.3-5V

Arduino A5 (SCL)
to
BNO055 SCL

Arduino A4 (SDA)
to
BNO055 SDA

=== Programming the arduino ===

Use the orient.ino program included in this repository to program the arduino. You will also need these two libraries:
https://github.com/adafruit/Adafruit_Sensor
https://github.com/adafruit/Adafruit_BNO055

=== Testing ===

If you set the "binary" variable in orient.ino to 0, you should be able to program the Arduino, and then use the "Serial Monitor" on the arduino (set to 115200 buad) to monitor the live output of the sensor. The OpenGL examples program, however, assumes that your sensor is using a more compact binary output. So, you must have "binary" set to 1 when using the orientation sensor with the OpenGL code.


