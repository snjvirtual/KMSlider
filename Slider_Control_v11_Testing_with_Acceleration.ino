#include <EEPROMex.h>
#include <EEPROMVar.h>

#include <U8glib.h>
#include <ClickEncoder.h>
#include <TimerOne.h>
#include <AccelStepper.h>
#include <MultiStepper.h>


U8GLIB_ST7920_128X64_1X u8g(23, 17, 16); // SPI Com: SCK = en = 23, MOSI = rw = 17, CS = di = 16
/*
 // For RAMPS 1.4
#define X_STEP_PIN         54
#define X_DIR_PIN          55
#define X_ENABLE_PIN       38
#define X_MIN_PIN           3
#define X_MAX_PIN           2

#define Y_STEP_PIN         60
#define Y_DIR_PIN          61
#define Y_ENABLE_PIN       56
#define Y_MIN_PIN          14
#define Y_MAX_PIN          15

#define Z_STEP_PIN         46
#define Z_DIR_PIN          48
#define Z_ENABLE_PIN       62
#define Z_MIN_PIN          18
#define Z_MAX_PIN          19

#define E_STEP_PIN         26
#define E_DIR_PIN          28
#define E_ENABLE_PIN       24

#define SDPOWER            -1
#define SDSS               53
#define LED_PIN            13

#define FAN_PIN            9

#define PS_ON_PIN          12
#define KILL_PIN           -1

#define HEATER_0_PIN       10
#define HEATER_1_PIN       8
#define TEMP_0_PIN          13   // ANALOG NUMBERING
#define TEMP_1_PIN          14   // ANALOG NUMBERING

AccelStepper stepper(1, 46, 48); // Defaults to AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5
AccelStepper stepper1(1, 54, 55);

   pinMode(62, OUTPUT);
   pinMode(38, OUTPUT);

*/

ClickEncoder *encoder;
int16_t last, value;
int menu0curs = 0;
int menu1curs;
int menu2curs;
int menu3curs = 800;  //Time Curser || Change the Default Speed Here
int menu4curs;
long menu11curs; //Slide moving counter
long menu12curs; //Pan Moving Counter
long menu13curs; //Tilt Moving Counter
long menu14curs = 2000; //Increment Counter
long menu16curs;
int incre;
boolean lcdref = 1;
int pgno = 0;
boolean justrendered;

//MOTION VARIABLES
#define S_STEP_PIN         54
#define S_DIR_PIN          55
#define S_ENABLE_PIN       38

#define P_STEP_PIN         46
#define P_DIR_PIN          48
#define P_ENABLE_PIN       62

#define T_STEP_PIN         36
#define T_DIR_PIN          34
#define T_ENABLE_PIN       30

#define F_STEP_PIN         60
#define F_DIR_PIN          61
#define F_ENABLE_PIN       56
int T_Speed = 5000;
int Accel = T_Speed / 2;

long apositions[4];
long bpositions[4];
long cpositions[4];
long currentpos;
long memory[13];

int address = 0;

int movetime;
int move_distance;
int move_speed;
int move_multiplier = 100;
unsigned long previousMillis = 0;
const long milliinterval = 10;
const long millistepstop = 100;

boolean pos;
boolean moving;
boolean aftermove;

AccelStepper s_stepper(1, S_STEP_PIN, S_DIR_PIN);
AccelStepper p_stepper(1, P_STEP_PIN, P_DIR_PIN);
AccelStepper t_stepper(1, T_STEP_PIN, T_DIR_PIN);
AccelStepper f_stepper(1, F_STEP_PIN, F_DIR_PIN);

MultiStepper steppers;

void timerIsr() {
  encoder->service();
}



void setup() {
  Serial.begin(9600);
  Serial.println("Connected");
//____________Setup Encoder Settings_____________

  encoder = new ClickEncoder(33, 31, 35);
  pinMode (37, OUTPUT);
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr); 
  last = 0;

  Serial.println("Encoder Setup .............. PASS");

//____________SETUP DISPLAY SETTINGS____________
  u8g.setFont(u8g_font_profont12);
  u8g.setColorIndex(1); // Instructs the display to draw with a pixel on. 

  Serial.println("Display Setup .............. PASS");
  
//___________SETUP MOTORS_____________
s_stepper.setMaxSpeed(T_Speed); 
s_stepper.setAcceleration(Accel);
pinMode(S_ENABLE_PIN, OUTPUT);
  Serial.println("Slide Motor .............. ENABLED");

