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
#define WLAN_SSID       "myNetwork"        // cannot be longer than 32 characters!
#define WLAN_PASS       "myPassword"
#define WLAN_SECURITY   WLAN_SEC_WPA2 // This can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2

// What TCP port to listen on for connections.
#define LISTEN_PORT           8888    

// Create CC3000 instances
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIV2);                                
                                         
// Create server
Adafruit_CC3000_Server robotServer(LISTEN_PORT);

void setup() {
   
  Serial.begin(115200);
  
  result = "";
  
  for(int i=4;i<=7;i++)
  {  
    pinMode(i, OUTPUT);  //set pin 4,5,6,7 to output mode
  }
  
  /* Initialise the module */
  Serial.println(F("\nInitializing..."));
  if (!cc3000.begin())
  {
    Serial.println(F("Couldn't begin()! Check your wiring?"));
    while(1);
  }
  
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Failed!"));
    while(1);
  }
   
  Serial.println(F("Connected!"));
  
  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP())
  {
    delay(100); // ToDo: Insert a DHCP timeout!
  }  

  /* Display the IP address DNS, Gateway, etc. */  
  while (! displayConnectionDetails()) {
    delay(1000);
  }
 
  // Start listening for connections
  robotServer.begin();
  
  Serial.println(F("Listening for connections..."));
 
}

void loop() {
  
  // Try to get a client which is connected.
  Adafruit_CC3000_ClientRef client = robotServer.available();
  if (client) {
     boolean currentLineIsBlank = true;
     // Check if there is data available to read.
     while (client.available()) {
     
       char c = client.read();
       result = result + c;
       Serial.write(c);
       
       // Delete HTTP headers
      if(result.endsWith("Content-Type: text/html"))
      {
        result="";
      }
       
       if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println();          
       }
       if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
       
     }
     // give the web browser time to receive the data
    delay(5);
    // close the connection:
    client.close();
    Serial.println("client disconnected");
    
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

/**************************************************************************/
/*!
    @brief  Tries to read the IP address and other connection details
*/
/**************************************************************************/
bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;
  
  if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("Unable to retrieve the IP Address!\r\n"));
    return false;
  }
  else
  {
    Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(ipAddress);
    Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netmask);
    Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway);
    Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
    Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
    Serial.println();
    return true;
  }
}

