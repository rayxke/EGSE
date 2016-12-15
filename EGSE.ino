/*
    RECONSO Project
    EGSE Arduino Code
    Georgia Institute of Technology
    
    Notes:  
            Use Ctrl+f "#TODO" to pick up on where I left off

*/

#include <Wire.h>
#include <SoftwareSerial.h>
#include <RTClib.h>
#include <Adafruit_MCP23017.h>

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
#define ARD_MAINBAT 5       // OUTPUT   Enables the battery connection    CHECK THIS
#define ARD_DISCHARGE 6     // OUTPUT   Activated discharging             CHECK THIS
#define ARD_CHARGESELECT 7  // OUTPUT   Selects the charge state          CHECK THIS
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

SoftwareSerial logger(RX,TX);
Adafruit_MCP23017 mcp;
RTC_DS3231 rtc;

//Obviously I like enums a little too much....
enum State{NO_STATE, CHARGE_STATE, DISCHARGE_STATE, INHIBIT_STATE, OFF_STATE, ERROR_STATE}; // Enum for continuing state
enum Led_State { LED_OFF, LED_ON, LED_BLINK};

State state = NO_STATE;
Led_State led1_state = LED_OFF;
Led_State led2_state = LED_OFF;
Led_State led3_state = LED_OFF;
Led_State led4_state = LED_OFF;
Led_State led5_state = LED_OFF;
Led_State led6_state = LED_OFF;
bool error = false;

void setup() {
  
  pinMode(INTA,INPUT);
  pinMode(INTB,INPUT);
  pinMode(ENABLE,OUTPUT);
  pinMode(ARD_MAINBAT,OUTPUT);
  pinMode(ARD_DISCHARGE,OUTPUT);
  pinMode(ARD_CHARGESELECT,OUTPUT);
  pinMode(IND_LED7,OUTPUT); 
  pinMode(IND_LED9,OUTPUT); 
  pinMode(IND_LED8,OUTPUT); 
  pinMode(CHARGE,INPUT);
  pinMode(VBAT,INPUT);

  rtc.begin();
  
  mcp.begin();
  mcp.pinMode(EGSE_DISABLE_BUS, OUTPUT);
  mcp.pinMode(EGSE_RESET, OUTPUT);
  mcp.pinMode(ATTINY_PWR, OUTPUT);
  mcp.pinMode(HEART, INPUT);
  mcp.pinMode(SUNSENSE_DET, INPUT);
  mcp.pinMode(SW1_DET, INPUT); //Charge Switch
  mcp.pinMode(SW2_DET, INPUT);  // Discharge Switch
  mcp.pinMode(SW3_DET, INPUT); // Inhibit Switch
  mcp.pinMode(BUZZER, OUTPUT);
  mcp.pinMode(IND_LED1, OUTPUT); //State 1 LED
  mcp.pinMode(IND_LED2, OUTPUT); //State 2 LED
  mcp.pinMode(IND_LED3, OUTPUT); //State 3 LED
  mcp.pinMode(IND_LED4, OUTPUT);
  mcp.pinMode(IND_LED5, OUTPUT);
  mcp.pinMode(IND_LED6, OUTPUT);

  logger.begin(9600);                         //Serial for the data logger. Keenan has more info oh how this works.

  Serial.begin(9600);                         //Serial for the main computer. Commands will need to be processed?
  Serial.println("Pin config complete");
}





void beginCharging()
{
    mcp.digitalWrite(EGSE_RESET, HIGH);
    mcp.digitalWrite(EGSE_DISABLE_BUS, HIGH);
    delay(500);
    mcp.digitalWrite(EGSE_RESET, LOW);
    digitalWrite(ENABLE, HIGH);
    digitalWrite(ARD_MAINBAT, HIGH);
    digitalWrite(ARD_CHARGESELECT, HIGH);
    digitalWrite(ENABLE, HIGH);
    digitalWrite(IND_LED2, HIGH);
}


//#TODO: Add I2C EPS Check Voltage
void contCharging()
{
    //I2C to EPS check Voltage
    //If charging complete
    bool comp_charg = false;
    if (comp_charg)
    {
        led2_state = LED_BLINK;
        led3_state = LED_ON;
    }
}

void exitCharging()
{
    bool error = 0; //Will probably need to make global variable
    digitalWrite(ARD_MAINBAT, LOW);
    digitalWrite(ARD_CHARGESELECT, LOW);
    mcp.digitalWrite(EGSE_RESET, HIGH);
    mcp.digitalWrite(EGSE_DISABLE_BUS, LOW);
    //delay(500);
    digitalWrite(ENABLE, LOW);
    mcp.digitalWrite(EGSE_RESET, LOW);
    digitalWrite(IND_LED2, LOW);
    digitalWrite(IND_LED3, LOW);
    digitalWrite(IND_LED1, HIGH);
    if (error)
    {
        led4_state = LED_BLINK;
        state = ERROR_STATE;
        return;
    }
    state = OFF_STATE;
    
}

