//----------------------------------------------------------------
//-- OTTO version 9 
//-----------------------------------------------------------------

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
  #include <pins_arduino.h>
#endif

#include "GarraBot.h"

//void GarraBot::init(int YL, int YR, int RL, int RR, bool load_calibration, int NoiseSensor, int Buzzer, int USTrigger, int USEcho) {
void GarraBot::init(int YL, int YR, int RL, int RR, bool load_calibration, int NoiseSensor) {

  int USTrigger =  8;
  int USEcho    =  9;
  int Buzzer    = 13;
  
  servo_pins[0] = YL;
  servo_pins[1] = YR;
  servo_pins[2] = RL;
  servo_pins[3] = RR;

  attachServos();
  isGarraBotResting=false;

  if (load_calibration) {
    for (int i = 0; i < 4; i++) {
      int servo_trim = EEPROM.read(i);
      if (servo_trim > 128) servo_trim -= 256;
      servo[i].SetTrim(servo_trim);
    }
  }
  
  for (int i = 0; i < 4; i++) servo_position[i] = 90;

  //US sensor init with the pins:
  us.init(USTrigger, USEcho);

  //Buzzer & noise sensor pins: 
  pinBuzzer = Buzzer;
  pinNoiseSensor = NoiseSensor;

  pinMode(Buzzer,OUTPUT);
  pinMode(NoiseSensor,INPUT);
}
////////////////////////////////////////////////////////////////////////////
void GarraBot::initDC(int NoiseSensor, int Buzzer, int USTrigger, int USEcho) {

  isGarraBotResting=false;

  //US sensor init with the pins:
  us.init(USTrigger, USEcho);

  //Buzzer & noise sensor pins: 
  pinBuzzer = Buzzer;
  pinNoiseSensor = NoiseSensor;

  pinMode(Buzzer,OUTPUT);
  pinMode(NoiseSensor,INPUT);
}
///////////////////////////////////////////////////////
void GarraBot::initMATRIX(int DIN, int CS, int CLK, int rotate){
ledmatrix.init( DIN, CS, CLK, 1, rotate);   // set up Matrix display
}
void GarraBot::matrixIntensity(int intensity){
ledmatrix.setIntensity(intensity);
}
void GarraBot::initBatLevel(int batteryPIN){
  battery.init(batteryPIN);
}

///////////////////////////////////////////////////////////////////
//-- ATTACH & DETACH FUNCTIONS ----------------------------------//
///////////////////////////////////////////////////////////////////
void GarraBot::attachServos(){
    servo[0].attach(servo_pins[0]);
    servo[1].attach(servo_pins[1]);
    servo[2].attach(servo_pins[2]);
    servo[3].attach(servo_pins[3]);
}

void GarraBot::detachServos(){
    servo[0].detach();
    servo[1].detach();
    servo[2].detach();
    servo[3].detach();
}

///////////////////////////////////////////////////////////////////
//-- OSCILLATORS TRIMS ------------------------------------------//
///////////////////////////////////////////////////////////////////
void GarraBot::setTrims(int YL, int YR, int RL, int RR) {
  servo[0].SetTrim(YL);
  servo[1].SetTrim(YR);
  servo[2].SetTrim(RL);
  servo[3].SetTrim(RR);
}

void GarraBot::saveTrimsOnEEPROM() {
  
  for (int i = 0; i < 4; i++){ 
      EEPROM.write(i, servo[i].getTrim());
  } 
      
}


///////////////////////////////////////////////////////////////////
//-- BASIC MOTION FUNCTIONS -------------------------------------//
///////////////////////////////////////////////////////////////////
void GarraBot::_moveServos(int time, int  servo_target[]) {

  attachServos();
  if(getRestState()==true){
        setRestState(false);
  }

  if(time>10){
    for (int i = 0; i < 4; i++) increment[i] = ((servo_target[i]) - servo_position[i]) / (time / 10.0);
    final_time =  millis() + time;

    for (int iteration = 1; millis() < final_time; iteration++) {
      partial_time = millis() + 10;
      for (int i = 0; i < 4; i++) servo[i].SetPosition(servo_position[i] + (iteration * increment[i]));
      while (millis() < partial_time); //pause
    }
  }
  else{
    for (int i = 0; i < 4; i++) servo[i].SetPosition(servo_target[i]);
  }
  for (int i = 0; i < 4; i++) servo_position[i] = servo_target[i];
}

void GarraBot::_moveSingle(int position, int servo_number) {
if (position > 180) position = 90;
if (position < 0) position = 90;
  attachServos();
  if(getRestState()==true){
        setRestState(false);
  }
int servoNumber = servo_number;
if (servoNumber == 0){
  servo[0].SetPosition(position);
}
if (servoNumber == 1){
  servo[1].SetPosition(position);
}
if (servoNumber == 2){
  servo[2].SetPosition(position);
}
if (servoNumber == 3){
  servo[3].SetPosition(position);
}
}