p_stepper.setMaxSpeed(T_Speed); 
p_stepper.setAcceleration(Accel);
pinMode(P_ENABLE_PIN, OUTPUT);
  Serial.println("Pan Motor .............. ENABLED");

t_stepper.setMaxSpeed(T_Speed); 
t_stepper.setAcceleration(Accel);
pinMode(T_ENABLE_PIN, OUTPUT);
  Serial.println("Tilt Motor .............. ENABLED");

f_stepper.setMaxSpeed(T_Speed); 
f_stepper.setAcceleration(Accel);
pinMode(F_ENABLE_PIN, OUTPUT);
  Serial.println("Focus Motor .............. ENABLED");

steppers.addStepper(s_stepper);
steppers.addStepper(p_stepper);
steppers.addStepper(t_stepper);
steppers.addStepper(f_stepper);

  Serial.println("Motor Setup .............. PASS");

EEPROM.readBlock(address, memory, 13);
apositions[0] = memory[0];
apositions[1] = memory[1];
apositions[2] = memory[2];
apositions[3] = memory[3];
bpositions[0] = memory[4];
bpositions[1] = memory[5];
bpositions[2] = memory[6];
bpositions[3] = memory[7];
cpositions[0] = memory[8];
cpositions[1] = memory[9];
cpositions[2] = memory[10];
cpositions[3] = memory[11];
currentpos = memory[12];


  Serial.println("Way Point Acquisition...............PASS");
  Serial.println(bpositions[0]);


  Serial.println("...............Initiating Core Program.................");


  
}

void loop() {  
  value += encoder->getValue();
  ClickEncoder::Button b = encoder->getButton();
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= milliinterval){
    s_stepper.setMaxSpeed(T_Speed);
    p_stepper.setMaxSpeed(T_Speed);
    t_stepper.setMaxSpeed(T_Speed);
    f_stepper.setMaxSpeed(T_Speed);
    s_stepper.run();
    p_stepper.run();
    t_stepper.run();
    f_stepper.run();
  }
//  if ( moving == 1 ){
//    steppers.run();
//    }
    if (s_stepper.distanceToGo() == 0 || p_stepper.distanceToGo() == 0 || t_stepper.distanceToGo() == 0 || f_stepper.distanceToGo() == 0 ){
      moving = 0;
    }
  
  
  if (value != last) {
    incre = value - last;
    if (pgno == 11){
      menu11counter();
        s_stepper.moveTo(menu11curs);
     } 
    if (pgno == 12){
      menu12counter();
        p_stepper.moveTo(menu12curs);
     } 
    if (pgno == 13){
      menu13counter();
        t_stepper.moveTo(menu13curs);
     } 
     if (pgno == 16){
      menu16counter();
        f_stepper.moveTo(menu16curs);
     } 
    if ( pgno == 0){
      menu0counter();
    }
    if ( pgno == 1){
      menu1counter();
    }
    if ( pgno == 2){
      menu2counter();
    }
    if ( pgno == 3){
      menu3counter();
    }
    if ( pgno == 4){
      menu4counter();
    }
    if ( pgno == 14){
      menu14counter();
    }
    if (pgno == 15){
      pgno = 4;
    }
    last = value;
    lcdref = 1;
  }
  if (lcdref == 1){
    pgsort();
  }
  if (b != ClickEncoder::Open) {
    bclick();
    Serial.println("Button Press Detected");
    lcdref = 1;
    pgsort();
  }
  lcdref = 0;
}



//Sorts which page is current and send draw command
void pgsort(){
  Serial.println("Sorting Pages");
  if (pgno == 0){
    drawpg0(); 
    Serial.println("In Main Menu");
  }
  
  if (pgno == 1) {
    drawpg1();
    Serial.println("In Move Axis");
  }
  if (pgno == 2){
    drawpg2();
    Serial.println("In Set Positions");
  }
  if (pgno == 3){
    drawpg3();
    Serial.println("In Set Time");
  }
  if (pgno == 4){
    drawpg4();
    Serial.println("In Perform Action");
  }
  if (pgno==11){
    drawpg11();
  }
  if (pgno==12){
    drawpg12();
  }
  if (pgno==13){
    drawpg13();
  }
  if (pgno==14){
    drawpg14();
  }
  if (pgno==15){
    drawpg15();
  }
  if (pgno==16){
    drawpg16();
  }
}

