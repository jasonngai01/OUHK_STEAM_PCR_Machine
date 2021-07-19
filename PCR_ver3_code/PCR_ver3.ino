/********************************************************
 * PCR Machine Arduino Code
 * In this program, it allows users to define the Temperture 
 * , Duratuation of different stage and number of cycles. 
 * It is helpful to make copies of different samples DNA with 
 * different specification. 
 * Please check the mechanical design and the electrontic
 * diagram in the Github. 
 * https://github.com/jasonngai01/PCR_Machine_for_STEAM
 ********************************************************/

// Inport the library for driving the Temperauture sensor (DS18B20) and LiquidCrystal display (IC2 1602)

#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <math.h>
#include <Event.h>
#include <Timer.h>

// set the LCD address to 0x3F for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x3F,16,2);  

// Create the Variable for storing the User Input and Sensor message
int denatTime;          
int denatTemp;          
int annealTime;
int annealTemp;
int extendTime;
int extendTemp;
int cycles;        
int finalannealTemp; 
int decreTemp;  
unsigned long now = millis ();
int heater = 7; // D2 Pin to Control Relay, Heater relay setup is ALWAY ON
int cooler = 3; //D6 Pin to Control Relay, Cooler relay setup is ALWAY OFF
int time = 0;   // Time counter for cycle timing
int cycle = 0;  // Cycle counter
float tempC0;
float tempC1;
float tempC_avg; // The average temperature from the sensors.
int deviceCount = 0;

float tempC;
float tempC_max1;
float tempC_max2;
float tempC_max;
float tempC_highest;

const int buzzer = 10; //buzzer to arduino pin 10
 
// Data (Temperature) wire is plugged into pin 4 on the Arduino
#define ONE_WIRE_BUS 2

#define LED_heater 4  // The pin the LED is connected to D4
#define LED_cooler 5  // The pin the LED is connected to D5
#define LED_denat 13  // The pin the LED is connected to D13
#define LED_anneal 12  // The pin the LED is connected to D12
#define LED_extend 11  // The pin the LED is connected to D11
 
// Setup a oneWire instance to communicate with any OneWire devices 
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
 
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);


// Setup up the timer. The source code of the timer is here. https://github.com/JChristensen/Timer
Timer t0;
Timer t1;
Timer t2;
Timer t3;
Timer t4;
Timer t5;
Timer t6;
Timer t7;
Timer t8;

static  unsigned long time0 = 0;
static  unsigned long time1 = 0;
static  unsigned long time2 = 0;
static  unsigned long time3 = 0;
static  unsigned long time4 = 0;
static  unsigned long time5 = 0;
static  unsigned long time6 = 0;
static  unsigned long time7 = 0;
static  unsigned long time8 = 0;

//Timer function 

void writeToSerial_phase_0(){
  Serial.print(time0);
  Serial.print(" Seconds, Temperature is: ");
  Serial.println(tempC_highest);
  time0++;
}

void writeToSerial_phase_1(){
  Serial.print(time1);
  Serial.print(F(" out of 180 seconds, Current temperature: "));
  Serial.println((tempC_highest));
  time1++;
}

void writeToSerial_phase_2(){
  Serial.print(time2);
  Serial.print(F(" Seconds, temperature is "));
  Serial.println((tempC_highest));
  time2++;
}

void writeToSerial_phase_3(){
  Serial.print(time3);
  Serial.print(" / ");
  Serial.print(denatTime);
  Serial.print(F(" Seconds, temperature is "));
  Serial.println(tempC_highest);
  time3++;
}

void writeToSerial_phase_4(){
  Serial.print(time4);
  Serial.print(F(" Seconds, temperature is "));
  Serial.println(tempC_highest);
  time4++;
}

void writeToSerial_phase_5(){
  Serial.print(time5);
  Serial.print(" / ");
  Serial.print(annealTime);
  Serial.print(F(" Seconds, temperature is "));
  Serial.println(tempC_highest);
  time5++;
}
 
void writeToSerial_phase_6(){
  Serial.print(time6);
  Serial.print(F(" Seconds, temperature is "));
  Serial.println(tempC_highest);
  time6++;
}

