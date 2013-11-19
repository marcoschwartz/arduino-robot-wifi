/* 
*  Simple robot control with Arduino & the CC3000 WiFi chip
*/

// Include required libraries
#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
#include <string.h>
#include "utility/debug.h"
#include <stdlib.h>

String result;
int motorCommand[4];

// Define CC3000 chip pins
#define ADAFRUIT_CC3000_IRQ   3
#define ADAFRUIT_CC3000_VBAT  8
#define ADAFRUIT_CC3000_CS    10

// Motor pins
int speed_motor1 = 6;  
int speed_motor2 = 5;
int direction_motor1 = 7;
int direction_motor2 = 4;
 
const unsigned long
  dhcpTimeout     = 60L * 1000L, // Max time to wait for address from DHCP
  connectTimeout  = 15L * 1000L, // Max time to wait for server connection
  responseTimeout = 15L * 1000L; // Max time to wait for data from server
uint32_t t;

int resultLength;

// WiFi network (change with your settings !)
#define WLAN_SSID       "yourNetwork"        // cannot be longer than 32 characters!
#define WLAN_PASS       "yourPassword"
#define WLAN_SECURITY   WLAN_SEC_WPA2 // This can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2

// Create CC3000 instances
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIV2);                                
                                         
// Local server IP, port, and repository (change with your settings !)
uint32_t ip = cc3000.IP2U32(192,168,0,1);
int port = 80;
String repository = "/arduino-robot-nn/";

void setup() {
   
  Serial.begin(115200);
  
  result = "";
  
  for(int i=4;i<=7;i++)
  {  
    pinMode(i, OUTPUT);  //set pin 4,5,6,7 to output mode
  }
  
  // Initialise the CC3000 module
  if (!cc3000.begin())
  {
    while(1);
  }

  // Connect to  WiFi network
  cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY);
  Serial.println("Connected to WiFi network!");
    
  // Check DHCP
  Serial.print(F("Requesting address from DHCP server..."));
  for(t=millis(); !cc3000.checkDHCP() && ((millis() - t) < dhcpTimeout); delay(1000));
  if(cc3000.checkDHCP()) {
    Serial.println(F("OK"));
  } else {
    Serial.println(F("failed"));
    return;
  }
 
}

void loop() {
  
  // Send request to get relay state
  Adafruit_CC3000_Client www = cc3000.connectTCP(ip, port);
  if (www.connected()) {
    Serial.println(F("Connected !"));   
    www.println("GET " + repository + "server.php HTTP/1.0");
    www.fastrprintln(F("Connection: close"));
    www.fastrprintln(F(""));
  } else {
    Serial.println(F("Connection failed"));    
    return;
  }

  while (www.connected()) {
    while (www.available()) {
      char c = www.read();
      result = result + c;
      
      // Delete HTTP headers
      if(result.endsWith("Content-Type: text/html"))
      {
        result="";
      }
    }
  }
  www.close();
 
 // Format result and extract the variables
 format_result(motorCommand,result);
 
 // Print received values
 Serial.println("Motor 1 speed: " + String(motorCommand[0]) + " and direction: " + String(motorCommand[2]));
 Serial.println("Motor 2 speed: " + String(motorCommand[1]) + " and direction: " + String(motorCommand[3]));

 // Send motor commands
 send_motor_command(speed_motor1,direction_motor1,motorCommand[0],motorCommand[2]);
 send_motor_command(speed_motor2,direction_motor2,motorCommand[1],motorCommand[3]);
 
 // Reset result variable
 result = "";
           
}

void send_motor_command(int speed_pin, int direction_pin, int pwm, boolean reverse)
{
  analogWrite(speed_pin,pwm); // Set PWM control, 0 for stop, and 255 for maximum speed
  if(reverse)
  { 
    digitalWrite(direction_pin,HIGH);    
  }
  else
  {
    digitalWrite(direction_pin,LOW);    
  }
}

void format_result(int* array, String result) {
 
 result.trim();
 resultLength = result.length();
 Serial.println(result);
 
 int commaPosition;
 int i = 0;
 do
  {
      commaPosition = result.indexOf(',');
      if(commaPosition != -1)
      {
          Serial.println( result.substring(0,commaPosition));
          array[i] = result.substring(0,commaPosition).toInt();
          i = i+1;
          result = result.substring(commaPosition+1, result.length());
      }
      else
      {
         if(result.length() > 0) {
           Serial.println(result);
          }
      }
      
   }
   while(commaPosition >=0);  
} 