void beginDischarging()
{
    mcp.digitalWrite(EGSE_RESET, HIGH);
    mcp.digitalWrite(EGSE_DISABLE_BUS, HIGH);
    delay(500);
    mcp.digitalWrite(EGSE_RESET, LOW);
    digitalWrite(ENABLE, HIGH);
    digitalWrite(ARD_MAINBAT, HIGH);
    digitalWrite(ARD_CHARGESELECT, HIGH);
    digitalWrite(IND_LED4, HIGH);
    
}


//#TODO: Add I2C to EPS Voltage Check
void contDischarging()
{
    //I2C to EPS check Voltage
    //If charging complete
    bool comp_dis = false, error = false;
    if (comp_dis)
    {
        led2_state = LED_BLINK;
        led3_state = LED_ON;
    }
    else if (error)
    {
        state = ERROR_STATE;
        return;
    }
    else
    {
        //Nothing here...
        //There should probs be a default here not sure what that should be tho.
    }
}


void exitDischarging()
{
    bool error = 0; //Will probably need to make global variable
    digitalWrite(ARD_MAINBAT, LOW);
    digitalWrite(ARD_DISCHARGE, LOW);
    mcp.digitalWrite(EGSE_RESET, HIGH);
    mcp.digitalWrite(EGSE_DISABLE_BUS, LOW);
    //delay(500);
    digitalWrite(ENABLE, LOW);
    mcp.digitalWrite(EGSE_RESET, LOW);
    
    led4_state = LED_OFF;
    led5_state = LED_OFF;
    led1_state = LED_ON;
    
    
    if (error)
    {
        led4_state = LED_BLINK;
        state = ERROR_STATE;
        return;
    }
    state = OFF_STATE;
    
}

//This is the Error/ Monitor State
void errorState()
{
    if (!error)
    {
        led1_state = LED_OFF;
        led6_state = LED_ON;
        digitalWrite(ARD_MAINBAT, HIGH);
    }
    else
    {
        led6_state = LED_BLINK;
    }
}


/*#TODO: See the code doc on the EGSE drive
         Need to flesh out this functionality
         Also this state may be useless... idk
*/
void errorCont()
{
    if (!error)
    {
       //Send serial data to the computer
       Serial.println("Error Test Data"); //#TODO: Will need to implement test
    }
    else
    {
        //Print out error over serial
        led6_state = LED_BLINK;
        //Add a wait for button presses
    }
}


//TODO: May need to add more leds
void offState()
{
    digitalWrite(ENABLE, LOW);
    mcp.digitalWrite(EGSE_RESET, LOW);
    mcp.digitalWrite(EGSE_DISABLE_BUS, LOW);
    digitalWrite(ARD_MAINBAT, LOW);
    digitalWrite(ARD_CHARGESELECT, LOW);
    digitalWrite(ARD_DISCHARGE, LOW);
    
    //#TODO: Change to for loop
    led1_state = LED_OFF;
    led2_state = LED_OFF;
    led3_state = LED_OFF;
    led4_state = LED_OFF;
    led5_state = LED_OFF;
    led6_state = LED_OFF;
    
    led1_state = LED_ON;
    
    //#TODO:Include while loop for button presses?
}

//Turn LEDS on or off or blink
/*#TODO:Make sure pins are being passed correctly
*/

void assignLED(Led_State led_state, int my_pin)
{
    switch (led_state)
    {
        case LED_OFF:
        {
            digitalWrite(my_pin, LOW);
            break;
        }
        case LED_ON:
        {
            digitalWrite(my_pin, HIGH);
            break;
        }
        case LED_BLINK:
        {
            digitalWrite(my_pin, HIGH);
            delay(1000);
            digitalWrite(my_pin, LOW);
            delay(1000);
            break;
        }
    }
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
    assignLED(led1_state, IND_LED1);
    assignLED(led2_state, IND_LED2);
    assignLED(led3_state, IND_LED3);
    assignLED(led4_state, IND_LED4);
    assignLED(led5_state, IND_LED5);
    assignLED(led6_state, IND_LED6);
    
    //#TODO: No idea what buzzer should do...
    
}

//Main loop of the fucntion
void loop() {
  DateTime now = rtc.now();                   //Updates the date stored in "now"
  

  delay(1000);
}