void writeToSerial_phase_7(){
  Serial.print(time7);
  Serial.print(" / ");
  Serial.print(extendTime);
  Serial.print(F(" Seconds, temperature is "));
  Serial.println(tempC_highest);
  time7++;
}

void writeToSerial_phase_8(){
  Serial.print(time8);
  Serial.print(F(" out of 600 seconds, Current temperature: "));
  Serial.println((tempC_highest));
  time8++;
}

void setup(void)
{
  Serial.begin(9600);
  sensors.begin();
  //Setup Temperature sensor
  deviceCount = sensors.getDeviceCount();
  Serial.print(F("Locating Temperature devices..."));
  Serial.print(F("Found "));
  Serial.print(deviceCount, DEC);
  Serial.println(F(" devices."));
  Serial.println(F(""));
  
  //Setup Relay module
  pinMode(heater, OUTPUT);  // Set Pin connected to Relay as an OUTPUT
  pinMode(cooler, OUTPUT);  // Set Pin connected to Relay as an OUTPUT
  digitalWrite(heater, HIGH);  // Set Pin to HIGH to turn Relay OFF >> Testing 
  digitalWrite(cooler, HIGH);  // Set Pin to LOW to turn Relay ON >> Testing 
  delay(1080);
  digitalWrite(heater, HIGH);  // Set Pin to HIGH to turn Relay OFF
  digitalWrite(cooler, LOW);  // Set Pin to LOW to turn Relay OFF
  
  //Setup IC2 LCD
  lcd.init();
  lcd.clear();         
  lcd.backlight();      // Make sure backlight is on

  // Print a message on both lines of the LCD.
  lcd.setCursor(3,0);   //Set cursor to character 2 on line 0
  lcd.print("OUHK STEAM");
  
  lcd.setCursor(0,1);   //Move cursor to character 2 on line 1
  lcd.print("PCR Thermocycler");

  //Setup buzzer

  pinMode(buzzer, OUTPUT); // Set buzzer - pin 9 as an output
  tone(buzzer, 440); // Send 1KHz sound signal...
  delay(500);        // ...for 1 sec
  tone(buzzer, 493); // Send 1KHz sound signal...
  delay(500);        // ...for 1 sec
  tone(buzzer, 440); // Send 1KHz sound signal...
  delay(500);        // ...for 1 sec
  tone(buzzer, 493); // Send 1KHz sound signal...
  delay(500);        // ...for 1 sec
  noTone(buzzer);     // Stop sound...
  delay(1000);        // ...for 1sec

  //Setup the LED

  pinMode(LED_heater, OUTPUT); // Declare the LED as an output
  pinMode(LED_cooler, OUTPUT); // Declare the LED as an output
  pinMode(LED_denat, OUTPUT); // Declare the LED as an output
  pinMode(LED_anneal, OUTPUT); // Declare the LED as an output
  pinMode(LED_extend, OUTPUT); // Declare the LED as an output

  digitalWrite(LED_heater, HIGH); // Turn the LED on
  digitalWrite(LED_cooler, HIGH); // Turn the LED on
  digitalWrite(LED_denat, HIGH); // Turn the LED on
  digitalWrite(LED_anneal, HIGH); // Turn the LED on
  digitalWrite(LED_extend, HIGH); // Turn the LED on
  delay(500);
  
  digitalWrite(LED_heater, LOW); // Turn the LED on
  digitalWrite(LED_cooler, LOW); // Turn the LED on
  digitalWrite(LED_denat, LOW); // Turn the LED on
  digitalWrite(LED_anneal, LOW); // Turn the LED on
  digitalWrite(LED_extend, LOW); // Turn the LED on
  delay(500);


  //timer setup
  t0.every(600, writeToSerial_phase_0); // 每經過1000毫秒，就會呼叫writeToSerial
  t1.every(600, writeToSerial_phase_1); // 每經過1000毫秒，就會呼叫writeToSerial
  t2.every(600, writeToSerial_phase_2); // 每經過1000毫秒，就會呼叫writeToSerial
  t3.every(600, writeToSerial_phase_3); // 每經過1000毫秒，就會呼叫writeToSerial
  t4.every(600, writeToSerial_phase_4); // 每經過1000毫秒，就會呼叫writeToSerial
  t5.every(600, writeToSerial_phase_5); // 每經過1000毫秒，就會呼叫writeToSerial
  t6.every(600, writeToSerial_phase_6); // 每經過1000毫秒，就會呼叫writeToSerial
  t7.every(600, writeToSerial_phase_7); // 每經過1000毫秒，就會呼叫writeToSerial
  t8.every(600, writeToSerial_phase_8); // 每經過1000毫秒，就會呼叫writeToSerial

  delay(500);

}

