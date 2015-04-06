/*
 * Arduino Analog Clock
 *
 * This program will use analog meters, like Ampere or Volt meters, to display
 * hours, minutes, and seconds.
 *
 * First shared on Know How and Coding 101 on the TWIT.tv network.
 * Check out the following episodes for the coding:
 *     http://twit.tv/show/coding-101/54
 *     http://twit.tv/show/coding-101/55
 *     http://twit.tv/show/coding-101/56
 *     http://twit.tv/show/coding-101/57
 * Check out the following episodes for the hardware:
 *     http://twit.tv/show/know-how/132
 *     http://twit.tv/show/know-how/133
 *     http://twit.tv/show/know-how/134
 *     http://twit.tv/show/know-how/135
 *
 * Thanks to Fr. Robert Ballecer, SJ and Mark Smith.
 *
 * Additional Libraries that need to be downloaded and installed in your
 * libraries directory:
 *     Time
 *         http://www.pjrc.com/teensy/td_libs_Time.html
 *     DS1307TYC
 *         http://www.pjrc.com/teensy/td_libs_DS1307RTC.html
 *
 * Changes from the original:
 *   - I used an Arduino Nano from Sainsmart for the processor.
 *   - I used chose the DS3231 from SwitchDoc Labs with a rechargable LIR2032
 *     battery.
 *   - I soldered everything to a circuit board using Snappable PC Breadboard.
 *   - Power and Ground are supplied to the DS3231 from the power and ground
 *     on the Arudino Nano not the Analog pins.
 *   - I simplied the setup by initializing the Pull Up restisters in 1 step
 *     by using INPUT_PULLUP instead of the 2 step process of pinMode followed
 *     with digitalWrite.
 *   - Moved the pins around so the PWM Analog Meter pins are 9-11, the
 *     Button & Switch inputs are 2-6, and indicator LEDs to 12-13.
 *   - Added indicator LED for when in Calibration mode.
 *   - Added a Daylight Savings Time (DST) switch.  The RTC will keep time
 *     without DST.
 *   - Added Smooth or Tick switch to change the display from moving smoothly
 *     or staying on the Hour, Minute, and Second until it changes.
 *
 * Modified by Gregg Ubben.
 *
 */

#include <DS1307RTC.h>
#include <Time.h>
#include <Wire.h>

// What is on which pin?
// Wire assumes SDA on A4
// Wire assumes SCL on A5

#define PIN_BUTTON_MINUTE 2
#define PIN_BUTTON_HOUR 3
#define PIN_BUTTON_CALIBRATE 4
#define PIN_SWITCH_DST 5
#define PIN_SWITCH_SMOOTH 6

#define PIN_PWM_SECOND 9
#define PIN_PWM_MINUTE 10
#define PIN_PWM_HOUR 11

#define PIN_LED_CALIBRATE 12
#define PIN_LED_TICKTOCK 13

#define PWM_MAX 239


time_t lastTime;
uint8_t buttonMinute;
uint8_t buttonHour;
unsigned long millisAtTopOfSecond;
uint8_t lastTicks;

void setup() {
  
  // Configure the time set buttons and
  // Enable the build-in pull-up resistors
  pinMode(PIN_BUTTON_MINUTE, INPUT_PULLUP);
  pinMode(PIN_BUTTON_HOUR, INPUT_PULLUP);
  
  // Calibrate button
  pinMode(PIN_BUTTON_CALIBRATE, INPUT_PULLUP);
  
  // Daylight Savings Time Switch
  pinMode(PIN_SWITCH_DST, INPUT_PULLUP);
  
  // Smoooth or Tick Display Switch
  pinMode(PIN_SWITCH_SMOOTH, INPUT_PULLUP);
  
  // Init indicator LEDs
  pinMode(PIN_LED_CALIBRATE, OUTPUT);
  digitalWrite(PIN_LED_CALIBRATE, LOW);
  pinMode(PIN_LED_TICKTOCK, OUTPUT);
  digitalWrite(PIN_LED_TICKTOCK, LOW);
    
  // Setup the serial console for debug output
  Serial.begin(9600);
  while(!Serial)  // Wait for Serial to be ready.
    ;
  delay(200);
  Serial.println("Analog Clock Ready!");
  
  // Initialize the time to zero so the first time through loop()
  // it will be set and dealt with properly.
  lastTime = 0;
  millisAtTopOfSecond = 0;
  lastTicks = 0;
  
  // Initialize buttons. They pull to ground, so LOW is being pushed
  buttonMinute = HIGH;
  buttonHour = HIGH;
}


