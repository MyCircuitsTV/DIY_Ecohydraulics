/*
 *  BASIC EXAMPLE OF JS-SR04T ULTRASOUND SENSOR
 *
 *  This sketch reads and prints the distance from a JS-SR04T ultrasound sensor every 10 seconds.
 *  Author: Juan Francisco Fuentes Pérez - My Circuits 2024
 *  Last Edit: 15/04/24
 */

/* DIGITAL PIN FOR ULTRASOUND SETUP */
#define ECHOPIN 26 // Pin to receive the echo pulse
#define TRIGPIN 25 // Pin to send the trigger pulse

/* VARIABLES */
float dt = 0; // Distance measured by the sensor in centimeters

void setup(void)
{
  Serial.begin(115200); // Initialize serial communication at 115200 baud rate
  
  pinMode(ECHOPIN, INPUT); // Set echo pin as input
  pinMode(TRIGPIN, OUTPUT); // Set trigger pin as output
}

void loop(void)
{
  // Calculate distance from Ultrasound sensor
  getLevel();

  // Print distance readings
  Serial.print("Distance (cm): ");
  Serial.println(dt);

  delay(1000); // Delay for 10 seconds before the next loop iteration
}

/* ULTRASOUND CONTROL */
void getLevel()
{
  int distance = 0;
  int numReadings = 15;
  int numNearest = 5;
  int readings[numReadings];

  String readingsString =""; 
 
  for(int i =0;i<numReadings;i++)
  {
    digitalWrite(TRIGPIN, LOW); // Set the trigger pin to low for 2 microseconds
    delayMicroseconds(2);
    digitalWrite(TRIGPIN, HIGH); // Send a 10 microseconds high pulse to trigger the sensor
    delayMicroseconds(20); 
    digitalWrite(TRIGPIN, LOW); // Set the trigger pin back to low
    
    readings[i] = pulseIn(ECHOPIN, HIGH,26000); // Read the echo pin, pulse length in microseconds

    readingsString += readings[i]/58.00;
    readingsString += ",";

    delay(500);
  }
  
  // Determine the maximum value
  int maxValue = readings[0];
  for (int i = 1; i < numReadings; i++) {
    if (readings[i] > maxValue) {
      maxValue = readings[i];
    }
  }

  // Sort the values based on their difference from the average
  int diffs[numReadings];
  for (int i = 0; i < numReadings; i++) {
    diffs[i] = abs(readings[i] - maxValue);
  }

  for (int i = 0; i < numReadings - 1; i++) {
    for (int j = i + 1; j < numReadings; j++) {
      if (diffs[j] < diffs[i]) {
        int temp = diffs[i];
        diffs[i] = diffs[j];
        diffs[j] = temp;

        temp = readings[i];
        readings[i] = readings[j];
        readings[j] = temp;
      }
    }
  }

  // Print/select the 5 nearest values
  for (int i = 0; i < numNearest; i++) 
  {
    distance = distance + readings[i];
  }

  dt = distance/(58.00*numNearest); // Compute average distance
  
}