// Navigates to different pages on button click
void bclick(){
  if (pgno == 0){
    if (menu0curs == 0){
      pgno = 1;
      menu1curs = 1;
      justrendered = 1;
    }
    if (menu0curs == 1){
      pgno = 2;
      menu2curs =1;
      justrendered = 1;
    }
    if (menu0curs == 2){
      pgno = 3;
      justrendered = 1;
    }
    if (menu0curs == 3){
      pgno = 4;
      menu4curs =1;
      justrendered = 1;
    }
  }
  
  //Add Other Menu itelms
  if (pgno == 1 && justrendered == 0){
    if (menu1curs == 0){
      pgno = 0;
      justrendered = 1; 
    }
    if (menu1curs == 1){
      pgno = 11;
      justrendered = 1; 
    }
    if (menu1curs == 2){
      pgno = 12;
      justrendered = 1; 
    }
    if (menu1curs == 3){
      pgno = 13;
      justrendered = 1; 
    }
    if (menu1curs == 4){
      pgno = 16;
      justrendered = 1; 
    }
    if (menu1curs == 5){
      pgno = 14;
      justrendered = 1; 
    }  
  }
  
  if (pgno == 2 && justrendered == 0){
    if (menu2curs == 0){
      pgno = 0;
      justrendered = 1; 
    }
    if (menu2curs == 1){
      pgno = 0;
      apositions[0] = menu11curs;
      apositions[1] = menu12curs;
      apositions[2] = menu13curs;
      apositions[3] = menu16curs;
      memory[0] = apositions[0];
      memory[1] = apositions[1];
      memory[2] = apositions[2];
      memory[3] = apositions[3];
      currentpos = 1;
      memory[12] = currentpos;
      EEPROM.updateBlock(address, memory, 13);
      Serial.println("Memory Updated");
      Serial.println("A Point Saved");
      pos = 0;
      justrendered = 1; 
    }
    if (menu2curs == 2){
      pgno = 0;
      bpositions[0] = menu11curs;
      bpositions[1] = menu12curs;
      bpositions[2] = menu13curs;
      bpositions[3] = menu16curs;
      memory[4] = bpositions[0];
      memory[5] = bpositions[1];
      memory[6] = bpositions[2];
      memory[7] = bpositions[3];
      currentpos = 2;
      memory[12] = currentpos;
      EEPROM.updateBlock(address, memory, 13);
      Serial.println("Memory Updated");
      Serial.println("B Point Saved");
      pos = 1;
      justrendered = 1; 
    }
    if (menu2curs == 3){
      pgno = 0;
      cpositions[0] = menu11curs;
      cpositions[1] = menu12curs;
      cpositions[2] = menu13curs;
      cpositions[3] = menu16curs;
      memory[8] = cpositions[0];
      memory[9] = cpositions[1];
      memory[10] = cpositions[2];
      memory[11] = cpositions[3];
      currentpos = 3;
      memory[12] = currentpos;
      EEPROM.updateBlock(address, memory, 13);
      Serial.println("Memory Updated");
      Serial.println("C Point Saved");
      pos = 1;
      justrendered = 1;
    }
    
  }
  if (pgno == 3 && justrendered == 0){
      pgno = 0;
      T_Speed = menu3curs;
      justrendered = 1; 
    }
  if (pgno == 4 && justrendered == 0){
    digitalWrite(S_ENABLE_PIN, LOW);
    digitalWrite(P_ENABLE_PIN, LOW);
    digitalWrite(T_ENABLE_PIN, LOW);
    digitalWrite(F_ENABLE_PIN, LOW);
    if (menu4curs == 0){
      pgno = 0;
      justrendered = 1; 
    }
    if (menu4curs == 1 && justrendered == 0){
      Serial.println("Motion Starting A to B");
      Serial.println(bpositions[0]);
      s_stepper.setMaxSpeed(T_Speed);
      p_stepper.setMaxSpeed(T_Speed);
      t_stepper.setMaxSpeed(T_Speed);
      f_stepper.setMaxSpeed(T_Speed);
      if (currentpos != 1){
        steppers.moveTo(apositions);
        steppers.runSpeedToPosition();
        delay(1000);
      }
      steppers.moveTo(bpositions);
      steppers.runSpeedToPosition();
      menu11curs = bpositions[0];
      menu12curs = bpositions[1];
      menu13curs = bpositions[2];
      menu16curs = bpositions[3];
      aftermove = 1;
      pgno = 0;
      menu4curs = 2;
      justrendered = 1;
      currentpos = 2;
      memory[12] = currentpos;
      EEPROM.updateBlock(address, memory, 13);
      Serial.println("Memory Updated");
      
    }
    
    if (menu4curs == 2 && justrendered == 0){
      Serial.println("Motion Starting B to A");
      Serial.println(apositions[0]);
      s_stepper.setMaxSpeed(T_Speed);
      p_stepper.setMaxSpeed(T_Speed);
      t_stepper.setMaxSpeed(T_Speed);
      f_stepper.setMaxSpeed(T_Speed);
      if (currentpos != 2){
        steppers.moveTo(bpositions);
        steppers.runSpeedToPosition();
        delay(1000);
      }
      
      steppers.moveTo(apositions);
      steppers.runSpeedToPosition();
      menu11curs = apositions[0];
      menu12curs = apositions[1];
      menu13curs = apositions[2];
      menu16curs = apositions[3];
      aftermove = 1;
      pgno = 0;
      menu4curs = 1;
      justrendered = 1; 
      moving = 1;
      currentpos = 1;
      memory[12] = currentpos;
      EEPROM.updateBlock(address, memory, 13);
      Serial.println("Memory Updated");
    }
    if (menu4curs == 3 && justrendered == 0){
      Serial.println("Motion Starting B to A");
      Serial.println(apositions[0]);
      s_stepper.setMaxSpeed(T_Speed);
      p_stepper.setMaxSpeed(T_Speed);
      t_stepper.setMaxSpeed(T_Speed);
      f_stepper.setMaxSpeed(T_Speed);
      if (currentpos != 1){
        steppers.moveTo(apositions);
        steppers.runSpeedToPosition();
        delay(1000);
      }
      steppers.moveTo(bpositions);
      steppers.runSpeedToPosition();
      steppers.moveTo(cpositions);
      steppers.runSpeedToPosition();
      menu11curs = cpositions[0];
      menu12curs = cpositions[1];
      menu13curs = cpositions[2];
      menu16curs = cpositions[3];
      aftermove = 1;
      pgno = 0;
      menu4curs = 1;
      justrendered = 1; 
      moving = 1;
      currentpos = 3;
      memory[12] = currentpos;
      EEPROM.updateBlock(address, memory, 13);
      Serial.println("Memory Updated");
    }
    if (menu4curs == 4 && justrendered == 0){
      s_stepper.setMaxSpeed(T_Speed);
      p_stepper.setMaxSpeed(T_Speed);
      t_stepper.setMaxSpeed(T_Speed);
      f_stepper.setMaxSpeed(T_Speed);
      if (currentpos != 1){
        steppers.moveTo(apositions);
        steppers.runSpeedToPosition();
      }
      steppers.moveTo(bpositions);
      steppers.runSpeedToPosition();
      steppers.moveTo(cpositions);
      steppers.runSpeedToPosition();
      delay(500);
      steppers.moveTo(bpositions);
      steppers.runSpeedToPosition();
      steppers.moveTo(apositions);
      steppers.runSpeedToPosition();
      delay(500);
      steppers.moveTo(bpositions);
      steppers.runSpeedToPosition();
      steppers.moveTo(cpositions);
      steppers.runSpeedToPosition();
      delay(500);
      steppers.moveTo(bpositions);
      steppers.runSpeedToPosition();
      steppers.moveTo(apositions);
      steppers.runSpeedToPosition();
      menu11curs = s_stepper.currentPosition();
      menu12curs = p_stepper.currentPosition();
      menu13curs = t_stepper.currentPosition();
      menu16curs = f_stepper.currentPosition();
      currentpos = 1;
      memory[12] = currentpos;
      EEPROM.updateBlock(address, memory, 13);
      Serial.println("Memory Updated");
      aftermove = 1;
      pgno = 0;
      menu4curs = 3;
      justrendered = 1; 
  }
  }
  if (pgno == 11 && justrendered == 0){
      pgno = 1;
      justrendered = 1;
  }
  if (pgno == 12 && justrendered == 0){
      pgno = 1;
      justrendered = 1;
  }
  if (pgno == 13 && justrendered == 0){
      pgno = 1;
      justrendered = 1;
  }
  if (pgno == 14 && justrendered == 0){
      pgno = 1;
      justrendered = 1; 
  }
  if (pgno == 15 && justrendered == 0){
      pgno = 4;
      justrendered = 1; 
  }
  if (pgno == 16 && justrendered == 0){
      pgno = 1;
      justrendered = 1; 
  }
justrendered = 0;
Serial.print("OK click works"); 
}