void loop() {
  // See if it is Daylight Savings Time (DST)
  uint8_t dst = digitalRead(PIN_SWITCH_DST);

  // Read the current time from the RTC and optionally add an hour if DST
  time_t thisTime = RTC.get();
  tmElements_t thisTm;
  breakTime(thisTime + (dst * SECS_PER_HOUR), thisTm);

  // Check for a read error.
  if(thisTime == 0) {
    if (RTC.chipPresent()) {
      // Chip is there, but read error. Probably transient.
      // Give a bit of time for things to recover, and try again.
      Serial.println("RTC is present, but error reading from it. Trying again.");
      delay(1000);
    } else {
      // Chip is not detected. Probably a hardware error of some kind.
      Serial.println("No RTC detected. Check circuits.");
      delay(10000);
    }
    return;
  }
 
  // Calibrate the meters
  if (digitalRead(PIN_BUTTON_CALIBRATE) == LOW) {
    digitalWrite(PIN_LED_CALIBRATE, HIGH);
    Serial.println("Calibration mode:");
    Serial.println("Adjust potentimeters so that the needles read full scale.");
    // Set analog output to maximum so we can adjust pots
    analogWrite(PIN_PWM_SECOND, PWM_MAX);
    analogWrite(PIN_PWM_MINUTE, PWM_MAX);
    analogWrite(PIN_PWM_HOUR, PWM_MAX);
    
    // Wait for the button to be released, pressed and released again.
    while(digitalRead(PIN_BUTTON_CALIBRATE) == LOW)
      ;
    while(digitalRead(PIN_BUTTON_CALIBRATE) == HIGH)
      ;
    while(digitalRead(PIN_BUTTON_CALIBRATE) == LOW)
      ;
    
    digitalWrite(PIN_LED_CALIBRATE, LOW);
    Serial.println("Calibration mode completed.");
    // Set lastTime to 0 so the time will be reset next loop.
    lastTime = 0;
    return;
  }
      
  // Look for time setting buttons.
  // Remember, HIGH is not being pushed, LOW is when it's being pushed.
  uint8_t tmp;
  tmp = digitalRead(PIN_BUTTON_MINUTE);
  if (buttonMinute == HIGH && tmp == LOW) {
    // Button was open, now being pushed.
    int tmpMin = thisTm.Minute;
    tmpMin = (tmpMin + 1) % 60;
    thisTm.Minute = tmpMin;
    thisTime = makeTime(thisTm);
    RTC.set(thisTime);
  }
  buttonMinute = tmp;
  
  tmp = HIGH;
  tmp = digitalRead(PIN_BUTTON_HOUR);
  if (buttonHour == HIGH && tmp == LOW) {
    // Button was open, now being pushed.
    uint8_t tmpHour = thisTm.Hour;
    tmpHour = (tmpHour + 1 - dst) % 24;     // Remove hour if DST to keep RTC normal
    thisTm.Hour = tmpHour;
    thisTime = makeTime(thisTm);
    RTC.set(thisTime);
  }
  buttonHour = tmp;
    
  // See if the time has changed, and print if it has.
  if(thisTime != lastTime) {
    millisAtTopOfSecond = millis();
    lastTime = thisTime;
    
    // We don't stand on proper formatting here. No leading zeros. Who cares.
    Serial.print(thisTm.Hour);
    Serial.print(":");
    Serial.print(thisTm.Minute);
    Serial.print(":");
    Serial.println(thisTm.Second);
  }

  // Set the analog outputs.  PWMs are 8 bit, 0 to 255.
  // Time is in 60 increments (seconds, minutes, etc).  
  // If we use 0 to 239, we'll get 4 "ticks" per increment.
  // We'll make good use of our PWM, and have a relatively
  // smooth movement of the needles.
  uint8_t ticks = (millis() - millisAtTopOfSecond)/250;
  if (ticks != lastTicks) {
    lastTicks = ticks;
    
    uint8_t second_offset = 0;
    uint8_t minute_offset = 0;
    uint8_t hour_offset = 0;
    
    if (digitalRead(PIN_SWITCH_SMOOTH)) {
      // Make the movement smooth
      second_offset = ticks;            // plus 1 tick for every 250ms
      minute_offset = thisTm.Second/15; // plus 1 tick for every 15 seconds
      hour_offset = thisTm.Minute/3;    // plus 1 tick for every 3 minutes
    }
    else {
      // Make the movement exact and tick only when changed.
      // No change to offsets
    }
    
    // Tick-tock the LED
    //digitalWrite(PIN_LED, ticks%2 ? HIGH : LOW);
    digitalWrite(PIN_LED_TICKTOCK, thisTm.Second%2 ? HIGH : LOW);
    
    // Seconds: 4 ticks per second, 
    // plus 1 "tick" for every 250ms since the last second update
    ticks = thisTm.Second*4 + second_offset;
    analogWrite(PIN_PWM_SECOND, ticks);
  
    // Minutes: 4 ticks per minute,
    // plus 1 "tick" for every 15 seconds.
    ticks = thisTm.Minute*4 + minute_offset;
    analogWrite(PIN_PWM_MINUTE, ticks);
  
    // Hour: This one is more complex.  Convert 24hr time from thisTm
    // to 12hr time. 240 ticks per 12 hours is 20 ticks per hour,
    // plus 1 tick for every 3 minutes.
    ticks = (thisTm.Hour%12)*20 + hour_offset;
    analogWrite(PIN_PWM_HOUR, ticks);
  }
  
  delay(50);
}
