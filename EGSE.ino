/*
    RECONSO Project
    EGSE Arduino Code
    Georgia Institute of Technology

    Notes:
            Use Ctrl+f "#TODO" to pick up on where I left off

*/

#include <Wire.h>
#include <SoftwareSerial.h>
#include "libraries/RealTimeClock/RTClib.h"
#include "libraries/PortExpander/Adafruit_MCP23017.h"

// Config Values:
#define VERSION 1.21       // Program Version
#define LED_REFRESH_RATE 2 // (Value in Hz)
#define ERROR_BLOCK false  // Defines whether an error connecting to the RTC or MCP blocks startup

#define MCP23017_ADDRESS 0x20
uint8_t i2caddr = 0;
//EXPANDER PINS ONLY

//#TODO: This is basically an enum. You may want to look into using one (This is fine too tho esp since the comments are helpful)
#define EGSE_DISABLE_BUS 1  // OUTPUT   Disables bus outputs on the Inhibit board. Prevents high level activation on the cube sat. Used for charging.
#define EGSE_RESET 2        // OUTPUT   Resets the enture inhibit system. Can be used to unlock the inhibit board if anything happens.
#define ATTINY_PWR 3        // OUTPUT   Activates the timer chip on the inhibit board. Can be used for programming or for skipping the inhibits to this point.
#define HEART 4             // INPUT    Heart input from the timer chip on the inhibit board. Also activates the "heart" light on the sat connector board.
#define SUNSENSE_DET 5      // INPUT    Raises high when the sunsensor is read as "active" and the inhibit board should be allowing activation.
#define SW1_DET 6           // INPUT    Activates when sep switch 1 is activated
#define SW2_DET 7           // INPUT    Activates when sep switch 2 is activated
#define SW3_DET 8           // INPUT    Activates when sep switch 3 is activated
#define BUZZER 9            // OUTPUT   Output buzzer. Raise high to alert the user.
#define IND_LED1 10         // OUTPUT   TBD
#define IND_LED2 11         // OUTPUT   TBD
#define IND_LED3 12         // OUTPUT   TBD
#define IND_LED4 13         // OUTPUT   TBD
#define IND_LED5 14         // OUTPUT   TBD
#define IND_LED6 15         // OUTPUT   TBD

//ARDUINO PINS ONLY
#define INTA 2              // INT      Configurable interrupt from the expander
#define INTB 3              // INT      Configurable interrupt from the expander
#define ENABLE 4            // I/O      Inhibit enable pin. If this pin is high, the sat is activated. Bring this pin high and hold to activate the sat manually. To disable, toggle EGSE_RESET
#define ARD_BATSELECT 5     // OUTPUT   Swaps between charge and discharge    CHECK THIS
#define ARD_MAINBAT 6       // OUTPUT   Enables SAT connection                CHECK THIS
#define ARD_CHARGE_DIS 7    // OUTPUT   Sets charge or discharge              CHECK THIS
#define IND_LED7 8          // OUTPUT   TBD
#define RX 9                // I/O      Serial communication with the data logger
#define TX 10               // I/O      Serial communication with the data logger
#define MOSI 11             // I/O      SPI interface with the timer chip on the inhibit board
#define MISO 12             // I/O      SPI interface with the timer chip on the inhibit board
#define SCK 13              // I/O      SPI interface with the timer chip on the inhibit board

#define I2C_SCL A5          // I/O      I2C interface with the expander, real time clock, and EPS charge board
#define I2C_SDA A4          // I/O      I2C interface with the expander, real time clock, and EPS charge board
#define IND_LED9 A3         // OUTPUT   TBD
#define IND_LED8 A2         // OUTPUT   TBD
#define CHARGE A1           // INPUT    ANALOG
#define VBAT A0             // INPUT    ANALOG

// Global Defs:
SoftwareSerial logger(RX,TX);
Adafruit_MCP23017 mcp;
RTC_DS3231 rtc;


// Command Defs:
String inputString = "";
String temp = "";
String commandWord = "";
bool stringComplete = false;
bool done = false;
byte pos = 0;
byte newPos = 0;

double RA = 10000.0;
double RB = 10000.0;
double voltScale = 0.009765625;//(((1024 / (5))^-1) * 2)

// Enum Defs:
enum State {NO_STATE, CHARGE_STATE, DISCHARGE_STATE, INHIBIT_STATE, OFF_STATE, ERROR_STATE}; // Enum for continuing state
enum Led_State {LED_OFF, LED_ON, LED_BLINK};



State state = NO_STATE;
Led_State led1_state = LED_BLINK;
Led_State led2_state = LED_OFF;
Led_State led3_state = LED_OFF;
Led_State led4_state = LED_OFF;
Led_State led5_state = LED_OFF;
Led_State led6_state = LED_OFF;
Led_State led7_state = LED_OFF;
Led_State led8_state = LED_OFF;

// Other Defs:
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
bool error = false;
bool blink = 0; //Blink state
bool lampTest = 0;

