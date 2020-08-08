//AIM 1: Dispenser should be manually operatable if required.
//AIM 2: Amount of the liquid being dispensed in automatic mode must be controlable.
//AIM 3: A log of the dispensed amount should be maintained automatically.
//Machine uses an Arduino NANO for storing the data and operating the machine.
#include <EEPROM.h>

//Pump activation, sensor, RGB LEDs and Reset button are wired according to the following allocation: 

long counter=0;  //total time for which motor has dispensed the liquid so far.
int pumpin=6; // the pin designated for activation of the pump
int rate= 5; //rate of flow for the pump in Ml/Sec
int sensorpin = 7;  //arduino pin the sensor is plugged to 
int regpin = A0;  //pin which is connected to potentiometer- designated to control the time of pump activation for dispensing.
long potval;
int maxtime=2500; //the maximum caliberatable activation time for each pump cycle
long timer;//timer is the pump activation time set using the potentiometer connected to regpin: min time is 10 milimeter,max time is 2.5 seconds right now.
int resetpin = 3; // button to activate counter reset function : as per code, it'll be activated if button is contineously pressed for more then 3 seconds
int red = 9;    //red led pin : designated to illuminate when memory reset function is activated.
int green = 10; //green led pin : designated to illuminate when dispensing is activated
int blue = 11;  //blue led pin : designated to blink in standby state 

//********************************************************************************************************************
//********************************************************************************************************************

void setup() {
  EEPROM.get(0, counter); // obtain the last value of total dispense time from EEPROMs first memory address into counter variable.
  pinMode(sensorpin, INPUT); 
  pinMode(red, OUTPUT);
  pinMode(blue, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(pumpin, OUTPUT);   
  digitalWrite(pumpin, LOW);//pump is set to it's off state
  Serial.begin(9600);   //serial communication is started at frequency 9600
 }

void loop() {
   //we'll start with reading the potentiometer value
    potval=(analogRead(regpin));//value of potentiometer is read, to set the active time for pump per stimulus
    timer= ((maxtime/1023)*potval);
    digitalWrite(blue,HIGH);     //in standby state, the indicator is blue
    digitalWrite(green,LOW);     //green is off right now. blue will be replaced with green when machine is activated
    digitalWrite(red, LOW);      //red indicator is off right now, it'll be on when the reset function is activated.
//STEP 1: Loop initialisation
     //STEP 1-A: Start the program by an option to reset the counter. if someone holds the reset button for 3 seconds, the counter resets
       if((debounce(resetpin))==HIGH){
          reset();
        }
  
     // STEP 1-B: with every loop, the information about the current statistics is printed in the serial monitor
        Serial.print("Current value of counter is: ");
        Serial.println(counter);
      //  Serial.print("and amount of sanitizer dispensed in mililiter is approximately =  ");
        //Serial.println((counter*rate));
        Serial.print("current value of pump activation time (In Mili Seconds) is");
        Serial.println((timer));
        
        delay(500);
//*********************initialisation complete*****************************************************
  

//STEP 2: the sensor status is checked and if activated, dispensor is activated 
      //STEP 2-A: sensor is checked
         if((digitalRead(sensorpin))== HIGH){
      //STEP 2-B: necessary indicators are given/changed
            digitalWrite(blue,LOW);
            digitalWrite(green,HIGH);
      //STEP 2-C: pump is activated to begin the dispensing.
            digitalWrite(pumpin, HIGH);
            delay(timer);
            digitalWrite(pumpin, LOW);
     // STEP 2-D: conter value is increased and EEPROM value is set to counter value.
            counter = counter+timer;
            EEPROM.put(0, counter);
     //STEP 2-E: necessary indicators are given/changed       
            digitalWrite(green,LOW);
            digitalWrite(blue,HIGH);
         }
//*******************************Action  completed**************************************************
        
        else{
        //in lack of an activation from sensor, always pull the pump to off position.
          digitalWrite(pumpin, LOW);
         
        }
         delay(200);
         digitalWrite(blue,LOW);
         delay(300);
}


//######################-All the functions called in the loop are listed below-####################################

//FUNCTION 1: Sometimes due to electrical disturbances, there maybe registeration of stray activation of a push switch
//debounce function is used to verify any button inputs; as is required for the counter reset function.
bool debounce(int pin){
  int A = digitalRead(pin);
  delay(30);                // this delay makes it possible to take two readings at 30 milisecond gap for verification of a desired activation of the button
  int B = digitalRead(pin);
  if(A == B){return A;}
  else{debounce(resetpin);}
  }



//FUNCTION 2: reset function will reset the counter and 0 address of the EEPROM to 0 when called for
void reset(){
  digitalWrite(blue, LOW);
  digitalWrite(red, HIGH);
  delay(3000);    // it is required to hold the button for 3 seconds for the reset function to begin it's job
  // after 3 seconds, another reading is taken, which, if matched HIGH, will reset the values
  if((debounce(resetpin)) == 1){ 
       digitalWrite(red, LOW);
       EEPROM.put(0,0);
       EEPROM.get(0,counter);
       delay(1000);
     }
  
  digitalWrite(blue, HIGH);
  }
