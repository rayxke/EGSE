/*
    RECONSO Project
    EGSE Arduino Code
    Georgia Institute of Technology

    Notes:
      ModeControl: Functions related to controlling the states are placed here

*/

void safeMode(){
  digitalWrite(ARD_MAINBAT,HIGH);
  digitalWrite(ARD_BATSELECT,LOW);
  digitalWrite(ARD_CHARGE_DIS,LOW);
  Serial.println("> ENTERED SAFEMODE");
}

void powerSafe(){
  digitalWrite(ARD_MAINBAT,LOW);
  digitalWrite(ARD_BATSELECT,LOW);
  digitalWrite(ARD_CHARGE_DIS,LOW);
  Serial.println("> Deactivate Relays");
}

void beginCharging()
{
    mcp.digitalWrite(EGSE_RESET, HIGH);
    mcp.digitalWrite(EGSE_DISABLE_BUS, HIGH);
    delay(500);
    mcp.digitalWrite(EGSE_RESET, LOW);
    digitalWrite(ENABLE, HIGH);
    digitalWrite(ARD_MAINBAT, HIGH);
    digitalWrite(ARD_BATSELECT, HIGH);
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
    digitalWrite(ARD_BATSELECT, LOW);
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
    digitalWrite(ARD_BATSELECT, HIGH);
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
    digitalWrite(ARD_CHARGE_DIS, LOW);
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
    digitalWrite(ARD_BATSELECT, LOW);
    digitalWrite(ARD_CHARGE_DIS, LOW);

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
