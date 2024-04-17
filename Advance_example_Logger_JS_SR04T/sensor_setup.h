/* PIN DEFINITIONS */
#define ECHOPIN 26               // Pin to receive echo pulse from the ultrasound sensor
#define TRIGPIN 25               // Pin to send trigger pulse to the ultrasound sensor

float dt = 0;                    // Distance in centimeters

// Forward declaration
void getLevel();

void initializeSensors() 
{
    // Digital pin definition for ULTRASOUND
    pinMode(ECHOPIN, INPUT);
    pinMode(TRIGPIN, OUTPUT);
    digitalWrite(ECHOPIN, HIGH); // Pre-set ECHO pin to high
}

String createDataString() 
{
    getLevel();

	String dataString = "";

    dataString += ",";
    dataString += dt; //cm
	
    return dataString;
}

String getColumnNames()
{
    return "Date,Time,Distance,Bat"; // Column headers for data logging - "Date,Time,sensor1,sensor2,...,Bat"
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

    delay(100);
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