void setup() {



  Serial.begin(9600);                         //Serial for the main computer. Commands will need to be processed?
  serialWelcome();
  Serial.println(F("\nEGSE Init.."));
  inputString.reserve(200);
  temp.reserve(25);
  commandWord.reserve(25);

  attachInterrupt(digitalPinToInterrupt(2),buttonDetect,FALLING);

  pinMode(INTA,INPUT);
  pinMode(INTB,INPUT);
  pinMode(ENABLE,OUTPUT);
  pinMode(ARD_MAINBAT,OUTPUT);
  pinMode(ARD_CHARGE_DIS,OUTPUT);
  pinMode(ARD_BATSELECT,OUTPUT);
  pinMode(IND_LED7,OUTPUT);
  pinMode(IND_LED9,OUTPUT);
  pinMode(IND_LED8,OUTPUT);
  pinMode(CHARGE,INPUT);
  pinMode(VBAT,INPUT);

  safeMode();

  Serial.println(F("> Native pin config complete"));

  //TODO: Check if this should actually be blocking or not
  if (! rtc.begin()) {
    Serial.println(F(">> RTC ERROR - CHECK WIRING"));
    while (ERROR_BLOCK);
  }

  //TODO: USED UNTIL BATTERY IS SOLDERED ON
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  Serial.println(F("> RTC Init complete"));

  mcp.begin();

  // We mirror INTA and INTB, so that only one line is required between MCP and Arduino for int reporting
  // The INTA/B will not be Floating
  // INTs will be signaled with a LOW
  mcp.setupInterrupts(true,false,LOW);

  Serial.println("> MCP Init complete");




  mcp.pinMode(HEART, INPUT);
  mcp.pinMode(SUNSENSE_DET, INPUT);
  mcp.pinMode(SW1_DET, INPUT); //Charge Switch
  mcp.pinMode(SW2_DET, INPUT);  // Discharge Switch
  mcp.pinMode(SW3_DET, INPUT); // Inhibit Switch

  /*mcp.pullUp(SW1_DET, HIGH);
  mcp.pullUp(SW2_DET, HIGH);
  mcp.pullUp(SW3_DET, HIGH);*/

  /*mcp.setupInterruptPin(SW1_DET,FALLING);
  mcp.setupInterruptPin(SW2_DET,FALLING);
  mcp.setupInterruptPin(SW3_DET,FALLING);*/


  mcp.pinMode(EGSE_DISABLE_BUS, OUTPUT);
  mcp.pinMode(EGSE_RESET, OUTPUT);
  mcp.pinMode(ATTINY_PWR, OUTPUT);
  mcp.pinMode(BUZZER, OUTPUT);
  mcp.pinMode(IND_LED1, OUTPUT); //State 1 LED
  mcp.pinMode(IND_LED2, OUTPUT); //State 2 LED
  mcp.pinMode(IND_LED3, OUTPUT); //State 3 LED
  mcp.pinMode(IND_LED4, OUTPUT);
  mcp.pinMode(IND_LED5, OUTPUT);
  mcp.pinMode(IND_LED6, OUTPUT);

  Serial.println("> MCP pin config complete");

  logger.begin(9600);                         //Serial for the data logger. Keenan has more info oh how this works.
  Serial.println(F("> LOGGER INIT complete"));

  Serial.print(F("> Voltage: "));
  Serial.println(analogRead(VBAT) * voltScale);

  Serial.println(F("> EGSE INIT complete\n"));
  commandReady();
  //mcp.digitalWrite(10,HIGH);
  //delay(1000);
  mcp.digitalWrite(10,LOW);

  delay(500);

  initWatchdog();

  //uint16_t val = mcp.readGPIOAB();

  //Serial.println(val);


}

void initWatchdog(){
  cli();
  //set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register
  OCR1A = ((16000000.0) / (LED_REFRESH_RATE*1024) - 1);// (must be <65536) (x in Hz)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12 and CS10 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei();
}


//Turn LEDs on or off or blink
/*#TODO:Make sure pins are being passed correctly
*/
ISR(TIMER1_COMPA_vect){
  //noInterrupts();
  //interrupts();
  assignLED(led7_state, IND_LED7, false);
}

bool intTrigger = false;
void buttonDetect(){
  //led7_state = LED_BLINK;
  //uint8_t pin=mcp.getLastInterruptPin();
  //uint8_t val=mcp.getLastInterruptPinValue();
  //Serial.println(pin);
  //Serial.println(val);
  intTrigger = true;
}

void cleanInterrupts(){
  EIFR=0x01;
}

void selfCheck(){
  //Check and set State
}

