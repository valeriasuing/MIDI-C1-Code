// VALERIA SUING - C1 MIDI CONTROLLER
// Atelier 2 
// Code used for C1 MIDI Controller, MIDI messages were later mapped 
// in Ableton Live 

// This code takes MIDI notes for 12 buttons, Octave Shifts for two buttons,
// CC messages for 2 more buttons, and 4 CC messages with different values 
// for four potentiometers 

// Code referenced Gustavo Silveira's code https://github.com/silveirago/Fliper-2/blob/master/Code/Fliper/Fliper.ino
// as well as Liam Lacey's code https://ask.video/article/audio-hardware/how-to-build-a-simple-diy-usb-midi-controller-using-teensy


#define DEBUG 1 //allows to check serial 
#include <Bounce.h> //library used in keyboard buttons

int LED = 35; // Pin 35 connected to LED
const int KEYBOARD_BUTTONS = 12; //number of push buttons for MIDI keyboard 
const int EFFECT_BUTTONS = 2; //number of effect buttons 
const int EFFECT_PINS[EFFECT_BUTTONS] = {26,27}; // PINS for effect buttons

int EFFECT_CS[EFFECT_BUTTONS] = {}; //Current state of effect button
int EFFECT_PS[EFFECT_BUTTONS] = {}; //Previous state of effect button 

//debounce 
unsigned long LAST_DEBOUNCE_TIME[EFFECT_BUTTONS] = {0}; 
unsigned long DEBOUNCE_DELAY = 50; 
const int DEBOUNCE_TIME = 5;

const int POT_NUMBERS = 4; //number of Potentiometers
const int POT_PINS[POT_NUMBERS] = {A6, A7, A8, A9}; //analog pins connected to potentiometers
int POT_CS[POT_NUMBERS] = {0}; //current state of potentiometer 
int POT_PS[POT_NUMBERS] = {0}; //Previous state of potentiometer
int POT_VAR = 0; //Difference between the current and previous state of the potentiometer
int MIDI_CS[POT_NUMBERS] = {0}; //Current state of the MIDI value 
int MIDI_PS[POT_NUMBERS] = {0}; //Previous state of the MIDI value 

const int TIMEOUT = 300; //Amount of time the potentiometer will be read after it exceeds threshold 
const int VAR_THRESHOLD = 10; //Threshold for the potentiometer signal variation 
boolean POT_MOVING  = true;
unsigned long POT_TIME[POT_NUMBERS] = {0}; //Previously stored time 
unsigned long TIMER[POT_NUMBERS] = {0}; //Stores the time that has elapsed since timer was reset 


const int MIDI_CHAN = 1; //MIDI channel 
const int MIDI_CC_VAL = 127; //CC value
//MIDI CC
byte CC = 1; //for potentiometers
byte CC_BUTTONS = 20; //for buttons

int OCTAVE_SHIFT = 0; //variable to change octave 
//PINS connected to ocatve buttons
const int OCTAVE_UP_BUTTON = 24;
const int OCTAVE_DOWN_BUTTON = 25;

//Bounce Object (deals with contact chatter) 
Bounce buttons[KEYBOARD_BUTTONS] = 
{
  Bounce (2, DEBOUNCE_TIME),
  Bounce (3, DEBOUNCE_TIME),
  Bounce (4, DEBOUNCE_TIME),
  Bounce (5, DEBOUNCE_TIME),
  Bounce (6, DEBOUNCE_TIME),
  Bounce (7, DEBOUNCE_TIME),
  Bounce (8, DEBOUNCE_TIME),
  Bounce (9, DEBOUNCE_TIME),
  Bounce (10, DEBOUNCE_TIME),
  Bounce (11, DEBOUNCE_TIME),
  Bounce (12, DEBOUNCE_TIME),
  Bounce (13, DEBOUNCE_TIME),
};


//Array that stores the exact note each button will need
  const int MIDI_NOTE_NUMS[KEYBOARD_BUTTONS] = {60,61,62,63,64,65,66,67,68,69,70,71};
//velocity  
  const int MIDI_NOTE_VELS[KEYBOARD_BUTTONS] = {110,110,110,110,110,110,110,110,110,110,110,110};
  