//Menu Cursor Counter for each Menu
//Main Menu Counter
void menu0counter(){
    if (menu0curs <= 3 && menu0curs >= 0){
      if ((menu0curs + incre) < 0) {
      menu0curs = 3;  
      }
      else if ((menu0curs + incre) > 3){
      menu0curs = 0;  
      }
      else if ((menu0curs + incre) <= 3 && (menu0curs + incre) >= 0){
      menu0curs = menu0curs + incre;
      }
    }   
}

//Move Axis Second Menu Counter
void menu1counter(){
    if (menu1curs <= 5 && menu1curs >= 0){
      if ((menu1curs + incre) < 0) {
      menu1curs = 5;  
      }
      else if ((menu1curs + incre) > 5){
      menu1curs = 0;  
      }
      else if ((menu1curs + incre) <= 5 && (menu1curs + incre) >= 0){
      menu1curs = menu1curs + incre;
      }
    }   
}

//Set Position Second Menu Counter
void menu2counter(){
    if (menu2curs <= 3 && menu2curs >= 0){
      if ((menu2curs + incre) < 0) {
      menu2curs = 3;  
      }
      else if ((menu2curs + incre) > 3){
      menu2curs = 0;  
      }
      else if ((menu2curs + incre) <= 3 && (menu2curs + incre) >= 0){
      menu2curs = menu2curs + incre;
      }
    }  
}
//Set Time Menu Counter
void menu3counter(){
      if (incre != 0){
        if ((menu3curs + incre) < 0) {
      menu3curs = 0;  
      }
      else {
      menu3curs = menu3curs + (incre * 50);
      }
      lcdref = 1;  
      }
}

    
//Start Motion Second Menu Counter
void menu4counter(){
  if (menu4curs <= 4 && menu4curs >= 0){
      if ((menu4curs + incre) < 0) {
      menu4curs = 4;  
      }
      else if ((menu4curs + incre) > 4){
      menu4curs = 0;  
      }
      else if ((menu4curs + incre) <= 4 && (menu4curs + incre) >= 0){
      menu4curs = menu4curs + incre;
      }
    }  
}

