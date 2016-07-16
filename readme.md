# ks103-i2c-test

## brief

This code is used to control ks103, which is a ultrasonic sensor with a high distance and precision. 

This code is only to test ks103, if you like, you can write your own code to control ks103 following this code under LGPL 3.0 license.

## playground

This code is writend by C languange ~~and run in raspberrypi 3~~, maybe I only had raspberrypi in my hands at that time, and now i use it in another one linux cpu and it works well.

i2c mode is used to connect ks103 with raspberrypi. You can connect the only i2c pins (SDA and SCK) to ks103, several ks103 can connected with a parallel mode and distinguished each other at different addresses.

Be attentain to your I2C pins and I2C device name(always in /dev/) in linux system.

## about me

You can connect with me by e-mail: dicksonliuming@gmail.com or follow my Github: PWESiberiaBear
