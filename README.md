# Libraries
A purpose of this repository is to discuss and document properly the GarraBot libraries for Arduino IDE.

## Installation:
How to install the Arduino IDE and libraries?
Answer: "There are multiple ways to add libraries to your Arduino IDE after installed the most easy for me is to just Download all the libraries from the files tab for the specific project and move them to C:\Users\user\Documents\Arduino\libraries (or wherever your libraries folder was installed)
[Another way is to use .zip installation through menu is a very general topic that all new Arduino users face](https://www.arduino.cc/en/guide/libraries) 
 
## Compatibility:
GarraBot Core Libraries is the code to be deployed to the user's Documents\Arduino\libraries directory, including: BatReader, EnableInterrupt, MaxMatrix, Oscillator, GarraBot, GarraBotSerialCommand
Different and similar versions of GarraBot Core Libraries are currently included in multiple repositories and sub-projects:

Robot  | Library can use
------------- | -------------
GarraBot | GarraBot
GarraBot+  | GarraBot
GarraBot Eyes  | GarraBot
W (Wheels)  | GarraBot

To resolve the need to maintain so many different deployments/versions here in this centralized repository, in order to maintin consistency along the GarraBot remixes we stablished a naming structure: 
GarraBot_(followed by the component name and version) for example:

* GarraBot contains all the main functions
* GarraBot_gestures contains all the gestures functions
* GarraBot_mouth	contains all the mouth functions
* GarraBot_sound contains all the sound functions
* GarraBot_Matrix contains all the matrix functions

Complementary libraries like Ultrasound, Interrupts and BatReader are independent of GarraBot (they can work with other projects) therefore are just named like they are, for example:
* US contains the functions of the ultrasound
* Bat Reader contains the functions to meassure the battery.
* Adafruit_NeoPixel external library from Adafruit to control NeoPixel LEDs
 
 ## Wirings and Electornic diagrams:
We will follow these standard connections

SERVOS:
```
PIN_YL           2 // Left leg
PIN_YR           3 // Right leg
PIN_RL           4 // Left foot
PIN_RR           5 // Right foot
PIN_LA           6 // Left arm if enabled
PIN_RA           7 // Right arm if enabled
```

ULTRASONIC:
```
PIN_Trigger      8 // TRIGGER pin (8)
PIN_Echo         9 // ECHO pin (9)
```

BUZZER:
```
PIN_Buzzer      13 // BUZZER pin (13)
```

SOUND SENSOR:
```
PIN_NoiseSensor A6 // SOUND SENSOR   ANALOG pin (A6)
```
 
LED MATRIX:
```
DIN_PIN         A3 // DIN pin (A3)
CS_PIN          A2 // CS pin (A2)
CLK_PIN         A1 // CLK pin (A1)
LED_DIRECTION    1 // LED MATRIX CONNECTOR position (orientation) 1 = top 2 = bottom 3 = left 4 = right  DEFAULT = 1
```
BATTERY SENSE:
```
boolean BATTcheck = true;    // SET TO FALSE IF NOT USING THIS OPTION
PIN_Battery     A7 // 3.7V BATTERY MONITOR   ANALOG pin (A7)
```
TOUCH SENSOR or PUSH BUTTON 
```
PIN_Button      A0 // TOUCH SENSOR Pin (A0) PULL DOWN RESISTOR MAYBE REQUIRED to stop false interrupts (interrupt PIN)
```
SERVO ASSEMBLY:
to help assemble GarraBot's feet and legs - wire link between GND and pin 7 for PLUS & pin 10 for humanoid
```
PIN_ASSEMBLY    10 // ASSEMBLY pin (10) LOW = assembly    HIGH  = normal operation
```
 
 ## How to Contribute
 Contributing to this software is warmly welcomed. There are 3 ways you can contribute to this project:
1. Test and if find a problem then post an issue.
2. Helps us solve the issues or other bugs by commenting
3. Bring missing libraries from other GarraBot remixes here.
You can do this [basically by forking](https://help.github.com/en/articles/fork-a-repo), committing modifications and then [pulling requests](https://help.github.com/en/articles/about-pull-requests). Please add a change log and your contact into file header.

Thanks for your contribution.
Just make sure to keep consistency in the naming and make a record of the change or improvement made.
Welcome to the GarraBot DIY code development team!