//Slider Move with Encoder Counter
void menu11counter(){
  if (incre != 0){
      menu11curs = menu11curs + (incre * menu14curs * 4);
  }
}

void menu12counter(){
  if (incre != 0){
      menu12curs = menu12curs + (incre * menu14curs);
  }
}

void menu13counter(){
  if (incre != 0){
      menu13curs = menu13curs + (incre * menu14curs);
  }
}
void menu16counter(){
  if (incre != 0){
      menu16curs = menu16curs + (incre * menu14curs * 0.2);
  }
}
void menu14counter(){
      if (incre != 0){
        if ((menu14curs + incre) < 0) {
      menu14curs = 0;  
      }
      else {
      menu14curs = menu14curs + (incre * move_multiplier);
      }
      lcdref = 1;  
      }
}


//Menu Cursor Logic

//Main Menu Curser Logic
void drawpg0(){
if (menu0curs == 0){
  u8g.firstPage();
  do {  
    draw0main0();
  } while( u8g.nextPage() );   
}
if (menu0curs == 1){
  u8g.firstPage();
  do {  
    draw0main1();
  } while( u8g.nextPage() );  
}
if (menu0curs == 2){
  u8g.firstPage();
  do {  
    draw0main2();
  } while( u8g.nextPage() );  
}
if (menu0curs == 3){
  u8g.firstPage();
  do {  
    draw0main3();
  } while( u8g.nextPage() );  
}
}

//Move Axis Menu Curser Logic
void drawpg1(){
if (menu1curs == 0){
  u8g.firstPage();
  do {  
    draw1main0();
  } while( u8g.nextPage() );   
}
if (menu1curs == 1){
  u8g.firstPage();
  do {  
    draw1main1();
  } while( u8g.nextPage() );  
}
if (menu1curs == 2){
  u8g.firstPage();
  do {  
    draw1main2();
  } while( u8g.nextPage() );  
}
if (menu1curs == 3){
  u8g.firstPage();
  do {  
    draw1main3();
  } while( u8g.nextPage() );  
}
if (menu1curs == 4){
  u8g.firstPage();
  do {  
    draw1main4();                       //is the follow focus
  } while( u8g.nextPage() ); 
}
if (menu1curs == 5){
  u8g.firstPage();
  do {  
    draw1main5();                       //is the set increments counter
  } while( u8g.nextPage() );   
}
}