void GarraBot::oscillateServos(int A[4], int O[4], int T, double phase_diff[4], float cycle=1){

  for (int i=0; i<4; i++) {
    servo[i].SetO(O[i]);
    servo[i].SetA(A[i]);
    servo[i].SetT(T);
    servo[i].SetPh(phase_diff[i]);
  }
  double ref=millis();
   for (double x=ref; x<=T*cycle+ref; x=millis()){
     for (int i=0; i<4; i++){
        servo[i].refresh();
     }
  }
}


void GarraBot::_execute(int A[4], int O[4], int T, double phase_diff[4], float steps = 1.0){

  attachServos();
  if(getRestState()==true){
        setRestState(false);
  }


  int cycles=(int)steps;    

  //-- Execute complete cycles
  if (cycles >= 1) 
    for(int i = 0; i < cycles; i++) 
      oscillateServos(A,O, T, phase_diff);
      
  //-- Execute the final not complete cycle    
  oscillateServos(A,O, T, phase_diff,(float)steps-cycles);
}



///////////////////////////////////////////////////////////////////
//-- HOME = GarraBot at rest position -------------------------------//
///////////////////////////////////////////////////////////////////
void GarraBot::home(){

  if(isGarraBotResting==false){ //Go to rest position only if necessary

    int homes[4]={90, 90, 90, 90}; //All the servos at rest position
    _moveServos(500,homes);   //Move the servos in half a second

    detachServos();
    isGarraBotResting=true;
  }
}

bool GarraBot::getRestState(){

    return isGarraBotResting;
}

void GarraBot::setRestState(bool state){

    isGarraBotResting = state;
}


///////////////////////////////////////////////////////////////////
//-- PREDETERMINED MOTION SEQUENCES -----------------------------//
///////////////////////////////////////////////////////////////////

//---------------------------------------------------------
//-- GarraBot movement: Jump
//--  Parameters:
//--    steps: Number of steps
//--    T: Period
//---------------------------------------------------------
void GarraBot::jump(float steps, int T){

  int up[]={90,90,150,30};
  _moveServos(T,up);
  int down[]={90,90,90,90};
  _moveServos(T,down);
}


//---------------------------------------------------------
//-- GarraBot gait: Walking  (forward or backward)    
//--  Parameters:
//--    * steps:  Number of steps
//--    * T : Period
//--    * Dir: Direction: FORWARD / BACKWARD
//---------------------------------------------------------
void GarraBot::walk(float steps, int T, int dir){

  //-- Oscillator parameters for walking
  //-- Hip sevos are in phase
  //-- Feet servos are in phase
  //-- Hip and feet are 90 degrees out of phase
  //--      -90 : Walk forward
  //--       90 : Walk backward
  //-- Feet servos also have the same offset (for tiptoe a little bit)
  int A[4]= {30, 30, 20, 20};
  int O[4] = {0, 0, 4, -4};
  double phase_diff[4] = {0, 0, DEG2RAD(dir * -90), DEG2RAD(dir * -90)};

  //-- Let's oscillate the servos!
  _execute(A, O, T, phase_diff, steps);  
}


//---------------------------------------------------------
//-- GarraBot gait: Turning (left or right)
//--  Parameters:
//--   * Steps: Number of steps
//--   * T: Period
//--   * Dir: Direction: LEFT / RIGHT
//---------------------------------------------------------
void GarraBot::turn(float steps, int T, int dir){

  //-- Same coordination than for walking (see GarraBot::walk)
  //-- The Amplitudes of the hip's oscillators are not igual
  //-- When the right hip servo amplitude is higher, the steps taken by
  //--   the right leg are bigger than the left. So, the robot describes an 
  //--   left arc
  int A[4]= {30, 30, 20, 20};
  int O[4] = {0, 0, 4, -4};
  double phase_diff[4] = {0, 0, DEG2RAD(-90), DEG2RAD(-90)}; 
    
  if (dir == LEFT) {  
    A[0] = 30; //-- Left hip servo
    A[1] = 10; //-- Right hip servo
  }
  else {
    A[0] = 10;
    A[1] = 30;
  }
    
  //-- Let's oscillate the servos!
  _execute(A, O, T, phase_diff, steps); 
}


//---------------------------------------------------------
//-- GarraBot gait: Lateral bend
//--  Parameters:
//--    steps: Number of bends
//--    T: Period of one bend
//--    dir: RIGHT=Right bend LEFT=Left bend
//---------------------------------------------------------
void GarraBot::bend (int steps, int T, int dir){

  //Parameters of all the movements. Default: Left bend
  int bend1[4]={90, 90, 62, 35}; 
  int bend2[4]={90, 90, 62, 105};
  int homes[4]={90, 90, 90, 90};

  //Time of one bend, constrained in order to avoid movements too fast.
  //T=max(T, 600);

  //Changes in the parameters if right direction is chosen 
  if(dir==-1)
  {
    bend1[2]=180-35;
    bend1[3]=180-60;  //Not 65. GarraBot is unbalanced
    bend2[2]=180-105;
    bend2[3]=180-60;
  }

  //Time of the bend movement. Fixed parameter to avoid falls
  int T2=800; 

  //Bend movement
  for (int i=0;i<steps;i++)
  {
    _moveServos(T2/2,bend1);
    _moveServos(T2/2,bend2);
    delay(T*0.8);
    _moveServos(500,homes);
  }

}


