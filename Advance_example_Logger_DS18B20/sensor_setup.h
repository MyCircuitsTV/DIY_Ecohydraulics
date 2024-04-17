/* LIBRARIES */
#include <OneWire.h>
#include <DallasTemperature.h>

/* DIGITAL WIRE TEMPERATURE SETUP */
#define ONE_WIRE_BUS 16 // Digital pin for sensor connection

OneWire oneWire(ONE_WIRE_BUS); // Setup a OneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire); // Pass our OneWire reference to Dallas Temperature

/* VARIABLES */
float waterTemp_C = 0; // Water temperature in Celsius
float waterTemp_F = 0; // Water temperature in Fahrenheit


void initializeSensors() 
{
    // Initialize temperature sensor
    sensors.begin();
}

String createDataString() 
{
    // Read water temperature
    sensors.requestTemperatures(); // Send command to get temperature readings
    waterTemp_C = sensors.getTempCByIndex(0); // Get temperature in Celsius
    //waterTemp_F = sensors.getTempFByIndex(0); // Get temperature in Fahrenheit

	String dataString = "";

    dataString += ",";
    dataString += waterTemp_C; //Celsius
	
    return dataString;
}

String getColumnNames()
{
    return "Date,Time,Temperature,Bat"; // Column headers for data logging - "Date,Time,sensor1,sensor2,...,Bat"
}

