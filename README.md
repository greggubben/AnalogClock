
Arduino Analog Clock

This program will use analog meters, like Ampere or Volt meters, to display hours, minutes, and seconds.

First shared on Know How and Coding 101 on the TWIT.tv network.

Check out the following episodes for the coding:
- http://twit.tv/show/coding-101/54
- http://twit.tv/show/coding-101/55
- http://twit.tv/show/coding-101/56
- http://twit.tv/show/coding-101/57
Check out the following episodes for the hardware:
- http://twit.tv/show/know-how/132
- http://twit.tv/show/know-how/133
- http://twit.tv/show/know-how/134
- http://twit.tv/show/know-how/135

Thanks to Fr. Robert Ballecer, SJ and Mark Smith.

Additional Libraries that need to be downloaded and installed in your libraries directory:

Time
- http://www.pjrc.com/teensy/td_libs_Time.html

DS1307TYC
- http://www.pjrc.com/teensy/td_libs_DS1307RTC.html

Changes from the original:
  - I used an Arduino Nano from Sainsmart for the processor.
  - I used chose the DS3231 from SwitchDoc Labs with a rechargable LIR2032 battery.
  - I soldered everything to a circuit board using Snappable PC Breadboard.
  - Power and Ground are supplied to the DS3231 from the power and ground on the Arudino Nano not the Analog pins.
  - I simplied the setup by initializing the Pull Up restisters in 1 step by using INPUT_PULLUP instead of the 2 step process of pinMode followed with digitalWrite.
  - Moved the pins around so the PWM Analog Meter pins are 9-11, the Button & Switch inputs are 2-6, and indicator LEDs to 12-13.
  - Added indicator LED for when in Calibration mode.
  - Added a Daylight Savings Time (DST) switch.  The RTC will keep time without DST.
  - Added Smooth or Tick switch to change the display from moving smoothly or staying on the Hour, Minute, and Second until it changes.

Modified by Gregg Ubben.