//---------------------------------------------------------
//-- GarraBot gait: Shake a leg
//--  Parameters:
//--    steps: Number of shakes
//--    T: Period of one shake
//--    dir: RIGHT=Right leg LEFT=Left leg
//---------------------------------------------------------
void GarraBot::shakeLeg (int steps,int T,int dir){

  //This variable change the amount of shakes
  int numberLegMoves=2;

  //Parameters of all the movements. Default: Right leg
  int shake_leg1[4]={90, 90, 58, 35};   
  int shake_leg2[4]={90, 90, 58, 120};
  int shake_leg3[4]={90, 90, 58, 60};
  int homes[4]={90, 90, 90, 90};

  //Changes in the parameters if left leg is chosen
  if(dir==-1)      
  {
    shake_leg1[2]=180-35;
    shake_leg1[3]=180-58;
    shake_leg2[2]=180-120;
    shake_leg2[3]=180-58;
    shake_leg3[2]=180-60;
    shake_leg3[3]=180-58;
  }
  
  //Time of the bend movement. Fixed parameter to avoid falls
  int T2=1000;    
  //Time of one shake, constrained in order to avoid movements too fast.            
  T=T-T2;
  T=max(T,200*numberLegMoves);  

  for (int j=0; j<steps;j++)
  {
  //Bend movement
  _moveServos(T2/2,shake_leg1);
  _moveServos(T2/2,shake_leg2);
  
    //Shake movement
    for (int i=0;i<numberLegMoves;i++)
    {
    _moveServos(T/(2*numberLegMoves),shake_leg3);
    _moveServos(T/(2*numberLegMoves),shake_leg2);
    }
    _moveServos(500,homes); //Return to home position
  }
  
  delay(T);
}


//---------------------------------------------------------
//-- GarraBot movement: up & down
//--  Parameters:
//--    * steps: Number of jumps
//--    * T: Period
//--    * h: Jump height: SMALL / MEDIUM / BIG 
//--              (or a number in degrees 0 - 90)
//---------------------------------------------------------
void GarraBot::updown(float steps, int T, int h){

  //-- Both feet are 180 degrees out of phase
  //-- Feet amplitude and offset are the same
  //-- Initial phase for the right foot is -90, so that it starts
  //--   in one extreme position (not in the middle)
  int A[4]= {0, 0, h, h};
  int O[4] = {0, 0, h, -h};
  double phase_diff[4] = {0, 0, DEG2RAD(-90), DEG2RAD(90)};
  
  //-- Let's oscillate the servos!
  _execute(A, O, T, phase_diff, steps); 
}


//---------------------------------------------------------
//-- GarraBot movement: swinging side to side
//--  Parameters:
//--     steps: Number of steps
//--     T : Period
//--     h : Amount of swing (from 0 to 50 aprox)
//---------------------------------------------------------
void GarraBot::swing(float steps, int T, int h){

  //-- Both feets are in phase. The offset is half the amplitude
  //-- It causes the robot to swing from side to side
  int A[4]= {0, 0, h, h};
  int O[4] = {0, 0, h/2, -h/2};
  double phase_diff[4] = {0, 0, DEG2RAD(0), DEG2RAD(0)};
  
  //-- Let's oscillate the servos!
  _execute(A, O, T, phase_diff, steps); 
}


//---------------------------------------------------------
//-- GarraBot movement: swinging side to side without touching the floor with the heel
//--  Parameters:
//--     steps: Number of steps
//--     T : Period
//--     h : Amount of swing (from 0 to 50 aprox)
//---------------------------------------------------------
void GarraBot::tiptoeSwing(float steps, int T, int h){

  //-- Both feets are in phase. The offset is not half the amplitude in order to tiptoe
  //-- It causes the robot to swing from side to side
  int A[4]= {0, 0, h, h};
  int O[4] = {0, 0, h, -h};
  double phase_diff[4] = {0, 0, 0, 0};
  
  //-- Let's oscillate the servos!
  _execute(A, O, T, phase_diff, steps); 
}


//---------------------------------------------------------
//-- GarraBot gait: Jitter 
//--  Parameters:
//--    steps: Number of jitters
//--    T: Period of one jitter 
//--    h: height (Values between 5 - 25)   
//---------------------------------------------------------
void GarraBot::jitter(float steps, int T, int h){

  //-- Both feet are 180 degrees out of phase
  //-- Feet amplitude and offset are the same
  //-- Initial phase for the right foot is -90, so that it starts
  //--   in one extreme position (not in the middle)
  //-- h is constrained to avoid hit the feets
  h=min(25,h);
  int A[4]= {h, h, 0, 0};
  int O[4] = {0, 0, 0, 0};
  double phase_diff[4] = {DEG2RAD(-90), DEG2RAD(90), 0, 0};
  
  //-- Let's oscillate the servos!
  _execute(A, O, T, phase_diff, steps); 
}