void loop(void)
{
  //In the Initial Stage, User need to input the recipe for PCR heating and cooling.
  Serial.println(F("Please input all the followings parameters for the PCR. "));
  Serial.println(F("1. Enter the Denaturation Time in seconds: "));
  tone(buzzer, 440); // Send 1KHz sound signal...
  delay(500);        // ...for 1 sec
  noTone(buzzer);     // Stop sound...
  delay(1000);        // ...for 1sec        
  while (Serial.available()==0)  {}
  denatTime=Serial.parseInt();  
  Serial.println(denatTime);                                        
  //

  //
  Serial.println(F("2. Enter the Denaturation Temp in Celsius ")); 
  tone(buzzer, 440); // Send 1KHz sound signal...
  delay(500);        // ...for 1 sec
  noTone(buzzer);     // Stop sound...
  delay(1000);        // ...for 1sec        
  now = millis ();
  while (millis () - now < 1000)
  Serial.read ();  // read and discard any input
  while (Serial.available()==0)  {}
  denatTemp=Serial.parseInt();                                      
  Serial.println(denatTemp); 
  //Read user input into age

  //
  Serial.println(F("3. Enter the Annealing Time in seconds ")); 
  tone(buzzer, 440); // Send 1KHz sound signal...
  delay(500);        // ...for 1 sec
  noTone(buzzer);     // Stop sound...
  delay(1000);        // ...for 1sec        
  now = millis ();
  while (millis () - now < 1000)
  Serial.read ();  // read and discard any input
  while (Serial.available()==0)  {}
  annealTime=Serial.parseInt();                                      
  Serial.println(annealTime);  
  //

  //
  Serial.println(F("4. Enter the Annealing Temp in Celsius ")); 
  tone(buzzer, 440); // Send 1KHz sound signal...
  delay(500);        // ...for 1 sec
  noTone(buzzer);     // Stop sound...
  delay(1000);        // ...for 1sec        
  now = millis ();
  while (millis () - now < 1000)
  Serial.read ();  // read and discard any input
  while (Serial.available()==0)  {}
  annealTemp=Serial.parseInt();                                     
  Serial.println(annealTemp);  
  //

  //
  Serial.println(F("5. Enter the Extension Time in seconds ")); 
  tone(buzzer, 440); // Send 1KHz sound signal...
  delay(500);        // ...for 1 sec
  noTone(buzzer);     // Stop sound...
  delay(1000);        // ...for 1sec        
  now = millis ();
  while (millis () - now < 1000)
  Serial.read ();  // read and discard any input
  while (Serial.available()==0)  {}
  extendTime=Serial.parseInt();                                      
  Serial.println(extendTime);  
  //

  //
  Serial.println(F("6. Enter the Extension Temp in Celsius ")); 
  tone(buzzer, 440); // Send 1KHz sound signal...
  delay(500);        // ...for 1 sec
  noTone(buzzer);     // Stop sound...
  delay(1000);        // ...for 1sec        
  now = millis ();
  while (millis () - now < 1000)
  Serial.read ();  // read and discard any input
  while (Serial.available()==0)  {}
  extendTemp=Serial.parseInt();                                      
  Serial.println(extendTemp);
  //
  
  //
  Serial.println(F("7. Enter the number of cycles ")); 
  tone(buzzer, 440); // Send 1KHz sound signal...
  delay(500);        // ...for 1 sec
  noTone(buzzer);     // Stop sound...
  delay(1000);        // ...for 1sec        
  now = millis ();
  while (millis () - now < 1000)
  Serial.read ();  // read and discard any input
  while (Serial.available()==0)  {}
  cycles=Serial.parseInt();                                      
  Serial.println(cycles);
  //
  
  //
  Serial.println(F("8. Enter the Final Annealing Temp in Celsius ")); 
  tone(buzzer, 440); // Send 1KHz sound signal...
  delay(500);        // ...for 1 sec
  noTone(buzzer);     // Stop sound...
  delay(1000);        // ...for 1sec        
  now = millis ();
  while (millis () - now < 1000)
  Serial.read ();  // read and discard any input
  while (Serial.available()==0)  {}
  finalannealTemp=Serial.parseInt();                                      
  Serial.println(finalannealTemp);   
  //Read user input into age

  //
  Serial.println(F("9. Enter the Temp decreases per cycle in Celsius ")); 
  tone(buzzer, 440); // Send 1KHz sound signal...
  delay(500);        // ...for 1 sec
  noTone(buzzer);     // Stop sound...
  delay(1000);        // ...for 1sec        
  now = millis ();
  while (millis () - now < 1000)
  Serial.read ();  // read and discard any input
  while (Serial.available()==0)  {}
  decreTemp=Serial.parseInt();                                      
  Serial.println(decreTemp);                                                                           
  //All the parameters of recipe for PCR heating and cooling are received. 
  Serial.println(F("All the parameters for the PCR are already inputted. ")); 
  delay(1080);
  Serial.println(F("The PCR process is running now. ")); 

  tone(buzzer, 440); // Send 1KHz sound signal...
  delay(500);        // ...for 1 sec
  noTone(buzzer);     // Stop sound...
  delay(1000);        // ...for 1sec        
  tone(buzzer, 440); // Send 1KHz sound signal...
  delay(500);        // ...for 1 sec
  noTone(buzzer);     // Stop sound...
  delay(1000);        // ...for 1sec        
  tone(buzzer, 440); // Send 1KHz sound signal...
  delay(500);        // ...for 1 sec
  noTone(buzzer);     // Stop sound...
  delay(1000);        // ...for 1sec       
  
  delay(1000);
  
  // Warm up stage: Heating to the denatTemp
  sensors.requestTemperatures(); 
  Serial.println(F("*** Warming up. This may take several minutes. ***"));
  Serial.print(F("*** Heating to "));
  Serial.print(denatTemp);
  Serial.println("C ***");
  delay(500);
  
  while (tempC_highest  < denatTemp) {      //***95*** While Loop to Bring up to Temp - 95C                     
    
    t0.update();
   
    tempC_highest = highest_temp();
    
    // Print a message on both lines of the LCD.
    lcd.setCursor(0,0);   //Set cursor to character 2 on line 0
    lcd.print("PCR Thermocycler");
    lcd.setCursor(0,1);   //Move cursor to character 2 on line 1
    lcd.print("Temperature: ");
    lcd.setCursor(12,1);   //Move cursor to character 2 on line 1
    lcd.print((tempC_highest));
    
    digitalWrite(heater, LOW);  // Heating   
    digitalWrite(cooler, LOW);  

    digitalWrite(LED_heater, HIGH); // Turn the LED on
    digitalWrite(LED_cooler, LOW); // Turn the LED off
    
  }
 
    
  //Phase 1: Stay at 94C for 180 sec
  Serial.println(F("*** Intialization: Phase 1 (Maintain at Denaturation Temp for 180 seconds).***"));
   
  
  while ( time1 < 180) {  // ***Set to 180*** While Loop to keep temp at 95C for 180 seconds         
    
    t1.update();
    
    tempC_highest = highest_temp();

    digitalWrite(LED_denat, HIGH); // Turn the LED on
    digitalWrite(LED_anneal, LOW); // Turn the LED off
    digitalWrite(LED_extend, LOW); // Turn the LED off
         
    // Print a message on both lines of the LCD.
    lcd.setCursor(0,0);   //Set cursor to character 2 on line 0
    lcd.print("PCR Thermocycler");
    lcd.setCursor(0,1);   //Move cursor to character 2 on line 1
    lcd.print("Temperature: ");
    lcd.setCursor(12,1);   //Move cursor to character 2 on line 1
    lcd.print((tempC_highest));
    
    if (tempC_highest > denatTemp ) {                                                                         
         digitalWrite(heater, HIGH); // Stop Heating
         digitalWrite(cooler, LOW);
         
         digitalWrite(LED_heater, LOW); // Turn the LED off
         digitalWrite(LED_cooler, LOW); // Turn the LED off
         }   
                                                                             
    else {                                                                         
         digitalWrite(heater, LOW); // Heating
         digitalWrite(cooler, LOW);

         digitalWrite(LED_heater, HIGH); // Turn the LED on
         digitalWrite(LED_cooler, LOW); // Turn the LED off

         }                                                                        
  }   
  
//Cycle Counter
//The cycle loops starts here. 
//The loop starts from Phase 2 and ends to Phase 7. 

while (cycle < cycles) {      
  cycle = cycle + 1;
  Serial.println(F("**********************************************"));
  Serial.print(cycle);
  Serial.print(F(" of "));
  Serial.print(cycles);
  Serial.println(F(" cycles"));
  Serial.println(F("**********************************************"));

  
// Print a message on both lines of the LCD.
  lcd_display(cycles, cycle, tempC_highest);

  //Phase 2: Increase Temp to 94C  
  sensors.requestTemperatures(); // Send the command to get temperatures
  
  Serial.println(F("**********************************************"));
  Serial.print(cycle);
  Serial.print(F(" of "));
  Serial.print(cycles);
  Serial.print(F(" cycles, "));
  Serial.print(F("Phase 2: Set to "));
  Serial.print(denatTemp);
  Serial.println(F("C"));
  Serial.println(F("**********************************************"));

  digitalWrite(LED_denat, HIGH); // Turn the LED on
  digitalWrite(LED_anneal, LOW); // Turn the LED off
  digitalWrite(LED_extend, LOW); // Turn the LED off
  
  time2 = 0; //reset the timer2

  while (tempC_highest < denatTemp) {      //***94*** While Loop to Bring up to Temp - 94C                  
    
    tempC_highest = highest_temp();

    t2.update();

    //Serial.print("Temperature is: ");
    //Serial.println(tempC_highest);
    
    // Print a message on both lines of the LCD.
    lcd_display(cycles, cycle, tempC_highest);

    digitalWrite(heater, LOW); // Heating    
    digitalWrite(cooler, LOW);

    digitalWrite(LED_heater, HIGH); // Turn the LED on
    digitalWrite(LED_cooler, LOW); // Turn the LED off
   
  }
  
  //Phase 3: Hold at 95C for Set Denaturation Time in sec Cycle  

  Serial.println(F("**********************************************"));
  Serial.print(cycle);
  Serial.print(F(" of "));
  Serial.print(cycles);
  Serial.print(F(" cycles, "));
  Serial.print(F("Phase 3: Hold at "));
  Serial.print(denatTemp);
  Serial.print(F("C for "));
  Serial.print(denatTime);
  Serial.println(F(" seconds"));
  Serial.println(F("**********************************************"));

  digitalWrite(LED_denat, HIGH); // Turn the LED on
  digitalWrite(LED_anneal, LOW); // Turn the LED on
  digitalWrite(LED_extend, LOW); // Turn the LED on
  
  time3=0;
  
  while ( time3 < denatTime ) {   // Hold at 95C.
    
    t3.update();
        
    // Print a message on both lines of the LCD.
    lcd_display(cycles, cycle, tempC_highest);

    tempC_highest = highest_temp();

    if (tempC_highest > denatTemp) {                                                                         
         digitalWrite(heater, HIGH); // Stop Heating
         digitalWrite(cooler, LOW);

         digitalWrite(LED_heater, LOW); // Turn the LED off
         digitalWrite(LED_cooler, LOW); // Turn the LED off
                  
                                          }                                                                       
    else {                                                                         
         digitalWrite(heater, LOW); // Heating
         digitalWrite(cooler, LOW);

         digitalWrite(LED_heater, HIGH); // Turn the LED on
         digitalWrite(LED_cooler, LOW); // Turn the LED off
              
         }                                                                        
  }   
 
//Phase 4: Drop Temp to 53C  
  
  Serial.println(F("**********************************************"));
  Serial.print(cycle);
  Serial.print(F(" of "));
  Serial.print(cycles);
  Serial.print(F(" cycles, "));
  Serial.print(F("Phase 4: Set to "));
  Serial.print(annealTemp);
  Serial.println(F("C"));
  Serial.println(F("**********************************************"));

  digitalWrite(LED_denat, LOW); // Turn the LED off
  digitalWrite(LED_anneal, HIGH); // Turn the LED on
  digitalWrite(LED_extend, LOW); // Turn the LED off

  time4=0;

  while (tempC_highest > annealTemp) {      //***53*** While Loop to Drop Temp - 53C                  

    t4.update();

    tempC_highest = highest_temp();

    //Serial.print("Temperature is: ");
    //Serial.println((tempC_highest));
    
    digitalWrite(heater, HIGH); // Stop Heating
    digitalWrite(cooler, HIGH); // Cooling

    digitalWrite(LED_heater, LOW); // Turn the LED off
    digitalWrite(LED_cooler, HIGH); // Turn the LED on
    
     // Print a message on both lines of the LCD.
    lcd_display(cycles, cycle, tempC_highest);

  }

//Phase 5: Hold at 53C for Set Annealing Time in sec Cycle  
  Serial.println(F("**********************************************"));
  Serial.print(cycle);
  Serial.print(F(" of "));
  Serial.print(cycles);
  Serial.print(F(" cycles, "));
  Serial.print(F("Phase 5: Hold at "));
  Serial.print(annealTemp);
  Serial.print(F("C for "));
  Serial.print(annealTime);
  Serial.println(F(" seconds"));
  Serial.println(F("**********************************************"));

  digitalWrite(LED_denat, LOW); // Turn the LED off
  digitalWrite(LED_anneal, HIGH); // Turn the LED on
  digitalWrite(LED_extend, LOW); // Turn the LED off
    
  time5 = 0;
  
  while ( time5 < annealTime) {   // Hold at 53C.
    
    t5.update();

    tempC_highest = highest_temp();

    // Print a message on both lines of the LCD.
    lcd_display(cycles, cycle, tempC_highest);

    
    if (tempC_highest > annealTemp) {                                                                         
         digitalWrite(heater, HIGH); // Stop Heating
         digitalWrite(cooler, LOW);

         digitalWrite(LED_heater, LOW); // Turn the LED off
         digitalWrite(LED_cooler, LOW); // Turn the LED off
                 
                                          }        
                                                                                                         
    else {                                                                         
         digitalWrite(heater, LOW); // Heating         
         digitalWrite(cooler, LOW);

         digitalWrite(LED_heater, HIGH); // Turn the LED on
         digitalWrite(LED_cooler, LOW); // Turn the LED off
        
         }                                                                        
  }   


//Phase 6: Increase Temp to 72C  
  
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println(F("**********************************************"));
  Serial.print(cycle);
  Serial.print(F(" of "));
  Serial.print(cycles);
  Serial.print(F(" cycles, "));
  Serial.print(F("Phase 6: Set to "));
  Serial.print(extendTemp);
  Serial.println(F("C"));
  Serial.println(F("**********************************************"));


  digitalWrite(LED_denat, LOW); // Turn the LED off
  digitalWrite(LED_anneal, LOW); // Turn the LED off
  digitalWrite(LED_extend, HIGH); // Turn the LED on

  time6 = 0;

  while (tempC_highest < extendTemp) {      //***72*** While Loop to Bring up to Temp - 72C   
    
    t6.update();
               
    tempC_highest = highest_temp();
    
    digitalWrite(heater, LOW); // Heating    
    digitalWrite(cooler, LOW);

    digitalWrite(LED_heater, HIGH); // Turn the LED on
    digitalWrite(LED_cooler, LOW); // Turn the LED off
    
    // Print a message on both lines of the LCD.
    lcd_display(cycles, cycle, tempC_highest);

  }

//Phase 7: Hold at 72C for set Extension Time in sec Cycle  
  Serial.println(F("**********************************************"));
  Serial.print(cycle);
  Serial.print(F(" of "));
  Serial.print(cycles);
  Serial.print(F(" cycles, "));
  Serial.print(F("Phase 7: Hold at "));
  Serial.print(extendTemp);
  Serial.print(F("C for "));
  Serial.print(extendTime);
  Serial.println(F(" seconds"));
  Serial.println(F("**********************************************"));

  digitalWrite(LED_denat, LOW); // Turn the LED off
  digitalWrite(LED_anneal, LOW); // Turn the LED off
  digitalWrite(LED_extend, HIGH); // Turn the LED on

  
  time7 = 0;
  
  while ( time7 < extendTime) {   // Hold at 72C.

    t7.update();

    tempC_highest = highest_temp();

    // Print a message on both lines of the LCD.
    lcd_display(cycles, cycle, tempC_highest);
    
    if (tempC_highest > extendTemp ) {                                                                         
         digitalWrite(heater, HIGH); // Stop Heating
         digitalWrite(cooler, LOW);

         digitalWrite(LED_heater, LOW); // Turn the LED on
         digitalWrite(LED_cooler, LOW); // Turn the LED on

                                                   }                                                                       
    else {                                                                         
         digitalWrite(heater, LOW); // Heating
         digitalWrite(cooler, LOW);

         digitalWrite(LED_heater, HIGH); // Turn the LED on
         digitalWrite(LED_cooler, LOW); // Turn the LED off
         
                 }                                                                        
  }
  
  //The annealTemp is going to decrease in decreTemp in every cycle until to the finalannealTemp.
  //For example, if the annealTemp is 53C, the decreTemp is 1C and the finalannealTemp is 50C.
  //There are total 20 cycles. In cycle 1, the annealTemp is 53C. In cycle 2, the annealTemp is 52C.
  //In cycle 3, the annealTemp is 51C. In cycle 4, the annealTemp is 50C.
  //In cycle 5 to cycle 20, it is already achieved finalannealTemp. It will keep 50C. 
  
  if (annealTemp - decreTemp > finalannealTemp) {
    annealTemp = annealTemp - decreTemp;
  }
  else {
  annealTemp = finalannealTemp;
  }

} //The cycle loops end here. 

//Phase 8: Hold at 72C for 10 min  
  Serial.println(F("**********************************************"));
  Serial.print(F("Phase 8: Hold at "));
  Serial.print(extendTemp);
  Serial.print(F("C for "));
  Serial.println(F("10 minutes"));
  Serial.println(F("**********************************************"));

  digitalWrite(LED_denat, LOW); // Turn the LED off
  digitalWrite(LED_anneal, LOW); // Turn the LED off
  digitalWrite(LED_extend, HIGH); // Turn the LED on
  
  time8 = 0;
  
  while ( time8 < 600) {   // Hold at 72C for 600 sec.
    
    t8.update();
     
    // Print a message on both lines of the LCD
    lcd.init();
    lcd.clear();         
    lcd.backlight();      // Make sure backlight is on
    lcd.setCursor(0,0);   //Set cursor to character 2 on line 0
    lcd.print("Final  Extension");
    lcd.setCursor(0,1);   //Move cursor to character 2 on line 1
    lcd.print("Temperature: ");
    lcd.setCursor(12,1);   //Move cursor to character 12 on line 1
    lcd.print((tempC_highest));
    
    tempC_highest = highest_temp();

    if (tempC_highest > extendTemp ) {                                                                         
         digitalWrite(heater, HIGH); // Stop Heating
         digitalWrite(cooler, LOW);
         
         digitalWrite(LED_heater, LOW); // Turn the LED off
         digitalWrite(LED_cooler, LOW); // Turn the LED off      
         }     
                                                                                            
    else {                                                                         
         digitalWrite(heater, LOW); // Heating
         digitalWrite(cooler, LOW);

         digitalWrite(LED_heater, HIGH); // Turn the LED on
         digitalWrite(LED_cooler, LOW); // Turn the LED off
         
         
         }                                                                        
  }   

  
//Phase 9: Final cooling stage 
// Print a message on both lines of the LCD.

tempC_highest = highest_temp();

lcd.init();
lcd.clear();         
lcd.backlight();      // Make sure backlight is on
lcd.setCursor(2,0);   //Set cursor to character 2 on line 0
lcd.print("Cooling Down");
lcd.setCursor(0,1);   //Move cursor to character 2 on line 1
lcd.print("Temperature: ");
lcd.setCursor(12,1);   //Move cursor to character 12 on line 1
lcd.print((tempC_highest));    
digitalWrite(heater, HIGH);  // Set Pin to HIGH to turn Relay OFF, Stop Heating
digitalWrite(cooler, HIGH);  // Set Pin to HIGH to turn Relay ON for permanent cool down

digitalWrite(LED_heater, LOW); // Turn the LED off
digitalWrite(LED_cooler, HIGH); // Turn the LED on

Serial.println(F("**********************************************"));
Serial.println(F("Phase9: Final cooling"));
Serial.println(F("STILL   HOT"));
Serial.println(F("   Cooling Down"));
Serial.println(F("   Completed"));

//ENDING EFFECT
digitalWrite(LED_denat, HIGH); // Turn the LED on
digitalWrite(LED_anneal, HIGH); // Turn the LED on
digitalWrite(LED_extend, HIGH); // Turn the LED on
tone(buzzer, 440); // Send 1KHz sound signal...

delay(500);

digitalWrite(LED_denat, LOW); // Turn the LED on
digitalWrite(LED_anneal, LOW); // Turn the LED on
digitalWrite(LED_extend, LOW); // Turn the LED on
noTone(buzzer);     // Stop sound...

delay(500);

digitalWrite(LED_denat, HIGH); // Turn the LED on
digitalWrite(LED_anneal, HIGH); // Turn the LED on
digitalWrite(LED_extend, HIGH); // Turn the LED on
tone(buzzer, 440); // Send 1KHz sound signal...

delay(500);

digitalWrite(LED_denat, LOW); // Turn the LED on
digitalWrite(LED_anneal, LOW); // Turn the LED on
digitalWrite(LED_extend, LOW); // Turn the LED on
noTone(buzzer);     // Stop sound...

delay(500);

digitalWrite(LED_denat, HIGH); // Turn the LED on
digitalWrite(LED_anneal, HIGH); // Turn the LED on
digitalWrite(LED_extend, HIGH); // Turn the LED on
tone(buzzer, 440); // Send 1KHz sound signal...

delay(500);

digitalWrite(LED_denat, LOW); // Turn the LED on
digitalWrite(LED_anneal, LOW); // Turn the LED on
digitalWrite(LED_extend, LOW); // Turn the LED on
noTone(buzzer);     // Stop sound...

delay(500);

digitalWrite(LED_denat, HIGH); // Turn the LED on
digitalWrite(LED_anneal, HIGH); // Turn the LED on
digitalWrite(LED_extend, HIGH); // Turn the LED on
tone(buzzer, 440); // Send 1KHz sound signal...

delay(500);

digitalWrite(LED_denat, LOW); // Turn the LED on
digitalWrite(LED_anneal, LOW); // Turn the LED on
digitalWrite(LED_extend, LOW); // Turn the LED on
noTone(buzzer);     // Stop sound...

delay(500);


while(true);

}

