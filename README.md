# Arduino-based altimeter
This is a project of an aviation device which gets atmospheric pressure from BMP085 sensor, recompute it to the altitude in meters (feets are not implemented yet) above (or below) some basic level and show the result on a graphic TFT-screen. Basic level is selected via encoder knob by entering base atmospheric pressure. It can be pressure either on a ground level (QFE) or on a sea level (QNH). Of cource you can also use the standard pressure 1013 hectopascal (760 millimeters of mercury) — QNE.
The project uses only standard arduino libraries such as <Wire.h> or <Arduino.h>.
## Feature list
- Digital value of altitude in meters which emulates analog counter;
- Analog value of altitude on a ruler;
- Entering base pressure via encoder knob;
- Using both hectopascals and millimeters of mercury;
- Entering specified altitude via encoder knob;
- Displaying specified altitude on the analog ruler and as digital value;
- Digital value of vertical speed in meters per second;
- Analog value of vertical speed using up and down arrows on a logarithmic scale;
- Flight level: the altitude in hundreds of feets based on the standard pressure;
- Temperature;
- Real time clock with backup battery;
- Timer that can be used for the flight time counting;
- LED indicator of approaching to specific altitude;
- LED indicator of timer working;
- Storing parameters in EEPROM.
## Hardware
- Arduino Mega 2560;
- TFT display 480x320, 16-bit color (RGB 5-6-5);
- DS3231 as RTC;
- BMP085 as pressure and temperature sensor;
- LM2596-5.0 as power supply;
- 3D-printed box, buttons and knobs.
