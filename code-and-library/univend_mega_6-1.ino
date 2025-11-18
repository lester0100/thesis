  
  //________________________________Headers_________________________________________
  #include <Wire.h>
  #include <LiquidCrystal_I2C.h>
  #include <Servo.h>
  #include <SoftwareSerial.h>
  #include "mp3tf16p.h"

  
  LiquidCrystal_I2C lcd(0x27, 16, 2);
  Servo servoJuice, servoCoffee;
  SoftwareSerial sim800(18, 19); //SoftwareSerial sim800(TX, RX);
  MP3Player mp3(10, 11);

  
  //___________________________Input/Output pins____________________________________
  #define myServoJuice 50
  #define myServoCoffee 2
  #define leftButton 14       //red button
  #define rightButton 15      //blue button
  #define counter A8
  #define coinSlotPower 3
  #define motorJuice A5 //7
  #define motorCoffee 5
  #define pumpJuice A2
  #define pumpCoffee A6 //A6
  #define waterPumpCold A3 //A3
  #define waterPumpHot A7
  #define waterPumpDown A0
  #define waterSensorUp 44
  #define waterValve A4
  #define IRJuice 16         //wala pa
  #define IRCoffee 51         //wala pa
  #define IRCoinBox 17
  #define mediumLevel A10
  #define fullLevel A13
  #define maintenance 8
  
  
  //___________________________Global variables___________________________________
  int isInserted;
  int value = 0;                // Coin balance
  int redButton;
  int blueButton;
  int lastButtonState = HIGH;
  int debounceDelay = 250;
  int transitionDelay = 100;
  //int waterTreshold = 350;      //Adjustable
  int volume = 30;

  //Dispensing-related intervals
  int plainDispenseDelay = 10000;
  int juicePowderDelay = 4000;
  int coffeePowderDelay = 4000;

  //Maintenance-related intervals 
  int flavoredPumpOn = 3500;
  int drainDelay = 5000;
  int numOfMaintenceSequence = 10;
  
  //powder Level Detection
  int IRsignalJuice;
  int IRsignalCoffee;
  bool juicePowderDetected = false;
  bool coffeePowderDetected = false;
  
  //Control boolean variables
  bool isCoinEnough = false;
  bool isHot = false;
  bool isCold = false;
  bool isCoffee = false;
  bool isHotWater = false;
  bool isJuice = false;
  bool isColdWater = false;
  bool isActivated = false;
  bool wantACup = false;
  bool isCup = false; 
  bool isNotCup = false;
  bool isReady = false;
  //bool bottomLevelEnough = false;
  bool rButtonState = false;
  bool lButtonState = false;
  bool onMaintenance = false;
  bool tankStatus; //true = full, false = not full
  
  //For coinslot time-out
  const unsigned long interval = 1000;
  unsigned long previousTime;
  int timeOut = 15; //Adjustable

  //For button press valdiation and refilling
  const unsigned long playInterval = 10000;
  unsigned long playPreviousTime = 0;

  String simNumber = "+639682269189"; //"+639764108583";

  //For SIM800L
  boolean getResponse(String expected_answer, unsigned int timeout=1000,boolean reset=false)  {
    boolean flag = false;
    String response = "";
    unsigned long previous;
    //*************************************************************
    for(previous=millis(); (millis() - previous) < timeout;){
      while(sim800.available()){
        response = sim800.readString();
        //----------------------------------------
        //Used in resetSIM800L function
        //If there is some response data
        if(response != ""){
          Serial.println(response);
          if(reset == true)
            return true;
        }
        //----------------------------------------
        if(response.indexOf(expected_answer) > -1){
          return true;
        }
      }
    }
  }