//Set Position Menu Curser Logic
void drawpg2(){
if (menu2curs == 0){
  u8g.firstPage();
  do {  
    draw2main0();
  } while( u8g.nextPage() );   
}
if (menu2curs == 1){
  u8g.firstPage();
  do {  
    draw2main1();
  } while( u8g.nextPage() );  
}
if (menu2curs == 2){
  u8g.firstPage();
  do {  
    draw2main2();
  } while( u8g.nextPage() );  
}
if (menu2curs == 3){
  u8g.firstPage();
  do {  
    draw2main3();
  } while( u8g.nextPage() );  
}
}

//Set Time Menu Curser Logic
void drawpg3(){
  u8g.firstPage();
  do {  
    draw3main0();
  } while( u8g.nextPage() );   
}


//Start Motion Menu Curser Logic
void drawpg4(){
if (menu4curs == 0){
  u8g.firstPage();
  do {  
    draw4main0();
  } while( u8g.nextPage() );   
}
if (menu4curs == 1){
  u8g.firstPage();
  do {  
    draw4main1();
  } while( u8g.nextPage() );  
}
if (menu4curs == 2){
  u8g.firstPage();
  do {  
    draw4main2();
  } while( u8g.nextPage() );  
}
if (menu4curs == 3){
  u8g.firstPage();
  do {  
    draw4main3();
  } while( u8g.nextPage() );  
}
if (menu4curs == 4){
  u8g.firstPage();
  do {  
    draw4main4();
  } while( u8g.nextPage() );  
}
}

//Slider Moving Menu Curser Logic
void drawpg11(){
  u8g.firstPage();
  do {  
    draw11main0();
  } while( u8g.nextPage() );   
}

//Pan Moving Menu Curser Logic
void drawpg12(){
  u8g.firstPage();
  do {  
    draw12main0();
  } while( u8g.nextPage() );   
}

//Tilt Moving Menu Curser Logic
void drawpg13(){
  u8g.firstPage();
  do {  
    draw13main0();
  } while( u8g.nextPage() );   
}

//Follow Focus Moving Menu Curser Logic
void drawpg16(){
  u8g.firstPage();
  do {  
    draw16main0();
  } while( u8g.nextPage() );   
}

//Set Increments Menu Curser Logic
void drawpg14(){
  u8g.firstPage();
  do {   
    draw14main0();
  } while( u8g.nextPage() );   
}

void drawpg15(){
  u8g.firstPage();
  do { 
    draw15main0();
  } while( u8g.nextPage() );
}




//Menu Visual Design Depending on Menu

//Main Menu
void draw0main0(){
  u8g.drawBox( 0, 1, 128, 12); 
  u8g.setColorIndex(0);
  u8g.drawStr( 2, 12, "Move Axis");
  u8g.setColorIndex(1);
  u8g.drawStr( 2, 24, "Set Position");
  u8g.drawStr( 2, 36, "Set Speed");
  u8g.drawStr( 2, 48, "Start Motion");
  u8g.drawBox( 62, 50, 66, 12); 
  u8g.setColorIndex(0);  
  u8g.drawStr( 64, 60, "By Sanjaya");
  u8g.setColorIndex(1);    
}
void draw0main1(){
  u8g.drawStr( 2, 12, "Move Axis");
  u8g.drawBox( 0, 13, 128, 12); 
  u8g.setColorIndex(0);
  u8g.drawStr( 2, 24, "Set Position");
  u8g.setColorIndex(1);
  u8g.drawStr( 2, 36, "Set Speed");
  u8g.drawStr( 2, 48, "Start Motion");
  u8g.drawBox( 62, 50, 66, 12); 
  u8g.setColorIndex(0);  
  u8g.drawStr( 64, 60, "By Sanjaya");
  u8g.setColorIndex(1); 
}
void draw0main2(){
  u8g.drawStr( 2, 12, "Move Axis");
  u8g.drawStr( 2, 24, "Set Position");
  u8g.drawBox( 0, 25, 128, 12); 
  u8g.setColorIndex(0);
  u8g.drawStr( 2, 36, "Set Speed");
  u8g.setColorIndex(1);
  u8g.drawStr( 2, 48, "Start Motion");
  u8g.drawBox( 62, 50, 66, 12); 
  u8g.setColorIndex(0);  
  u8g.drawStr( 64, 60, "By Sanjaya");
  u8g.setColorIndex(1);  
}
void draw0main3(){
  u8g.drawStr( 2, 12, "Move Axis");
  u8g.drawStr( 2, 24, "Set Position");
  u8g.drawStr( 2, 36, "Set Speed");
  u8g.drawBox( 0, 37, 128, 12); 
  u8g.setColorIndex(0);
  u8g.drawStr( 2, 48, "Start Motion");
  u8g.setColorIndex(1);
  u8g.drawBox( 62, 50, 66, 12); 
  u8g.setColorIndex(0);  
  u8g.drawStr( 64, 60, "By Sanjaya");
  u8g.setColorIndex(1);  
}

