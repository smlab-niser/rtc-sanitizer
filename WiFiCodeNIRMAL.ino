

#include<ESP8266WiFi.h>

#define SendKey 0                       // ******DIDN'T UNDERSTAND WHY.

const char *devid = "NIRMAL-001";       //device ID
const char *server = "10.0.2.33";       //IP address of the server
int port = 9003;                        // Port of communication between server and module.
int resetword = 2312;                   //the number to be recieved when reset is is desired by the server
int message;                            //whatever message this module recieves from the client.read() i.e. from server. gets stored in this integer.
                  

int trigger = 12;                       //pin being used to recieve trigger from arduino
int resetpin = 14;                      //pin that'll be used to reset the arduino with, upon request from server

//WiFiServer server(port);              //******DIDN'T UNDERSTAND WHAT AND WHY.

const char *ssid = "Jyo's WiFi";
const char *password = "23121998";

int count=0;                           // ******DIDN'T UNDERSTAND WHY.


void setup() 
{
  Serial.begin(115200);
  pinMode(SendKey,INPUT_PULLUP);      // ******DIDN'T UNDERSTAND WHY.


  pinMode(trigger, INPUT);
  pinMode(resetpin, OUTPUT);
  
  Serial.println();
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
  Serial.println();
  WiFi.mode(WIFI_STA);                // ******DIDN'T UNDERSTAND WHAT AND WHY.


  Serial.println("connecting to Wifi");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    delay(500);
  }
  Serial.println("connected to : ");
  Serial.print(ssid);
  Serial.println();
  Serial.println(" IP address : ");
  Serial.print(WiFi.localIP());
  //Serial.println(" on port : ");        // ******DIDN'T UNDERSTAND WHY.
 // Serial.print(port);
}


void loop() 
{
    
  WiFiClient client;
  client.connect(server,9003);
  if(client)
  {
    if(client.connected())
    {
      Serial.print("connected to server on port: ");
      Serial.println(port);
      Serial.println();
    }

      
    while(client.connected())
    {

     if((digitalRead(trigger))== HIGH){
      //limit is triggered in arduino
      client.print(devid);
      Serial.println("Mail trigger sent to server");
      delay(1000);
     }
  
      while(client.available()>0)
      {
        message = client.read();
        Serial.write(message);
        if(message == resetword){
          // server has requested a counter reset on arduino
          digitalWrite(resetpin, HIGH);
          delay(3500);
          digitalWrite(resetpin, LOW);
        }
        
      }
     }
    client.stop();
    Serial.print(" client disconnected");
  }

}
