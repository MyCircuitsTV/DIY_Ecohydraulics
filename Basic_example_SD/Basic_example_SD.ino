/*
 *  BASIC EXAMPLE TO SAVE DATA TO SD CARD FILE
 *
 *  This sketch demonstrates how to integrate an ESP32 with a DS18B20 temperature sensor 
 *  and log the data to an SD card. 
 *
 *  Author: Juan Francisco Fuentes Pérez - My Circuits 2024
 *  Last Edit: 15/04/24
 */

/* LIBRARIES */
#include <OneWire.h>           // Library for communicating with OneWire devices
#include <DallasTemperature.h> // Library for handling the DS18B20 temperature sensors
#include <SD.h>                // Library for SD card operations
#include <SPI.h>               // Library for SPI communication, used by SD library

/* PIN DEFINITIONS */
#define ONE_WIRE_BUS 16       // Pin number to which the DS18B20 sensor is connected
#define SD_pin 5              // Chip Select (CS) pin for the SD card module

/* VARIABLES */
String dataString = "";          // String to hold data for logging
uint32_t sRate = 10;             // Sleep time in seconds (not used in this sketch)
String FileName = "DATALOG.TXT"; // Name of the file for data logging

// Setup instances for OneWire and temperature sensor
OneWire oneWire(ONE_WIRE_BUS);          // Instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire);    // Using the OneWire instance for the temperature sensor

float waterTemp_C = 0; // Variable to store water temperature in Celsius

/*********  SETUP  **********/
void setup(void) 
{  
    Serial.begin(115200);        // Start serial communication at 115200 baud
    Serial.print(F("Initializing SD card..."));
    
    // Initialize SD card
    if (!SD.begin(SD_pin)) { 
        Serial.println(F("Card failed or not present, no SD Card data logging possible..."));
    } else {
        Serial.println(F("Card initialised... file access enabled..."));
    }
	
    Serial.println("Init Finish");
	
    // Initialize temperature sensor
    sensors.begin();
}

/*********  LOOP  **********/
void loop(void)
{
    // 1. Get data from the sensor
    sensors.requestTemperatures();          // Send command to get temperature readings
    waterTemp_C = sensors.getTempCByIndex(0); // Read temperature in Celsius from the sensor
    
    // Prepare data string for logging
    dataString = "";
    dataString += millis();   // Add current time in milliseconds
    dataString += ",";
    dataString += waterTemp_C; // Add temperature reading

    Serial.println(dataString); // Print data string to serial for debugging

    // 2. Save data to a file on SD card
    File dataF = SD.open("/" + FileName, FILE_APPEND); // Open file in append mode
    if (dataF) {
        dataF.println(dataString); // Write data string to file
        dataF.close();             // Close file
    }

    delay(1000 * sRate); // Delay between readings, here it's set by sRate (10 seconds)
}

