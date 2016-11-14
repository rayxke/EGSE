#include <Wire.h>
#include <SoftwareSerial.h>
#include <RTClib.h>
#include <Adafruit_MCP23017.h>

//EXPANDER PINS ONLY
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

  rtc.begin()
  
  mcp.begin();
  mcp.pinMode(EGSE_DISABLE_BUS, OUTPUT);
  mcp.pinMode(EGSE_RESET, OUTPUT);
  mcp.pinMode(ATTINY_PWR, OUTPUT);
  mcp.pinMode(HEART, INPUT);
  mcp.pinMode(SUNSENSE_DET, INPUT);
  mcp.pinMode(SW1_DET, INPUT);
  mcp.pinMode(SW2_DET, INPUT);
  mcp.pinMode(SW3_DET, INPUT);
  mcp.pinMode(BUZZER, OUTPUT);
  mcp.pinMode(IND_LED1, OUTPUT);
  mcp.pinMode(IND_LED2, OUTPUT);
  mcp.pinMode(IND_LED3, OUTPUT);
  mcp.pinMode(IND_LED4, OUTPUT);
  mcp.pinMode(IND_LED5, OUTPUT);
  mcp.pinMode(IND_LED6, OUTPUT);

  logger.begin(9600);                         //Serial for the data logger. Keenan has more info oh how this works.

  Serial.begin(9600);                         //Serial for the main computer. Commands will need to be processed?
  Serial.println("Pin config complete");
}

void loop() {
  DateTime now = rtc.now();                   //Updates the date stored in "now"

  delay(1000);
}
