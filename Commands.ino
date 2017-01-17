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
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
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
    Serial.println(F("---HELP---\n> \'help\' | \'-h\'\nIssues this command."));
    Serial.println(F("> \'?\' | \'status\'\nGets system status."));
    Serial.println(F("> \'set\'\nUsed for setting configurable items."));
    Serial.println(F("> \'safe\'\nSets mode directly to safemode."));
    Serial.println(F("> \'time\' | \'rtc\'\nPrints current set time."));
    Serial.println(F("> \'lamptest\'\nTests the output LEDs. Will all activate for a short time."));
    Serial.println(F("> \'test\'\nRuns the test function"));
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
