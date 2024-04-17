/*
 *  BASIC EXAMPLE OF TEMPERATURE LOGGER
 *
 *  This sketch integrates an ESP32 with a JS-SR04T ultrasound sensor, includes SD card logging,
 *  and RTC for timestamping sensor readings.
 *  
 *  To Update time of the RTC, insert update time via serial.
 *
 *  Author: Juan Francisco Fuentes Pérez - My Circuits 2024
 *  Last Edit: 15/04/24
 *
 */

#include <SD.h>                  // SD library
#include <SPI.h>
#include <DS3231.h>              // RTC library for handling the DS3231 real-time clock
#include "sensor_setup.h"        // Modify this for adding more sensors

/* PIN DEFINITIONS */
#define SD_pin 5                 // SD card module CS pin
#define on_pin 13                // Pin to trigger device ON/OFF
#define w_mode 14                // Work mode pin: logger or webserver

/* VARIABLES */
String dataString = "";          // For saving data
uint32_t sRate = 10;             // Default sleep time (s)
String FileName = "DATALOG.TXT"; // Default Data logging file 

DS3231 rtc;                      // RTC variable
byte time_1[7];                  // Temp buffer used to hold BCD time/date values
char buf[12];                    // Buffer used to convert time[] values to ASCII strings

/*********  SETUP  **********/

void setup(void) {  
    Serial.begin(115200);        // Initialize serial communication at 115200 baud rate
	
    // Initialize Digital Pins
    pinMode(on_pin, OUTPUT);
    pinMode(w_mode, INPUT); // Not needed

	  initializeSensors();
    
    // SD Card Initialization
    Serial.print(F("Initializing SD card..."));
    if (!SD.begin(SD_pin)) { 
        Serial.println(F("Card failed or not present, no SD Card data logging possible..."));
    } else {
        Serial.println(F("Card initialised... file access enabled..."));
    }
	
	  // Activate the power ON
    digitalWrite(on_pin, HIGH);

    // Initialize RTC
    rtc.begin();
    rtc.clearAlarms();
    rtc.enableAlarmInt(3, false);

    Serial.println("Init Finish");

}

/*********  LOOP  **********/

void loop(void)
{
  //1. We get data from sensor
  
  rtc.getDateTime(time_1);  // get current date and time.
  
  dataString = "";
  dataString += rtc.dateToString(time_1, buf);
  dataString += ",";
  dataString += rtc.timeToString(time_1, buf);
  
  dataString += createDataString(); // get sensor data from sensor_setup.h 
  
  Serial.println(dataString); // just for debuging

  //2. We save data into a file
  
  File dataF = SD.open("/" +FileName, FILE_APPEND);
  dataF.println(dataString);
  dataF.close();
 
  //3. We set the new alarm
  
  rtc.addTime(time_1, sRate); // Add Sampling time
  rtc.clearAlarms();
  rtc.setAlarm(time_1, ALARM1_HOUR_MIN_SEC_MATCH); // Set Alarm

  rtc.enableAlarmInt(1, true); //Activation of the alarm 1 
  Serial.print("Alarm set to: "); Serial.println(rtc.timeToString(time_1, buf));  
  
  //4. We switch off the sensor
  digitalWrite(on_pin, LOW); //it just need to be triggered


  //5. Update time via serial if conected to PC 
  
  String menuString = "JFK"; // Variable definition
  unsigned long startTime = millis();
  
  while (millis() - startTime < 5000) {
  
  menuString = Serial.readString(); //Read string from serial
  
  if (menuString.equalsIgnoreCase("Update time"))
  {
      Serial.print("Insert new time (Format: 05/16/24 13:59:55):  ");
      Serial.println();
      
      String S_Test = Serial.readString();
      
      while(S_Test=="")
      {
        S_Test = Serial.readString();    
      }

      char *cstr = new char[S_Test.length() + 1];
      strcpy(cstr, S_Test.c_str());
      
      if (rtc.parseDateTime(cstr, time_1)) 
      {
        rtc.setDateTime(time_1);
        Serial.print(F("DS3231 set to: "));
        Serial.println(rtc.timeToString(time_1, buf));
        Serial.println("Time updated.");
      } 
      else 
      {
        Serial.println(F("Unable to parse time."));
      }

      Serial.println();
    }
  }
}