//---------------------------------------------------------
//-- GarraBot gait: Ascending & turn (Jitter while up&down)
//--  Parameters:
//--    steps: Number of bends
//--    T: Period of one bend
//--    h: height (Values between 5 - 15) 
//---------------------------------------------------------
void GarraBot::ascendingTurn(float steps, int T, int h){

  //-- Both feet and legs are 180 degrees out of phase
  //-- Initial phase for the right foot is -90, so that it starts
  //--   in one extreme position (not in the middle)
  //-- h is constrained to avoid hit the feets
  h=min(13,h);
  int A[4]= {h, h, h, h};
  int O[4] = {0, 0, h+4, -h+4};
  double phase_diff[4] = {DEG2RAD(-90), DEG2RAD(90), DEG2RAD(-90), DEG2RAD(90)};
  
  //-- Let's oscillate the servos!
  _execute(A, O, T, phase_diff, steps); 
}


//---------------------------------------------------------
//-- GarraBot gait: Moonwalker. GarraBot moves like Michael Jackson
//--  Parameters:
//--    Steps: Number of steps
//--    T: Period
//--    h: Height. Typical valures between 15 and 40
//--    dir: Direction: LEFT / RIGHT
//---------------------------------------------------------
void GarraBot::moonwalker(float steps, int T, int h, int dir){

  //-- This motion is similar to that of the caterpillar robots: A travelling
  //-- wave moving from one side to another
  //-- The two GarraBot's feet are equivalent to a minimal configuration. It is known
  //-- that 2 servos can move like a worm if they are 120 degrees out of phase
  //-- In the example of GarraBot, the two feet are mirrored so that we have:
  //--    180 - 120 = 60 degrees. The actual phase difference given to the oscillators
  //--  is 60 degrees.
  //--  Both amplitudes are equal. The offset is half the amplitud plus a little bit of
  //-   offset so that the robot tiptoe lightly
 
  int A[4]= {0, 0, h, h};
  int O[4] = {0, 0, h/2+2, -h/2 -2};
  int phi = -dir * 90;
  double phase_diff[4] = {0, 0, DEG2RAD(phi), DEG2RAD(-60 * dir + phi)};
  
  //-- Let's oscillate the servos!
  _execute(A, O, T, phase_diff, steps); 
}


//----------------------------------------------------------
//-- GarraBot gait: Crusaito. A mixture between moonwalker and walk
//--   Parameters:
//--     steps: Number of steps
//--     T: Period
//--     h: height (Values between 20 - 50)
//--     dir:  Direction: LEFT / RIGHT
//-----------------------------------------------------------
void GarraBot::crusaito(float steps, int T, int h, int dir){

  int A[4]= {25, 25, h, h};
  int O[4] = {0, 0, h/2+ 4, -h/2 - 4};
  double phase_diff[4] = {90, 90, DEG2RAD(0), DEG2RAD(-60 * dir)};
  
  //-- Let's oscillate the servos!
  _execute(A, O, T, phase_diff, steps); 
}


//---------------------------------------------------------
//-- GarraBot gait: Flapping
//--  Parameters:
//--    steps: Number of steps
//--    T: Period
//--    h: height (Values between 10 - 30)
//--    dir: direction: FOREWARD, BACKWARD
//---------------------------------------------------------
void GarraBot::flapping(float steps, int T, int h, int dir){

  int A[4]= {12, 12, h, h};
  int O[4] = {0, 0, h - 10, -h + 10};
  double phase_diff[4] = {DEG2RAD(0), DEG2RAD(180), DEG2RAD(-90 * dir), DEG2RAD(90 * dir)};
  
  //-- Let's oscillate the servos!
  _execute(A, O, T, phase_diff, steps); 
}






///////////////////////////////////////////////////////////////////
//-- SENSORS FUNCTIONS  -----------------------------------------//
///////////////////////////////////////////////////////////////////

//---------------------------------------------------------
//-- GarraBot getDistance: return GarraBot's ultrasonic sensor measure
//---------------------------------------------------------
float GarraBot::getDistance(){

  return us.read();
}


//---------------------------------------------------------
//-- GarraBot getNoise: return GarraBot's noise sensor measure
//---------------------------------------------------------
int GarraBot::getNoise(){

  int noiseLevel = 0;
  int noiseReadings = 0;
  int numReadings = 2;  

    noiseLevel = analogRead(pinNoiseSensor);

    for(int i=0; i<numReadings; i++){
        noiseReadings += analogRead(pinNoiseSensor);
        delay(4); // delay in between reads for stability
    }

    noiseLevel = noiseReadings / numReadings;

    return noiseLevel;
}


//---------------------------------------------------------
//-- GarraBot getBatteryLevel: return battery voltage percent
//---------------------------------------------------------
double GarraBot::getBatteryLevel(){

  //The first read of the batery is often a wrong reading, so we will discard this value. 
    double batteryLevel = battery.readBatPercent();
    double batteryReadings = 0;
    int numReadings = 10;

    for(int i=0; i<numReadings; i++){
        batteryReadings += battery.readBatPercent();
        delay(1); // delay in between reads for stability
    }

    batteryLevel = batteryReadings / numReadings;

    return batteryLevel;
}


double GarraBot::getBatteryVoltage(){

  //The first read of the batery is often a wrong reading, so we will discard this value. 
    double batteryLevel = battery.readBatVoltage();
    double batteryReadings = 0;
    int numReadings = 10;

    for(int i=0; i<numReadings; i++){
        batteryReadings += battery.readBatVoltage();
        delay(1); // delay in between reads for stability
    }

    batteryLevel = batteryReadings / numReadings;

    return batteryLevel;
}


