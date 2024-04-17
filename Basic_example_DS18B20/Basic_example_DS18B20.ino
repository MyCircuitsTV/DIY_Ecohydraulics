/*
 *  BASIC EXAMPLE OF DS18B20 TEMPERATURE SENSOR
 *
 *  This sketch reads and prints the temperature from a DS18B20 temperature sensor every second.
 *  Author: Juan Francisco Fuentes Pérez - My Circuits 2024
 *  Last Edit: 08/03/24
 */

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

void setup(void)
{
  Serial.begin(115200); // Initialize serial communication

  // Initialize temperature sensor
  sensors.begin();
}

void loop(void)
{
  // Read water temperature
  sensors.requestTemperatures(); // Send command to get temperature readings
  waterTemp_C = sensors.getTempCByIndex(0); // Get temperature in Celsius
  waterTemp_F = sensors.getTempFByIndex(0); // Get temperature in Fahrenheit

  // Print temperature readings
  Serial.print("Water Temperature (Celsius): ");
  Serial.println(waterTemp_C);
  Serial.print("Water Temperature (Fahrenheit): ");
  Serial.println(waterTemp_F);

  delay(1000); // Wait for 1 second
}