//Move Axis Menu
void draw1main0(){
  u8g.drawBox( 0, 1, 128, 12); 
  u8g.setColorIndex(0);
  u8g.drawStr( 2, 12, "**RETURN**");
  u8g.setColorIndex(1);
  u8g.drawStr( 2, 24, "Slide");
  u8g.drawStr( 2, 36, "Pan");
  u8g.drawStr( 2, 48, "Tilt");
  u8g.drawStr( 2, 60, "Follow Focus");
     
}
void draw1main1(){
  u8g.drawStr( 2, 12, "**RETURN**");
  u8g.drawBox( 0, 13, 128, 12); 
  u8g.setColorIndex(0);
  u8g.drawStr( 2, 24, "Slide");
  u8g.setColorIndex(1);
  u8g.drawStr( 2, 36, "Pan");
  u8g.drawStr( 2, 48, "Tilt");
  u8g.drawStr( 2, 60, "Follow Focus");
}
void draw1main2(){
  u8g.drawStr( 2, 12, "**RETURN**");
  u8g.drawStr( 2, 24, "Slide");
  u8g.drawBox( 0, 25, 128, 12); 
  u8g.setColorIndex(0);
  u8g.drawStr( 2, 36, "Pan");
  u8g.setColorIndex(1);
  u8g.drawStr( 2, 48, "Tilt");
  u8g.drawStr( 2, 60, "Follow Focus");
  
}
void draw1main3(){
  u8g.drawStr( 2, 12, "**RETURN**");
  u8g.drawStr( 2, 24, "Slide");
  u8g.drawStr( 2, 36, "Pan");
  u8g.drawBox( 0, 37, 128, 12); 
  u8g.setColorIndex(0);
  u8g.drawStr( 2, 48, "Tilt");
  u8g.setColorIndex(1);
  u8g.drawStr( 2, 60, "Follow Focus");
}
void draw1main4(){
  u8g.drawStr( 2, 12, "**RETURN**");
  u8g.drawStr( 2, 24, "Slide");
  u8g.drawStr( 2, 36, "Pan");
  u8g.drawStr( 2, 48, "Tilt");
  u8g.drawBox( 0, 49, 128, 12); 
  u8g.setColorIndex(0);
  u8g.drawStr( 2, 60, "Follow Focus");
  u8g.setColorIndex(1);
}
void draw1main5(){
  u8g.drawBox( 0, 1, 128, 12); 
  u8g.setColorIndex(0);
  u8g.drawStr( 2, 12, "Set Increments");
  u8g.setColorIndex(1);
}

//Set Position
void draw2main0(){
  u8g.drawBox( 0, 1, 128, 12); 
  u8g.setColorIndex(0);
  u8g.drawStr( 2, 12, "**RETURN**");
  u8g.setColorIndex(1);
  u8g.drawStr( 2, 24, "Set Position 'A'");
  u8g.drawStr( 2, 36, "Set Position 'B'");
  u8g.drawStr( 2, 48, "Set Position 'C'");   
}
void draw2main1(){
  u8g.drawStr( 2, 12, "**RETURN**");
  u8g.drawBox( 0, 13, 128, 12); 
  u8g.setColorIndex(0);
  u8g.drawStr( 2, 24, "Set Position 'A'");
  u8g.setColorIndex(1);
  u8g.drawStr( 2, 36, "Set Position 'B'");
  u8g.drawStr( 2, 48, "Set Position 'C'");
}
void draw2main2(){
  u8g.drawStr( 2, 12, "**RETURN**");
  u8g.drawStr( 2, 24, "Set Position 'A'");
  u8g.drawBox( 0, 25, 128, 12); 
  u8g.setColorIndex(0);
  u8g.drawStr( 2, 36, "Set Position 'B'");
  u8g.setColorIndex(1);
  u8g.drawStr( 2, 48, "Set Position 'C'");
}
void draw2main3(){
  u8g.drawStr( 2, 12, "**RETURN**");
  u8g.drawStr( 2, 24, "Set Position 'A'");
  u8g.drawStr( 2, 36, "Set Position 'B'");
  u8g.drawBox( 0, 37, 128, 12); 
  u8g.setColorIndex(0);
  u8g.drawStr( 2, 48, "Set Position 'C'");
  u8g.setColorIndex(1);
}


