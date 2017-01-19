/*
    RECONSO Project
    EGSE Arduino Code
    Georgia Institute of Technology

    Notes:
      Commands: Functions related to commands are placed here

*/



/*
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
String newLine = "\n";
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    if(newLine == "\n\r" && inChar == '\177'){
      inputString2 = inputString;
      inputString = inputString2.substring(0,inputString2.length()-1);
      //Serial.write("\025");
      /*Serial.write(025);   //Print "esc"
      Serial.print(F("[2J"));

      Serial.write(025);   //Print "esc"
      Serial.print(F("[2K"));*/
      Serial.print(F("                                  "));
      Serial.print(F("\r> "));
      Serial.print(inputString);
      continue;
    }else{
      inputString += inChar;
    }

    if(SERIAL_ECHO && inChar != '\r'){
      Serial.print(inChar);
    }
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n'){
      newLine = "\n";
      stringComplete = true;
    }else if(inChar == '\r'){
      newLine = "\n\r";
      stringComplete = true;
    }
  }
}

String getNext(){
  newPos = inputString.indexOf(" ",pos+1);
  if(newPos == 255){
    done = true;
  }
  temp = inputString.substring(pos,newPos);
  pos = newPos;
  temp.trim();
  return temp;
}

void status(){
  Serial.println(F("---Status---"));
  Serial.print(F("> Battery Voltage: "));
  Serial.println(analogRead(VBAT) * voltScale);
}

void passthrough(){
  Serial.println(F("Passthrough to Logger. Type \'~\' to exit."));
  while (true){
    if (logger.available()) {
      Serial.write(logger.read());
    }
    if (Serial.available()) {
      char in = Serial.read();
      if(in == '~'){
        break;
      }
      logger.write(in);
    }
  }
  Serial.read();
  Serial.flush();
}

void commandTree(){
  commandWord = getNext();
  //Serial.println(commandWord);
  if (commandWord == "help" || commandWord == "-h"){
    Serial.print("---HELP---" + newLine + "> \'help\' | \'-h\'" + newLine + "Issues this command." + newLine);
    Serial.print("> \'?\' | \'status\'" + newLine + "Gets system status." + newLine);
    Serial.print("> \'set\'" + newLine + "Used for setting configurable items." + newLine);
    Serial.print("> \'safe\'" + newLine + "Sets mode directly to safemode." + newLine);
    Serial.print("> \'time\' | \'rtc\'" + newLine + "Prints current set time." + newLine);
    Serial.print("> \'lamptest\'" + newLine + "Tests the output LEDs. Will all activate for a short time." + newLine);
    Serial.print("> \'test\'" + newLine + "Runs the test function" + newLine);
  }else if (commandWord == "set"){
    if(done){
      // TODO: Write a help function for the set command
      Serial.println(F(">> ERR: Configurable item expected."));
      Serial.println(F(">> Options: mode, log"));
    }else{
      commandWord = getNext();
      if(commandWord == "mode"){
        if(done){
          Serial.println(F(">> ERR: mode type expected."));
          Serial.println(F(">> Options: charge, discharge, monitor, safe"));
        }else{
          commandWord = getNext();
          if(commandWord == "charge"){
            Serial.print(F("Set mode to"));
            Serial.println(F("\'charge\'"));
          }else if(commandWord == "discharge"){
            Serial.print(F("Set mode to"));
            Serial.println(F("\'discharge\'"));
          }else if(commandWord == "monitor"){
            Serial.print(F("Set mode to"));
            Serial.println(F("\'monitor\'"));
          }else if(commandWord == "safe"){
            Serial.print(F("Set mode to"));
            Serial.println(F("\'safe\'"));
          }else{
            Serial.print(F(">> ERR: Unknown mode type: set mode "));
            Serial.println(commandWord);
            Serial.println(F(">> Options: charge, discharge, monitor, safe"));
          }
        }
      }else if(commandWord == "relay"){
        if(done){
          Serial.println(F(">> ERR: mode type expected."));
          Serial.println(F(">> Options: main, c/d, bat"));
        }else{
          commandWord = getNext();
          if(commandWord == "main"){
            toggle(0);
          }else if(commandWord == "c/d"){
            toggle(1);
          }else if(commandWord == "bat"){
            toggle(2);
          }else{
            Serial.print(F(">> ERR: Unknown mode type: set relay "));
            Serial.println(commandWord);
            Serial.println(F(">> Options: main, c/d, bat"));
          }
        }
      }else if(commandWord == "log"){

      }else{
          Serial.print(F(">> ERR: Unknown set command: set "));
          Serial.println(commandWord);
          Serial.println(F(">> Options: mode, log"));
      }
    }
  }else if (commandWord == "disable"){
    powerSafe();
  }else if (commandWord == "test"){
    test();
  }else if (commandWord == "serial"){
    passthrough();
  }else if (commandWord == "?" || commandWord == "status"){
    Serial.println(F("STATUS HERE"));
    status();
  }else if (commandWord == "safe"){
    safeMode();
  }else if (commandWord == "lamptest"){
    Serial.println(F("Testing lamps..."));
    lampTest = true;
    delay(5000);
    lampTest = false;
    Serial.println(F("Returning to normal operation"));
  }else if (commandWord == "time" || commandWord == "rtc"){
    DateTime now = rtc.now();
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
    Serial.println();
  }else{
    Serial.print("Unknown command \'");
    Serial.print(commandWord);
    Serial.println("\' - Type \'help\' for help.");
  }
}