// There are 2 functions in the program. The first one is the highest temp reading. The second one is the lcd display.

float highest_temp(){
  float tempC_max1;
  float tempC_max2;
  float tempC_max;
  
  sensors.requestTemperatures(); // Send the command to get temperatures
  tempC_max1 = max(sensors.getTempCByIndex(0), sensors.getTempCByIndex(1));
  tempC_max2 = max(sensors.getTempCByIndex(2), sensors.getTempCByIndex(3));
  tempC_max = max(tempC_max1, tempC_max2);
  return tempC_max;
  
}

float lcd_display(int cycles, int cycle, float tempC_highest){
  lcd.init();
  lcd.clear();         
  lcd.backlight();      // Make sure backlight is on
  lcd.setCursor(0,0);   //Set cursor to character 2 on line 0
  lcd.print("Cycle:");
  lcd.setCursor(7,0);   //Move cursor to character 6 on line 0
  lcd.print(cycle);
  lcd.setCursor(10,0);   //Set cursor to character 2 on line 0
  lcd.print("of");
  lcd.setCursor(13,0);   //Move cursor to character 6 on line 0
  lcd.print(cycles);
  lcd.setCursor(0,1);   //Move cursor to character 2 on line 1
  lcd.print("Temperature: ");
  lcd.setCursor(12,1);   //Move cursor to character 12 on line 1
  lcd.print((tempC_highest));
  
}