//___________________________________SET-UP_______________________________________
  void setup() {
    Serial.begin(9600);
    sim800.begin(9600);
    mp3.initialize();
    servoJuice.attach(myServoJuice);
    servoCoffee.attach(myServoCoffee);
    pinMode(leftButton, INPUT_PULLUP);
    pinMode(rightButton, INPUT_PULLUP);
    pinMode(counter, INPUT_PULLUP);
    pinMode(coinSlotPower, OUTPUT);
    pinMode(motorJuice, OUTPUT);
    pinMode(motorCoffee, OUTPUT);
    pinMode(pumpJuice, OUTPUT);
    pinMode(pumpCoffee, OUTPUT);
    pinMode(waterPumpCold, OUTPUT);
    pinMode(waterPumpHot, OUTPUT);
    pinMode(waterPumpDown, OUTPUT);
    pinMode(waterSensorUp, INPUT_PULLUP);  // optional
    //pinMode(waterSensorDown, INPUT_PULLUP); optional
    pinMode(waterValve, OUTPUT);
    pinMode(IRJuice, INPUT_PULLUP);
    pinMode(IRCoffee, INPUT_PULLUP);
    pinMode(mediumLevel, INPUT_PULLUP);
    pinMode(fullLevel, INPUT_PULLUP);
    delayMicroseconds(10);

    //Set-initial state to HIGH(OFF)
    digitalWrite(coinSlotPower, HIGH);
    digitalWrite(motorJuice, HIGH);
    digitalWrite(motorCoffee, HIGH);
    digitalWrite(pumpJuice, HIGH);
    digitalWrite(pumpCoffee, HIGH);
    digitalWrite(waterPumpCold, HIGH);
    digitalWrite(waterPumpHot, HIGH);
    digitalWrite(waterPumpDown, HIGH);
    digitalWrite(waterValve, HIGH);
    servoJuice.write(90);
    servoCoffee.write(90);

    //For sim800l
    sim800.println("AT");
    getResponse("OK",1000);
    sim800.println("AT+CMGF=1");
    getResponse("OK",1000);
    sim800.println("AT+CNMI=1,2,0,0,0");
    getResponse("OK",1000);
    /*
    //delete all sms
    sim800.println("AT+CMGD=1,4");
    delay(1000);
    sim800.println("AT+CMGDA= \"DEL ALL\"");
    delay(1000);
    */
 
    //for tank down
    if (digitalRead(mediumLevel == HIGH) && digitalRead(fullLevel == HIGH)) {
      tankStatus = false;
    }
    if (digitalRead(mediumLevel == LOW) && digitalRead(fullLevel == HIGH)) {
      tankStatus = false;
    }
    if (digitalRead(mediumLevel == LOW) && digitalRead(fullLevel == LOW)) {
      tankStatus = true;
    }

    lcd.init();
    lcd.backlight();
    lcd.clear();
    delay(1000);
  }
  

  //______________________________Functions_______________________________________
  void ClearAndPrint(String msg) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(msg);
  }
  
  void PrintTwoLines(String line1, String line2) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(line1);
    lcd.setCursor(0, 1);
    lcd.print(line2);
  }
  
  void ButtonChoose(bool &optRed, bool &optBlue, String msg1, String msg2, int track) {
    PrintTwoLines(msg1, msg2);
    playPreviousTime = 0;

    while (true) {
      int playCurrentTime = millis();
      redButton = digitalRead(leftButton);
      blueButton = digitalRead(rightButton);
          
          // Handle button press (debounced)
          if ((redButton == LOW || blueButton == LOW) && lastButtonState == HIGH) {
            delay(debounceDelay);
              if (redButton == LOW && blueButton == HIGH) {
                lastButtonState = LOW;
                //lButtonState = true;
                mp3.stop();
                optRed = true;
                return;
              }
              if (blueButton == LOW && redButton == HIGH) {
                lastButtonState == LOW;
                //rButtonState = true;
                mp3.stop();
                optBlue = true;
                return;
              }
          }

        // Handle button release
        if ((redButton == HIGH && blueButton == HIGH) && lastButtonState == LOW) {
          lastButtonState = HIGH;
          //return;
        }

      if (playCurrentTime - playPreviousTime >= playInterval) {
        playPreviousTime = playCurrentTime;

        mp3.playTrackNumber(track, volume, false);

        while (!mp3.playCompleted()) {
          redButton = digitalRead(leftButton);
          blueButton = digitalRead(rightButton);
          
          // Handle button press (debounced)
          if ((redButton == LOW || blueButton == LOW) && lastButtonState == HIGH) {
            delay(debounceDelay);
              if (redButton == LOW && blueButton == HIGH) {
                lastButtonState = LOW;
                //lButtonState = true;
                mp3.stop();
                optRed = true;
                return;
              }
              if (blueButton == LOW && redButton == HIGH) {
                lastButtonState == LOW;
                //rButtonState = true;
                mp3.stop();
                optBlue = true;
                return;
              }
          }

          // Handle button release
          if ((redButton == HIGH && blueButton == HIGH) && lastButtonState == LOW) {
            lastButtonState = HIGH;
            //return;
          }
        }
      }
    }       
  }
  

  void Reset() {
    isActivated = false;
    isCoinEnough = false;
    isHot = false;
    isCold = false;
    isCoffee = false;
    isHotWater = false;
    isJuice = false;
    isColdWater = false;
    wantACup = false;
    isCup = false;
    isNotCup = false;
    isReady = false;
    value = 0;
    timeOut = 15;
    //bottomLevelEnough = false;
    //rButtonState = false;
    //lButtonState = false;
    digitalWrite(coinSlotPower, HIGH);
    digitalWrite(waterValve, HIGH);
    digitalWrite(waterPumpDown, HIGH);
    servoJuice.write(90);
    servoCoffee.write(90);
    lcd.clear();
  }

  void ButtonAny(bool &isActive, String msg, int track) {
    PrintTwoLines("PRESS ANY BUTTON", msg);
    mp3.playTrackNumber(track, volume);
    
    while (true) {
      redButton = digitalRead(leftButton);
      blueButton = digitalRead(rightButton);
  
      if (redButton == 0 || blueButton == 0) {
        isActive = true;
        break;
      }
      delay(debounceDelay);
    }
  }


  void SendMsg(String msg, String number)  {
    sim800.print("AT+CMGF=1\r");
    delay(1000);
    sim800.print("AT+CMGS=\""+number+"\"\r");
    delay(1000);
    sim800.print(msg);
    delay(100);
    sim800.write(0x1A); //ascii code for ctrl-26 //sim800.println((char)26); //ascii code for ctrl-26
    delay(1000);
    Serial.println("SMS Sent Successfully.");
  }

  
  //_________________________________Main Function_________________________________
  void loop() {

    if (!isActivated) {
      redButton = digitalRead(leftButton);
      blueButton = digitalRead(rightButton);
      int mediumLevelVal = digitalRead(mediumLevel);
      int fullLevelVal = digitalRead(fullLevel);
      int waterSensorUpVal = digitalRead(waterSensorUp);
      int IRJuiceVal = digitalRead(IRJuice);
      int IRCoffeeVal = digitalRead(IRCoffee);

      if (digitalRead(maintenance) == LOW && waterSensorUpVal == LOW) {
        onMaintenance = true;
      }

      if (onMaintenance) {
        /*
          maintenance starting notif
        */

        // (1) Cleaning for juice
        for (int i = 1; i <= 10000; i++) {
          digitalWrite(pumpJuice, LOW);
          delay(2000);
          digitalWrite(pumpJuice, HIGH);
          delay(4000);
        }

        /*
          refill the tank again
        */

        

      }

      if (!onMaintenance) {
        lcd.setCursor(0 ,0);
        lcd.print("PRESS ANY BUTTON");
        lcd.setCursor(0, 1);
        lcd.print("    TO START");

        Serial.print("Medium level:  ");
        Serial.println(mediumLevelVal);
        Serial.print("Full level:  ");
        Serial.println(fullLevelVal);
        Serial.print("Water sensor up:  ");
        Serial.println(waterSensorUpVal);
        Serial.print("IRJuiceVal:  ");
        Serial.println(IRJuiceVal);
        Serial.print("IRCoffeeVal:  ");
        Serial.println(IRCoffeeVal);

        if (redButton == LOW || blueButton == LOW) {
          isActivated = true;
          digitalWrite(waterValve, HIGH);    // valve OFF
          digitalWrite(waterPumpDown, HIGH); // pump OFF
          delay(debounceDelay);
          return;
        }

        //Dispenser water level is LOW
        if (waterSensorUpVal == HIGH) {
          if (!tankStatus) {
            if (mediumLevelVal == HIGH && fullLevelVal == HIGH) {
              delay(100);
              digitalWrite(waterValve, LOW);      // Open valve
              digitalWrite(waterPumpDown, HIGH);  // Ensure the pump is OFF
              return;
            }

            if (mediumLevelVal == LOW && fullLevelVal == HIGH) {
              delay(100);
              digitalWrite(waterValve, LOW);      // Open valve
              digitalWrite(waterPumpDown, HIGH);  // Ensure the pump is OFF
              return;
            }

            if (mediumLevelVal == LOW && fullLevelVal == LOW) {
              delay(100);
              digitalWrite(waterValve, HIGH);     // Ensure valve is OFF
              digitalWrite(waterPumpDown, LOW);   // pump ON
              tankStatus = true;
              return;
            }
          }

          else {
            if (mediumLevelVal == LOW && fullLevelVal == LOW) {
              delay(100);
              digitalWrite(waterValve, HIGH);     // Ensure valve is OFF
              digitalWrite(waterPumpDown, LOW);   // pump ON
              return;
            }

            if (mediumLevelVal == LOW && fullLevelVal == HIGH) {
              delay(100);
              digitalWrite(waterValve, HIGH);     // Ensure valve is OFF
              digitalWrite(waterPumpDown, LOW);   // pump ON
              return;
            }

            if (mediumLevelVal == HIGH && fullLevelVal == HIGH) {
              delay(100);
              digitalWrite(waterValve, LOW);     // Open valve
              digitalWrite(waterPumpDown, HIGH); // Ensure the pump is OFF
              tankStatus = false;
              return;
            }
          }
        }

        //Dispenser water level is HIGH
        else {
          if (!tankStatus) {
            if (mediumLevelVal == HIGH && fullLevelVal == HIGH) {
              delay(100);
              digitalWrite(waterValve, LOW);      // Open valve
              digitalWrite(waterPumpDown, HIGH);  // Ensure the pump is OFF
              return;
            }

            if (mediumLevelVal == LOW && fullLevelVal == HIGH) {
              delay(100);
              digitalWrite(waterValve, HIGH);     // Stay at medium level (for water purifier stock)
              digitalWrite(waterPumpDown, HIGH);  // Ensure the pump is OFF
              return;
            }

            if (mediumLevelVal == LOW && fullLevelVal == LOW) {
              delay(100);
              digitalWrite(waterValve, HIGH);     // Valve OFF
              digitalWrite(waterPumpDown, HIGH);  // Pump still OFF
              tankStatus = true;
              return;
            }
          }

          else {
            if (mediumLevelVal == LOW && fullLevelVal == LOW) {
              delay(100);
              digitalWrite(waterValve, HIGH);     // Valve OFF
              digitalWrite(waterPumpDown, HIGH);  // Pump OFF
              return;
            }

            if (mediumLevelVal == LOW && fullLevelVal == HIGH) {
              delay(100);
              digitalWrite(waterValve, HIGH);      // Stay at medium level (for water purifier stock)
              digitalWrite(waterPumpDown, HIGH);   // Ensure the pump is OFF
              return;
            }

            if (mediumLevelVal == HIGH && fullLevelVal == HIGH) {
              delay(100);
              digitalWrite(waterValve, LOW);     // Valve ON
              digitalWrite(waterPumpDown, HIGH); // Pump still OFF
              tankStatus = false;
              return;
            }
          }
        }
      }
    }


    if (isActivated)  {
      /*
      while (!wantACup) {
        ButtonChoose(isCup, isNotCup, "   WANT A CUP?  ", "YES    OR     NO");
        if (isCup) {
          PrintTwoLines("Here's your cup!","");
          
            //Insert coin goes here
          
          delay(3000);
          break;
        }
        if (isNotCup) {
          PrintTwoLines("No cups for you!","");
          delay(3000);
          break;
        }
      }
      */

      ButtonChoose(isHot, isCold, "", "Hot    or   Cold", 2);
      delay(transitionDelay);
      
      if (isHot) {
        ButtonChoose(isCoffee, isHotWater, "", "Coffee or  Water", 3);
        delay(debounceDelay);
      }

      if (isCold) {
        ButtonChoose(isJuice, isColdWater, "", "Juice  or  Water", 4);
        delay(debounceDelay);
      }

      ClearAndPrint(" INSERT 5-Pesos ");
      //lcd.print(amount);
      //lcd.print("-PESO ");
      lcd.setCursor(0, 1);
      lcd.print("Your balance: ");
      lcd.print(value);

      mp3.playTrackNumber(5, volume);
      delay(500);
      digitalWrite(coinSlotPower, LOW);

      while (!isCoinEnough) {
        isInserted = digitalRead(counter);
        delay(30);
  
        if (isInserted == LOW) {
          value += 1;
          ClearAndPrint("Your balance:  ");
          lcd.print(value);
          lcd.setCursor(0, 1);
          //timeOut = 15;
        }
        
        else {
          int currentTime = millis();

          if (currentTime - previousTime >= interval) {
            previousTime = currentTime;
            ClearAndPrint("Your balance:  ");
            lcd.print(value);
            lcd.setCursor(0, 1);
            lcd.print("  Timeout: ");
            lcd.print(timeOut);
            lcd.print("s");
            timeOut--;
            if (timeOut <= 0) {
              ClearAndPrint("  Resetting...");
              delay(2000);
              Reset();
              return;
            }
          }
        }

        //User inserted sufficient coin
        if (value >= 5) {
          digitalWrite(coinSlotPower, HIGH);
          ClearAndPrint("Your balance:  ");
          lcd.print(value);
          delay(2000);
          PrintTwoLines("  Coin balance","is sufficient :>");
          mp3.playTrackNumber(7, volume);
          delay(7000);
          //ButtonAny(isReady, "  TO DISPENSE.  ", 7);
          delay(transitionDelay);
          isCoinEnough = true;
          delay(1000);
          if (isCoinEnough) {
            if (isCoffee) {
              delay(transitionDelay);
              PrintTwoLines(" Coffee is now ", "  dispensing... ");

              //Dispensing
              delay(transitionDelay);
              servoCoffee.write(0);
              digitalWrite(pumpCoffee, LOW);
              delay(1000);
              digitalWrite(motorCoffee, LOW);
              delay(1000);
              servoCoffee.write(90);
              digitalWrite(pumpCoffee, HIGH);
              delay(4000);
              servoCoffee.write(0);
              digitalWrite(pumpCoffee, LOW);
              delay(2000);
              servoCoffee.write(90);
              digitalWrite(pumpCoffee, HIGH);
              delay(5000);
              digitalWrite(motorCoffee, HIGH);
              delay(transitionDelay);
            }

            if (isHotWater) {
              delay(transitionDelay);
              //if (!isReady) {
                PrintTwoLines("Hot water is now", "  dispensing... ");
                digitalWrite(waterPumpHot, LOW);
                delay(plainDispenseDelay);
                digitalWrite(waterPumpHot, HIGH);
              //}
            }

            if (isJuice) {
              delay(transitionDelay);
              //if(!isReady) {
                PrintTwoLines(" Juice is now ", "  dispensing... ");

              //Dispensing
              delay(transitionDelay);
              servoJuice.write(0);
              digitalWrite(pumpJuice, LOW);
              delay(1000);
              digitalWrite(motorJuice, LOW);
              delay(1000);
              servoJuice.write(90);
              digitalWrite(pumpJuice, HIGH);
              delay(4000);
              servoJuice.write(0);
              digitalWrite(pumpJuice, LOW);
              delay(2000);
              servoJuice.write(90);
              digitalWrite(pumpJuice, HIGH);
              delay(5000);
              digitalWrite(motorJuice, HIGH);
              delay(transitionDelay);
                
          //}
          }

            if (isColdWater) {
            delay(transitionDelay);
            //if (isReady) {
              PrintTwoLines("  Cold water is", "now dispensing..");
              digitalWrite(waterPumpCold, LOW);
              delay(plainDispenseDelay);
              digitalWrite(waterPumpCold, HIGH);
            //}
            }

            PrintTwoLines("   Thank you!", "Enjoy your drink");
            mp3.playTrackNumber(9, 30, true); //false
            delay(3000);
            Reset();
            return;
          }
        }
      }
    }

  }