///////////////////////////////////////////////////////////////////
//-- MOUTHS & ANIMATIONS ----------------------------------------//
///////////////////////////////////////////////////////////////////
#ifdef MOUTH_MATRIX
void GarraBot::setLed(byte X, byte Y, byte value){
  ledmatrix.setDot( X,  Y, value);
}


// EXAMPLE putAnimationMouth(dreamMouth,0);
void GarraBot::putAnimationMouth(unsigned long int aniMouth, int index){

ledmatrix.writeFull(PROGMEM_getAnything (&Gesturetable[aniMouth][index]));

}

//EXAMPLE putMouth(smile);
void GarraBot::putMouth(unsigned long int mouth, bool predefined){
  if (predefined){
// Here a direct entry into the Progmem Mouthttable is used!!

ledmatrix.writeFull(PROGMEM_getAnything(&Mouthtable[mouth]));
  }
  else{
    ledmatrix.writeFull(mouth);
  }
}


void GarraBot::clearMouth(){

  ledmatrix.clearMatrix();
}

void GarraBot::writeText(const char * s, byte scrollspeed){
 int a ;
 int b ;
  for(a = 0; s[a] != '\0'; a++){
    b = a +1 ;
    if (b > 9 ) b = 9; // only maximum of nine characters allowed
  }
  for(int charNUMBER = 0; charNUMBER <b; charNUMBER++){
      if ((* s < 48) || (* s > 91)) {
        if (* s == 32){
          ledmatrix.sendChar (44, charNUMBER, b, scrollspeed);
        }
        else
        {
          ledmatrix.sendChar (43, charNUMBER, b, scrollspeed);
        }
     }
      else
      {
      ledmatrix.sendChar ((* s - 48), charNUMBER, b, scrollspeed);
     }
  * s++;
  }

}
#endif

///////////////////////////////////////////////////////////////////
//-- SOUNDS -----------------------------------------------------//
///////////////////////////////////////////////////////////////////

void GarraBot::_tone (float noteFrequency, long noteDuration, int silentDuration){

    // tone(10,261,500);
    // delay(500);

      if(silentDuration==0){silentDuration=1;}

      TimerFreeTone(GarraBot::pinBuzzer, noteFrequency, noteDuration);
      //delay(noteDuration);       //REMOVED FOR TimerFreeTone, PUT BACK for TONE       milliseconds to microseconds
      //noTone(PIN_Buzzer);
      
      delay(silentDuration);     //
}



void GarraBot::bendTones (float initFrequency, float finalFrequency, float prop, long noteDuration, int silentDuration){

  //Examples:
  //  bendTones (880, 2093, 1.02, 18, 1);
  //  bendTones (note_A5, note_C7, 1.02, 18, 0);

  if(silentDuration==0){silentDuration=1;}

  if(initFrequency < finalFrequency)
  {
      for (int i=initFrequency; i<finalFrequency; i=i*prop) {
          _tone(i, noteDuration, silentDuration);
      }

  } else{

      for (int i=initFrequency; i>finalFrequency; i=i/prop) {
          _tone(i, noteDuration, silentDuration);
      }
  }
}


void GarraBot::sing(int songName){
  switch(songName){

    case S_connection:
      _tone(note_E5,50,30);
      _tone(note_E6,55,25);
      _tone(note_A6,60,10);
    break;

    case S_disconnection:
      _tone(note_E5,50,30);
      _tone(note_A6,55,25);
      _tone(note_E6,50,10);
    break;

    case S_buttonPushed:
      bendTones (note_E6, note_G6, 1.03, 20, 2);
      delay(30);
      bendTones (note_E6, note_D7, 1.04, 10, 2);
    break;

    case S_mode1:
      bendTones (note_E6, note_A6, 1.02, 30, 10);  //1318.51 to 1760
    break;

    case S_mode2:
      bendTones (note_G6, note_D7, 1.03, 30, 10);  //1567.98 to 2349.32
    break;

    case S_mode3:
      _tone(note_E6,50,100); //D6
      _tone(note_G6,50,80);  //E6
      _tone(note_D7,300,0);  //G6
    break;

    case S_surprise:
      bendTones(800, 2150, 1.02, 10, 1);
      bendTones(2149, 800, 1.03, 7, 1);
    break;

    case S_OhOoh:
      bendTones(880, 2000, 1.04, 8, 3); //A5 = 880
      delay(200);

      for (int i=880; i<2000; i=i*1.04) {
           _tone(note_B5,5,10);
      }
    break;

    case S_OhOoh2:
      bendTones(1880, 3000, 1.03, 8, 3);
      delay(200);

      for (int i=1880; i<3000; i=i*1.03) {
          _tone(note_C6,10,10);
      }
    break;

    case S_cuddly:
      bendTones(700, 900, 1.03, 16, 4);
      bendTones(899, 650, 1.01, 18, 7);
    break;

    case S_sleeping:
      bendTones(100, 500, 1.04, 10, 10);
      delay(500);
      bendTones(400, 100, 1.04, 10, 1);
    break;

    case S_happy:
      bendTones(1500, 2500, 1.05, 20, 8);
      bendTones(2499, 1500, 1.05, 25, 8);
    break;

    case S_superHappy:
      bendTones(2000, 6000, 1.05, 8, 3);
      delay(50);
      bendTones(5999, 2000, 1.05, 13, 2);
    break;

    case S_happy_short:
      bendTones(1500, 2000, 1.05, 15, 8);
      delay(100);
      bendTones(1900, 2500, 1.05, 10, 8);
    break;

    case S_sad:
      bendTones(880, 669, 1.02, 20, 200);
    break;

    case S_confused:
      bendTones(1000, 1700, 1.03, 8, 2); 
      bendTones(1699, 500, 1.04, 8, 3);
      bendTones(1000, 1700, 1.05, 9, 10);
    break;

    case S_fart1:
      bendTones(1600, 3000, 1.02, 2, 15);
    break;

    case S_fart2:
      bendTones(2000, 6000, 1.02, 2, 20);
    break;

    case S_fart3:
      bendTones(1600, 4000, 1.02, 2, 20);
      bendTones(4000, 3000, 1.02, 2, 20);
    break;

  }
}