//blink = !blink;
void assignLED(Led_State led_state, int my_pin, bool MCP){

    if(lampTest){
      led_state = LED_ON;
    }

    switch (led_state){
      case LED_OFF:
        if(MCP){
          mcp.digitalWrite(my_pin, LOW);
        }else{
          digitalWrite(my_pin, LOW);
        }
        break;
      case LED_ON:
        if(MCP){
          mcp.digitalWrite(my_pin, HIGH);
        }else{
          digitalWrite(my_pin, HIGH);
        }
        break;
      case LED_BLINK:

        if(blink){
          if(MCP){
            mcp.digitalWrite(my_pin, HIGH);
          }else{
            digitalWrite(my_pin, HIGH);
          }
        }else{
          if(MCP){
            mcp.digitalWrite(my_pin, LOW);
          }else{
            digitalWrite(my_pin, LOW);
          }
        }
        break;
    }
}

void serialWelcome(){
  Serial.print(F("Build Information:\n  Version: "));
  Serial.print(VERSION);
  Serial.print(F("\n  Compile Date: "));
  Serial.print(__DATE__);
  Serial.print(" ");
  Serial.print(__TIME__);
  Serial.print(F("\n  Compile File: "));
  Serial.println(__FILE__);
  Serial.println(F("\nAccess help by typing \'help\'"));
}

void commandReady(){
  Serial.println(F("Ready for Commands."));
  Serial.print(F("> "));
}



void test(){
  /*Serial.println("Relay Test:");
  delay(500);
  Serial.println("Relay MainBat:");
  digitalWrite(ARD_MAINBAT,HIGH);
  digitalWrite(ARD_BATSELECT,LOW);
  digitalWrite(ARD_CHARGE_DIS,LOW);
  delay(3000);
  Serial.println("Relay Bat Select:");
  digitalWrite(ARD_MAINBAT,LOW);
  digitalWrite(ARD_BATSELECT,HIGH);
  digitalWrite(ARD_CHARGE_DIS,LOW);
  delay(3000);
  Serial.println("Relay Charge/Discharge:");
  digitalWrite(ARD_MAINBAT,LOW);
  digitalWrite(ARD_BATSELECT,LOW);
  digitalWrite(ARD_CHARGE_DIS,HIGH);
  delay(3000);
  digitalWrite(ARD_MAINBAT,LOW);
  digitalWrite(ARD_BATSELECT,LOW);
  digitalWrite(ARD_CHARGE_DIS,LOW);
  Serial.println("Relay Test Done:");*/

  digitalWrite(ARD_MAINBAT,LOW);
  digitalWrite(ARD_BATSELECT,LOW);
  digitalWrite(ARD_CHARGE_DIS,HIGH);

  delay(500);

  digitalWrite(ARD_MAINBAT,LOW);
  digitalWrite(ARD_BATSELECT,LOW);
  digitalWrite(ARD_CHARGE_DIS,LOW);

  powerSafe();

}


bool toggles[] = {true,true,true};
byte arrNum[] = {ARD_MAINBAT,ARD_CHARGE_DIS,ARD_BATSELECT};
void toggle(byte in){
  Serial.print("Toggle: ");
  if (in == 0){
    Serial.print("MainBat - ");
  }else if (in == 1){
    Serial.print("Charge/Discharge - ");
  }else if (in == 2){
    Serial.print("BatSelect - ");
  }else{
    Serial.println("ERR");
    return;
  }
  if(toggles[in]){
    digitalWrite(arrNum[in],HIGH);
    Serial.println("ON");
  }else{
    digitalWrite(arrNum[in],LOW);
    Serial.println("OFF");
  }
  toggles[in] = !toggles[in];
}



void Thread1()
{

    //Now Begin States
    switch(state)
    {
        case CHARGE_STATE:
        {
            contCharging();
            break;
        }
        case DISCHARGE_STATE:
        {
            contDischarging();
            break;
        }
        case OFF_STATE:
        {
            offState();
            break;
        }
        case ERROR_STATE:
        {
            errorCont();
            break;
        }
    }

}
void Thread2()
{
    //I2C Stuff and Serial Interface
}
//Monitors Buttons, Switches, LEDS, and Buzzer
void Thread3()
{
    //Buttons to change State
    int charge, discharge, inhibit; //Variables for beginning a state
    charge    = digitalRead(SW1_DET);
    discharge = digitalRead(SW2_DET);
    inhibit   = digitalRead(SW3_DET);

    //Assign Light Values
    //#TODO:This can be turned into a for loop
    /*assignLED(led1_state, IND_LED1);
    assignLED(led2_state, IND_LED2);
    assignLED(led3_state, IND_LED3);
    assignLED(led4_state, IND_LED4);
    assignLED(led5_state, IND_LED5);
    assignLED(led6_state, IND_LED6);*/

    //#TODO: No idea what buzzer should do...

}


//Main loop of the fucntion
void loop() {
  /*DateTime now = rtc.now();                   //Updates the date stored in "now"

  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();*/



  if (stringComplete) {
    //Serial.println(inputString);
    // clear the string:

    pos = 0;
    newPos = 0;
    done = false;
    inputString.trim();
    Serial.println(inputString);
    commandTree();
    Serial.print("\n> ");

    inputString = "";
    stringComplete = false;
  }

  if(intTrigger){
    uint8_t pin = mcp.getLastInterruptPin();
    Serial.println(pin);

    intTrigger = false;
  }
}
