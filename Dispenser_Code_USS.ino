//                                    VERSION 3.
// IN THIS VERSION, WE HAVE MADE THE CODE TO WORK TOGETHER WITH AN ESP8266 WIFI MODULE. THE ARDUINO IS 
// SET TO TRIGGER THE WIFI MODULE UPON IT'S COUNTER REACHING/CROSSING A PARTICULAR VALUE OF DISPENSER ACTIVE TIME.
// THE DISPENSER'S COUNTER IS RESETABLE THROUGHT THE WIFI MODULE FROM THE CONTROL ROOM SERVER ITSELF.
// PINS 9 AND A5 ARE USED TO TRIGGER THE WIFI MODULE FOR LIMIT AND RECIEVE SIGNAL FROM WIFI MODULE FOR RESETTING RESPECTIVELY.


//                                    VERSION 2.
//IN THIS VERSION, WE HAVE ADDED A 7 SEGMENT DISPLAY TO TAKE THE READING OF THE COUNTER WHEN REQUIRED.
//THIS VERSION IS STILL UNDER DEVELOPMENT, AND THERE MIGHT BE BUGS.


//                                    VERSION 1.
//AIM 1: Dispenser should be manually operatable if required.
//AIM 2: Amount of the liquid being dispensed in automatic mode must be controlable.
//AIM 3: A log of the dispensed amount should be maintained automatically.
//Machine uses an Arduino NANO for storing the data and operating the machine.
#include <EEPROM.h>
//#include <Servo.h>
//Pump activation, sensor, RGB LEDs and Reset button are wired according to the following allocation: 
int pumpin=13; // the pin designated for activation of the pump
int trigPin = A4;  //trigger pin for sensor
int echoPin= 12;

int readpin=11; // to activate reader pin
int resetpin = 10; // button to activate counter reset function : as per code, it'll be activated if button is contineously pressed for more then 3 seconds
//pins 2 to 9 are dedicated for the 7 segment display.

int regpin = A0;  //pin which is connected to potentiometer- designated to control the time of pump activation for dispensing.
int red = A1;    //red led pin : designated to illuminate when memory reset function is activated.
int green = A2; //green led pin : designated to illuminate when dispensing is activated
int blue = A3;  //blue led pin : designated to blink in standby state
int noderesetpin = A5;
int node = 9;

long counter=0;  //total time for which motor HAS dispensed the liquid so far.
int rate= 5; //rate of flow for the pump in Ml/Sec
int potval; //value of the potentiometer
int maxtime=4000; //the maximum caliberatable activation time for each pump cycle
long timer;//timer is the pump activation time set using the potentiometer connected to regpin: min time is 10 milimeter,max time is 2.5 seconds right now.
long limit=1000000000;

int actdist = 8;
bool sensor;
long duration;
int distance;

//the following chart is for the coding of a 7 segment led display which will display counter when required
int statepan[12][7]={{0,0,0,0,0,0,1}, //0   //code is based on the circuit discussed at: https://www.allaboutcircuits.com/projects/interface-a-seven-segment-display-to-an-arduino/
                     {1,0,0,1,1,1,1},  //1
                     {0,0,1,0,0,1,0},  //2
                     {0,0,0,0,1,1,0},  //3
                     {1,0,0,1,1,0,0},  //4
                     {0,1,0,0,1,0,0},  //5
                     {0,1,0,0,0,0,0},  //6
                     {0,0,0,1,1,1,1},  //7
                     {0,0,0,0,0,0,0},  //8
                     {0,0,0,0,1,0,0},  //9
                     {1,1,1,1,1,1,0},  //-
                     {1,1,1,1,1,1,1}}; //off.


//********************************************************************************************************************
//********************************************************************************************************************

void setup() {
  EEPROM.get(0, counter); // obtain the last value of total dispense time from EEPROMs first memory address into counter variable.
  //pinMode(sensorpin, INPUT); 
  
  for(int n=2; n<<9; n++){
  pinMode(n, OUTPUT);
  }
 /* for(int n=2; n<<9; n++){
  digitalWrite(n, LOW);
  
  }*/
  leddisplay(11);
 
  pinMode(pumpin, OUTPUT);
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);
//  pinMode(sensorpin, INPUT);
  pinMode(readpin, INPUT);
  pinMode(resetpin, INPUT);

  pinMode(trigPin, OUTPUT); // Sets the trigPin for sensor as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin of sensor as an Input

  sensor = LOW;
  digitalWrite(pumpin, HIGH);//pump is set to it's off state
  Serial.begin(9600);   //serial communication is started at frequency 9600

}