void setup() {
   Serial.begin(31250);
  //setting keyboard buttons as inputs with pullup resistors
  for (int i = 0; i < KEYBOARD_BUTTONS + 1; i++)
  {   
    pinMode (i, INPUT_PULLUP);
  }
  //Pullup resistors for effect buttons
  pinMode(26, INPUT_PULLUP);
  pinMode(28, INPUT_PULLUP);
  pinMode (13, INPUT_PULLUP);
  //Pullup resistors for octave change buttons
  pinMode(OCTAVE_UP_BUTTON, INPUT_PULLUP);
  pinMode(OCTAVE_DOWN_BUTTON, INPUT_PULLUP);
  //LED as an Output
  pinMode(LED, OUTPUT);

}

void loop() {

   //turn on LED 
   digitalWrite(LED, HIGH); 
    
  //update buttons, no delays
  for (int i = 0; i < KEYBOARD_BUTTONS; i++)
  {
            buttons[i].update();
  }
  //check status of buttons 
  for (int i = 0; i < KEYBOARD_BUTTONS; i++)
  {
      if (digitalRead(OCTAVE_UP_BUTTON) == LOW){
       OCTAVE_SHIFT = 12; //if upper octave shift button is pressed, change octave shift vriable to 12
    } else if (digitalRead(OCTAVE_DOWN_BUTTON) == LOW){
     OCTAVE_SHIFT = -12; //if lower octave shift button is pressed, change octave shift vriable to -12
      } else {
        OCTAVE_SHIFT = 0; //else keep octave shift as 0 
        }
    //Falling Edge - falling = high (not pressed - voltage from pullup resistor) to low (pressed - button connects pin to ground) 
    if (buttons[i].fallingEdge())
    {
      usbMIDI.sendNoteOn(MIDI_NOTE_NUMS[i] + OCTAVE_SHIFT, 110, MIDI_CHAN); //send (note + octave shift, velocity, channel) 
    }
    //Rising = low (pressed) to high (not pressed)
    else if (buttons[i].risingEdge()) 
    {
      usbMIDI.sendNoteOff (MIDI_NOTE_NUMS[i] + OCTAVE_SHIFT, 110, MIDI_CHAN); //send not off (note + octave shift, velocity, channel) 
    }
  }

  //for all effect buttons
  for (int i = 0; i < EFFECT_BUTTONS; i++) { 
  
      EFFECT_CS[i] = digitalRead(EFFECT_PINS[i]);  // read pins from arduino
  
      if ((millis() - LAST_DEBOUNCE_TIME[i]) > DEBOUNCE_DELAY) {
  
        if (EFFECT_PS[i] != EFFECT_CS[i]) {
          LAST_DEBOUNCE_TIME[i] = millis();
  
          if (EFFECT_PS[i] == LOW) {
  
            // Sends MIDI CC number
             usbMIDI.sendControlChange(CC_BUTTONS + i, 127, MIDI_CHAN); 
          } 
          EFFECT_PS[i] = EFFECT_CS[i]; //resets
        }
      }
    }      

  
  
  for (int i = 0; i < POT_NUMBERS; i++) { // Loops through all potentiometers

    POT_CS[i] = analogRead(POT_PINS[i]); // reads the pins from arduino

    MIDI_CS[i] = map(POT_CS[i], 0, 1023, 0, 127); // Maps the reading of the potCState to a value usable in midi

    POT_VAR = abs(POT_CS[i] - POT_PS[i]); // Calculates the absolute value between the difference between the current and previous state of the pot

    if (POT_VAR > VAR_THRESHOLD) { // Opens the gate if the potentiometer variation is greater than the threshold
      POT_TIME[i] = millis(); // Stores the previous time
    }

    TIMER[i] = millis() - POT_TIME[i]; // Resets the timer 11000 - 11000 = 0ms

    if (TIMER[i] < TIMEOUT) { // If the timer is less than the maximum allowed time it means that the potentiometer is still moving
      POT_MOVING = true;
    }
    else {
      POT_MOVING = false;
    }

    if (POT_MOVING == true) { // If the potentiometer is still moving, send the change control
      if (MIDI_PS[i] != MIDI_CS[i]) {

        // Sends  MIDI CC 
        usbMIDI.sendControlChange(CC + i, MIDI_CS[i], MIDI_CHAN); 

        POT_PS[i] = POT_CS[i]; // Stores the current reading of the potentiometer to compare with the next
        MIDI_PS[i] = MIDI_CS[i];
      }
    }
  }  

  //MIDI controllers should discard incoming MIDI messages 
  while (usbMIDI.read()){
    //ignore incoming messages
  }
 }