//Set Time
void draw3main0(){
  u8g.drawStr( 2, 12, "Set Move Speed:");
  u8g.drawBox( 36, 13, 32, 12); 
  u8g.setColorIndex(0);
  char menu3buf[9];
  sprintf (menu3buf, "%d", menu3curs);
  u8g.drawStr( 37, 24, menu3buf);
  u8g.setColorIndex(1);
  u8g.drawStr( 70, 24, "Sec");
  u8g.drawStr( 12, 48, "Press OK to Save");
  Serial.print("Page 3 Printed"); 
}


//Start Motion
void draw4main0(){
  u8g.drawBox( 0, 1, 128, 12); 
  u8g.setColorIndex(0);
  u8g.drawStr( 2, 12, "**RETURN**");
  u8g.setColorIndex(1);
  u8g.drawStr( 2, 24, "A -> B");
  u8g.drawStr( 2, 36, "B -> A");
  u8g.drawStr( 2, 48, "A -> B -> C");
  u8g.drawStr( 2, 60, "A <> C LOOP x2");   
}
void draw4main1(){
  u8g.drawStr( 2, 12, "**RETURN**");
  u8g.drawBox( 0, 13, 128, 12); 
  u8g.setColorIndex(0);
  u8g.drawStr( 2, 24, "A -> B");
  u8g.setColorIndex(1);
  u8g.drawStr( 2, 36, "B -> A");
  u8g.drawStr( 2, 48, "A -> B -> C");
  u8g.drawStr( 2, 60, "A <> C LOOP x2");
}
void draw4main2(){
  u8g.drawStr( 2, 12, "**RETURN**");
  u8g.drawStr( 2, 24, "A -> B");
  u8g.drawBox( 0, 25, 128, 12); 
  u8g.setColorIndex(0);
  u8g.drawStr( 2, 36, "B -> A");
  u8g.setColorIndex(1);
  u8g.drawStr( 2, 48, "A -> B -> C");
  u8g.drawStr( 2, 60, "A <> C LOOP x2");
}
void draw4main3(){
  u8g.drawStr( 2, 12, "**RETURN**");
  u8g.drawStr( 2, 24, "A -> B");
  u8g.drawStr( 2, 36, "B -> A");
  u8g.drawBox( 0, 37, 128, 12); 
  u8g.setColorIndex(0);
  u8g.drawStr( 2, 48, "A -> B -> C");
  u8g.setColorIndex(1);  
  u8g.drawStr( 2, 60, "A <> C LOOP x2");
}
void draw4main4(){
  u8g.drawStr( 2, 12, "**RETURN**");
  u8g.drawStr( 2, 24, "A -> B");
  u8g.drawStr( 2, 36, "B -> A");
  u8g.drawStr( 2, 48, "A -> B -> C");
  u8g.drawBox( 0, 49, 128, 12); 
  u8g.setColorIndex(0);
  u8g.drawStr( 2, 60, "A <> C LOOP x2");
  u8g.setColorIndex(1);
}


//Slider Moving Page
void draw11main0(){
  u8g.drawStr( 24, 12, "Moving Slider");
  u8g.drawStr( 12, 48, "Press OK to RETURN");
}

//Slider Moving Page
void draw12main0(){
  u8g.drawStr( 36, 12, "Panning");
  u8g.drawStr( 12, 48, "Press OK to RETURN");
}

//Slider Moving Page
void draw13main0(){
  u8g.drawStr( 34, 12, "Tilting");
  u8g.drawStr( 12, 48, "Press OK to RETURN");
}

void draw16main0(){
  u8g.drawStr( 34, 12, "Adjusting Focus");
  u8g.drawStr( 12, 48, "Press OK to RETURN");
}

//Set Time
void draw14main0(){
  u8g.drawStr( 2, 12, "Motor Increments");
  u8g.drawBox( 36, 13, 32, 12); 
  u8g.setColorIndex(0);
  char menu14buf[9];
  sprintf (menu14buf, "%d", menu14curs);
  u8g.drawStr( 37, 24, menu14buf);
  u8g.setColorIndex(1);
  u8g.drawStr( 12, 48, "Press OK to Save");
}

//Video Motion in Progress Page
void draw15main0(){
  u8g.drawStr( 24, 12, "Motion in Progress");
  u8g.drawStr( 12, 48, "Press OK to RETURN");
}