void loop() {
//STEP 1: Loop initialisation
  //we'll start with reading the potentiometer value and setting the timer value according to that
  potval=(analogRead(regpin));//value of potentiometer is read, to set the active time for pump per stimulus
 // timer= ((maxtime/1023)*potval);
  timer= map(potval, 0, 1023, 0, maxtime);
  //next is necessary LED indications.
  
    digitalWrite(blue, HIGH);     //in standby state, the indicator is blue
    digitalWrite(green,LOW);     //green is off right now. blue will be replaced with green when machine is activated
    digitalWrite(red, LOW);      //red indicator is off right now, it'll be on when the reset function is activated.

     //STEP 1-A: Start the program by an option to reset the counter. if someone holds the reset button for 3 seconds, the counter resets
       if((debounce(resetpin))==HIGH){
          reset();
        }
       if((debounce(readpin))==HIGH){
          readfunction(counter);     
       }
       if ((analogRead(noderesetpin))>150){
          digitalWrite(blue, LOW);
          digitalWrite(red, HIGH);
          delay(3000);    // it is required to hold the button for 3 seconds for the reset function to begin it's job
          // after 3 seconds, another reading is taken, which, if matched HIGH, will reset the values
           if((analogRead(noderesetpin))>150){ 
                 digitalWrite(red, LOW);
                 EEPROM.put(0,0);
                 EEPROM.get(0,counter);
                 delay(1000);
            }
           digitalWrite(blue, HIGH);          
       }
         
     // STEP 1-B: with every loop, the information about the current statistics is printed in the serial monitor
        Serial.print("Current value of counter is: ");
        Serial.println(counter);
        Serial.println("-------------------------------");   
      //Serial.print("and amount of sanitizer dispensed in mililiter is approximately =  ");
      //Serial.println((counter*rate));
      //Serial.println("-------------------------------");  
        Serial.print("current value of pump activation time (In Mili Seconds) is");
        Serial.println((timer));
        Serial.println("================================");
        
        delay(100);
//*********************initialisation complete*****************************************************
  
  //in pumping step, if counter is about to hitting it's limit, normal operation needs to be changed so as to force user to reset the counter.
  //so, if counter > 1900000050 then give necessary indications while pumping, else pump normally.

//STEP 2: the sensor status is checked and if activated, dispensor is activated 
      //STEP 2-A: sensor is checked
      sensorread();
         if(sensor== HIGH){
      //STEP 2-B: necessary indicators are given/changed
           if(counter<limit){      // i.e. normal pumping action.
             digitalWrite(blue,LOW);
             digitalWrite(green,HIGH);
      //STEP 2-C: pump is activated to begin the dispensing.
            digitalWrite(pumpin, LOW);
            delay(timer);
            digitalWrite(pumpin, HIGH);
     // STEP 2-D: conter value is increased and EEPROM value is set to counter value.
            counter = counter+timer; 
            EEPROM.put(0, counter);
     //STEP 2-E: necessary indicators are given/changed       
            digitalWrite(green,LOW);
            digitalWrite(blue,HIGH);
           }
         if(counter>=limit){  //i.e. pumping action for when the counter is full.
             digitalWrite(blue,LOW);
             digitalWrite(red,HIGH);
             digitalWrite(pumpin, LOW);
             delay(timer);
             digitalWrite(pumpin, HIGH);
             analogWrite(node, 168);
             for(int s=1; s<=3;s++){
                leddisplay(10);
                delay(300);
                leddisplay(11);
                delay(300);
               }
             analogWrite(node, 0);
             digitalWrite(red,LOW);
             digitalWrite(blue,HIGH);
           }
         }
//*******************************Action  completed**************************************************
        
        else{
        //in lack of an activation from sensor, always pull the pump to off position.
          digitalWrite(pumpin, HIGH);         
        }
         delay(100);
         digitalWrite(blue,LOW);
         delay(500);
}


//######################-All the functions called in the loop are listed below-####################################

//FUNCTION 1: Sometimes due to electrical disturbances, there maybe registeration of stray activation of a push switch
//debounce function is used to verify any button inputs; as is required for the counter reset function.
bool debounce(int pin){
  int A = digitalRead(pin);
  delay(30);                // this delay makes it possible to take two readings at 30 milisecond gap for verification of a desired activation of the button
  int B = digitalRead(pin);
  if(A == B){return A;}
  else{debounce(pin);}
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


//FUNCTION 3: this function will display the counter on a 7 segment display when activated with a button.
  void readfunction(long data){
    leddisplay(11);
    delay(1000);
    int arr[10]={0};
    int i=9;
    while(i>=0){
        arr[(9-(i))]= (int(data/((pow(10,i))))) - (int(data/((pow(10,(i+1)))))*10);
        //Serial.println("The value of this segment is:"+ (arr[(9-i)]));
        leddisplay((arr[(9-i)]));
        delay(900);
        leddisplay(10);
        delay(100);
        leddisplay(11);
        i--;
        Serial.print((9-i));
        Serial.println("th digit printed");
        Serial.println("+++++++++++++++++++");
    }
  leddisplay(10);
  delay(1000);
        Serial.println("End of function");
        leddisplay(11);
        Serial.println("/////////////");
   }



//FUNCTION 4: this function operates the 7 segment display.
  void leddisplay(int k){           //where k is  the number to be displayed except: 10="-" and 11="OFF"
  int l=2;      //l will be used to address the display pins as well as to call the array data.
  while(l <9){
    Serial.println((statepan[k][(l-2)]));
    digitalWrite(l, ((statepan[k][(l-2)])));
    l++;
    delay(10);
    Serial.println("----------");
    } 
  }

//FUNCTION 5: this function reads and establish's the state of the sensor activation.

void sensorread(){

// Clears the trigPin
digitalWrite(trigPin, LOW);
delayMicroseconds(2);
// Sets the trigPin on HIGH state for 10 micro seconds
digitalWrite(trigPin, HIGH);
delayMicroseconds(10);
digitalWrite(trigPin, LOW);
// Reads the echoPin, returns the sound wave travel time in microseconds
duration = pulseIn(echoPin, HIGH);
// Calculating the distance
distance= duration*0.034/2;
Serial.println(distance);
if(distance < actdist){
sensor = HIGH;  
Serial.println("Activated");
}
else{
sensor = LOW;  
Serial.println("No Activation");
}
}


  