///////////////////////////////////////////////////////////////////
//-- GESTURES ---------------------------------------------------//
///////////////////////////////////////////////////////////////////

void GarraBot::playGesture(int gesture){
 int gesturePOSITION[4];
  
  switch(gesture){

    case GarraBotHappy: 
        _tone(note_E5,50,30);
#ifdef MOUTH_MATRIX
        putMouth(smile);
#endif
        sing(S_happy_short);
        swing(1,800,20); 
        sing(S_happy_short);

        home();
#ifdef MOUTH_MATRIX
        putMouth(happyOpen);
#endif
    break;


    case GarraBotSuperHappy:
#ifdef MOUTH_MATRIX
        putMouth(happyOpen);
#endif
        sing(S_happy);
#ifdef MOUTH_MATRIX
        putMouth(happyClosed);
#endif
        tiptoeSwing(1,500,20);
#ifdef MOUTH_MATRIX
        putMouth(happyOpen);
#endif
        sing(S_superHappy);
#ifdef MOUTH_MATRIX
        putMouth(happyClosed);
#endif
        tiptoeSwing(1,500,20); 

        home();  
#ifdef MOUTH_MATRIX
        putMouth(happyOpen);
#endif
    break;


    case GarraBotSad: 
#ifdef MOUTH_MATRIX
        putMouth(sad);
#endif
        gesturePOSITION[0] = 110;//int sadPos[6]=      {110, 70, 20, 160};
        gesturePOSITION[1] = 70;
         gesturePOSITION[2] = 20;
          gesturePOSITION[3] = 160;
        _moveServos(700, gesturePOSITION);     
        bendTones(880, 830, 1.02, 20, 200);
#ifdef MOUTH_MATRIX
        putMouth(sadClosed);
#endif
        bendTones(830, 790, 1.02, 20, 200);  
#ifdef MOUTH_MATRIX
        putMouth(sadOpen);
#endif
        bendTones(790, 740, 1.02, 20, 200);
#ifdef MOUTH_MATRIX
        putMouth(sadClosed);
#endif
        bendTones(740, 700, 1.02, 20, 200);
#ifdef MOUTH_MATRIX
        putMouth(sadOpen);
#endif
        bendTones(700, 669, 1.02, 20, 200);
#ifdef MOUTH_MATRIX
        putMouth(sad);
#endif
        delay(500);

        home();
        delay(300);
#ifdef MOUTH_MATRIX
        putMouth(happyOpen);
#endif
    break;


    case GarraBotSleeping:
    gesturePOSITION[0] = 100;//int bedPos[6]=      {100, 80, 60, 120};
        gesturePOSITION[1] = 80;
         gesturePOSITION[2] = 60;
          gesturePOSITION[3] = 120;
        _moveServos(700, gesturePOSITION);     
        for(int i=0; i<4;i++){
#ifdef MOUTH_MATRIX
          putAnimationMouth(dreamMouth,0);
#endif
          bendTones (100, 200, 1.04, 10, 10);
#ifdef MOUTH_MATRIX
          putAnimationMouth(dreamMouth,1);
#endif
          bendTones (200, 300, 1.04, 10, 10);  
#ifdef MOUTH_MATRIX
          putAnimationMouth(dreamMouth,2);
#endif
          bendTones (300, 500, 1.04, 10, 10);   
          delay(500);
#ifdef MOUTH_MATRIX
          putAnimationMouth(dreamMouth,1);
#endif
          bendTones (400, 250, 1.04, 10, 1); 
#ifdef MOUTH_MATRIX
          putAnimationMouth(dreamMouth,0);
#endif
          bendTones (250, 100, 1.04, 10, 1); 
          delay(500);
        } 

#ifdef MOUTH_MATRIX
        putMouth(lineMouth);
#endif
        sing(S_cuddly);

        home();  
#ifdef MOUTH_MATRIX
        putMouth(happyOpen);
#endif
    break;


    case GarraBotFart:
    gesturePOSITION[0] = 90;// int fartPos_1[6]=   {90, 90, 145, 122};
        gesturePOSITION[1] = 90;
         gesturePOSITION[2] = 145;
          gesturePOSITION[3] = 122;
        _moveServos(500,gesturePOSITION);
        delay(300);     
#ifdef MOUTH_MATRIX
        putMouth(lineMouth);
#endif
        sing(S_fart1);  
#ifdef MOUTH_MATRIX
        putMouth(tongueOut);
#endif
        delay(250);
        gesturePOSITION[0] = 90;// int fartPos_2[6]=   {90, 90, 80, 122};
        gesturePOSITION[1] = 90;
         gesturePOSITION[2] = 80;
          gesturePOSITION[3] = 122;
        _moveServos(500,gesturePOSITION);
        delay(300);
#ifdef MOUTH_MATRIX
        putMouth(lineMouth);
#endif
        sing(S_fart2); 
#ifdef MOUTH_MATRIX
        putMouth(tongueOut);
#endif
        delay(250);
        gesturePOSITION[0] = 90;// int fartPos_3[6]=   {90, 90, 145, 80};
        gesturePOSITION[1] = 90;
         gesturePOSITION[2] = 145;
          gesturePOSITION[3] = 80;
        _moveServos(500,gesturePOSITION);
        delay(300);
#ifdef MOUTH_MATRIX
        putMouth(lineMouth);
#endif
        sing(S_fart3);
#ifdef MOUTH_MATRIX
        putMouth(tongueOut);    
#endif
        delay(300);

        home(); 
        delay(500); 
#ifdef MOUTH_MATRIX
        putMouth(happyOpen);
#endif
    break;


    case GarraBotConfused:
    gesturePOSITION[0] = 110;//int confusedPos[6]= {110, 70, 90, 90};
        gesturePOSITION[1] = 70;
         gesturePOSITION[2] = 90;
          gesturePOSITION[3] = 90;
        _moveServos(300, gesturePOSITION); 
#ifdef MOUTH_MATRIX
        putMouth(confused);
#endif
        sing(S_confused);
        delay(500);

        home();  
#ifdef MOUTH_MATRIX
        putMouth(happyOpen);
#endif
    break;


    case GarraBotLove:
#ifdef MOUTH_MATRIX
        putMouth(heart);
#endif
        sing(S_cuddly);
        crusaito(2,1500,15,1);

        home(); 
        sing(S_happy_short);  
#ifdef MOUTH_MATRIX
        putMouth(happyOpen);
#endif
    break;


    case GarraBotAngry: 
    gesturePOSITION[0] = 90;//int angryPos[6]=    {90, 90, 70, 110};
        gesturePOSITION[1] = 90;
         gesturePOSITION[2] = 70;
          gesturePOSITION[3] = 110;
        _moveServos(300, gesturePOSITION); 
#ifdef MOUTH_MATRIX
        putMouth(angry);
#endif

        _tone(note_A5,100,30);
        bendTones(note_A5, note_D6, 1.02, 7, 4);
        bendTones(note_D6, note_G6, 1.02, 10, 1);
        bendTones(note_G6, note_A5, 1.02, 10, 1);
        delay(15);
        bendTones(note_A5, note_E5, 1.02, 20, 4);
        delay(400);
        gesturePOSITION[0] = 110;//int headLeft[6]=    {110, 110, 90, 90};
        gesturePOSITION[1] = 110;
         gesturePOSITION[2] = 90;
          gesturePOSITION[3] = 90;
        _moveServos(200, gesturePOSITION); 
        bendTones(note_A5, note_D6, 1.02, 20, 4);
        gesturePOSITION[0] = 70;//int headRight[6]=   {70, 70, 90, 90};
        gesturePOSITION[1] = 70;
         gesturePOSITION[2] = 90;
          gesturePOSITION[3] = 90;
        _moveServos(200, gesturePOSITION); 
        bendTones(note_A5, note_E5, 1.02, 20, 4);

        home();  
#ifdef MOUTH_MATRIX
        putMouth(happyOpen);
#endif
    break;


    case GarraBotFretful: 
#ifdef MOUTH_MATRIX
        putMouth(angry);
#endif
        bendTones(note_A5, note_D6, 1.02, 20, 4);
        bendTones(note_A5, note_E5, 1.02, 20, 4);
        delay(300);
#ifdef MOUTH_MATRIX
        putMouth(lineMouth);
#endif

        for(int i=0; i<4; i++){
          gesturePOSITION[0] = 90;//int fretfulPos[6]=  {90, 90, 90, 110};
        gesturePOSITION[1] = 90;
         gesturePOSITION[2] = 90;
          gesturePOSITION[3] = 110;
          _moveServos(100, gesturePOSITION);   
          home();
        }

#ifdef MOUTH_MATRIX
        putMouth(angry);
#endif
        delay(500);

        home();  
#ifdef MOUTH_MATRIX
        putMouth(happyOpen);
#endif
    break;


    case GarraBotMagic:

        //Initial note frecuency = 400
        //Final note frecuency = 1000
        
        // Reproduce the animation four times
        for(int i = 0; i<4; i++){ 

          int noteM = 400; 

            for(int index = 0; index<6; index++){
#ifdef MOUTH_MATRIX
              putAnimationMouth(adivinawi,index);
#endif
              bendTones(noteM, noteM+100, 1.04, 10, 10);    //400 -> 1000 
              noteM+=100;
            }

#ifdef MOUTH_MATRIX
            clearMouth();
#endif
            bendTones(noteM-100, noteM+100, 1.04, 10, 10);  //900 -> 1100

            for(int index = 0; index<6; index++){
#ifdef MOUTH_MATRIX
              putAnimationMouth(adivinawi,index);
#endif
              bendTones(noteM, noteM+100, 1.04, 10, 10);    //1000 -> 400 
              noteM-=100;
            }
        } 
 
        delay(300);
#ifdef MOUTH_MATRIX
        putMouth(happyOpen);
#endif
    break;


    case GarraBotWave:
        
        // Reproduce the animation four times
        for(int i = 0; i<2; i++){ 

            int noteW = 500; 

            for(int index = 0; index<10; index++){
#ifdef MOUTH_MATRIX
              putAnimationMouth(wave,index);
#endif
              bendTones(noteW, noteW+100, 1.02, 10, 10); 
              noteW+=101;
            }
            for(int index = 0; index<10; index++){
#ifdef MOUTH_MATRIX
              putAnimationMouth(wave,index);
#endif
              bendTones(noteW, noteW+100, 1.02, 10, 10); 
              noteW+=101;
            }
            for(int index = 0; index<10; index++){
#ifdef MOUTH_MATRIX
              putAnimationMouth(wave,index);
#endif
              bendTones(noteW, noteW-100, 1.02, 10, 10); 
              noteW-=101;
            }
            for(int index = 0; index<10; index++){
#ifdef MOUTH_MATRIX
              putAnimationMouth(wave,index);
#endif
              bendTones(noteW, noteW-100, 1.02, 10, 10); 
              noteW-=101;
            }
        }    

#ifdef MOUTH_MATRIX
        clearMouth();
#endif
        delay(100);
#ifdef MOUTH_MATRIX
        putMouth(happyOpen);
#endif
    break;

    case GarraBotVictory:
        
#ifdef MOUTH_MATRIX
        putMouth(smallSurprise);
#endif
        //final pos   = {90,90,150,30}
        for (int i = 0; i < 60; ++i){
          int pos[]={90,90,90+i,90-i};  
          _moveServos(10,pos);
          _tone(1600+i*20,15,1);
        }

#ifdef MOUTH_MATRIX
        putMouth(bigSurprise);
#endif
        //final pos   = {90,90,90,90}
        for (int i = 0; i < 60; ++i){
          int pos[]={90,90,150-i,30+i};  
          _moveServos(10,pos);
          _tone(2800+i*20,15,1);
        }

#ifdef MOUTH_MATRIX
        putMouth(happyOpen);
#endif
        //SUPER HAPPY
        //-----
        tiptoeSwing(1,500,20);
        sing(S_superHappy);
#ifdef MOUTH_MATRIX
        putMouth(happyClosed);
#endif
        tiptoeSwing(1,500,20); 
        //-----

        home();
#ifdef MOUTH_MATRIX
        clearMouth();
        putMouth(happyOpen);
#endif

    break;

    case GarraBotFail:
#ifdef MOUTH_MATRIX
        putMouth(sadOpen);
#endif
         gesturePOSITION[0] = 90;//int bendPos_1[6]=   {90, 90, 70, 35};
        gesturePOSITION[1] = 90;
         gesturePOSITION[2] = 70;
          gesturePOSITION[3] = 35;
        _moveServos(300,gesturePOSITION);
        _tone(900,200,1);
#ifdef MOUTH_MATRIX
        putMouth(sadClosed);
#endif
        gesturePOSITION[0] = 90;//int bendPos_2[6]=   {90, 90, 55, 35};
        gesturePOSITION[1] = 90;
         gesturePOSITION[2] = 55;
          gesturePOSITION[3] = 35;
        _moveServos(300,gesturePOSITION);
        _tone(600,200,1);
#ifdef MOUTH_MATRIX
        putMouth(confused);
#endif
        gesturePOSITION[0] = 90;//int bendPos_3[6]=   {90, 90, 42, 35};
        gesturePOSITION[1] = 90;
         gesturePOSITION[2] = 42;
          gesturePOSITION[3] = 35;
        _moveServos(300,gesturePOSITION);
        _tone(300,200,1);
        gesturePOSITION[0] = 90;//int bendPos_4[6]=   {90, 90, 34, 35};
        gesturePOSITION[1] = 90;
         gesturePOSITION[2] = 34;
          gesturePOSITION[3] = 35;
        _moveServos(300,gesturePOSITION);
#ifdef MOUTH_MATRIX
        putMouth(xMouth);
#endif

        detachServos();
        _tone(150,2200,1);
        
        delay(600);
#ifdef MOUTH_MATRIX
        clearMouth();
#endif
#ifdef MOUTH_MATRIX
        putMouth(happyOpen);
#endif
        home();

    break;

  }
} 
