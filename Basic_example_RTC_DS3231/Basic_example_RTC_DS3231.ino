/*
 *  BASIC EXAMPLE USING DS3231 RTC MODULE
 *
 *  This sketch demonstrates the use of a DS3231 Real-Time Clock module with an ESP32. It covers setting the time,
 *  configuring alarms, and handling them. The example includes serial output to illustrate how the RTC maintains
 *  time and how alarms are triggered and handled.
 *
 *  Author: Juan Francisco Fuentes Pérez - My Circuits 2024
 *  Last Edit: 15/04/24
 */

#include <DS3231.h>  // Include the library for the DS3231 real-time clock.

DS3231  rtc;         // Create an instance of the DS3231 object.
byte dateTime[7];        // Temporary buffer to hold time/date values in BCD format.
char buf[12];        // Buffer to convert time/date values to ASCII strings for printing.

void setup () {
  Serial.begin(115200); // Initialize serial communication at 115200 bits per second.
  Serial.println();     // Print a blank line for better readability in the serial output.
  rtc.begin();          // Initialize the RTC module.

  // Set DS3231 to 5/16/17 11:59:55 PM using AM/PM format
  if (rtc.parseDateTime("5/16/17 11:59:55P", dateTime)) {
    // If the date/time string is successfully parsed, set the RTC with these values.
    rtc.setDateTime(dateTime);
    Serial.print(F("DS3231 set to: "));
    // Print the newly set time to the serial monitor.
    Serial.println(rtc.timeToString(dateTime, buf));
  } else {
    // If the parsing fails, print an error message.
    Serial.println(F("Unable to parse time"));
  }

  rtc.clearAlarms();  // Clear all alarms in the RTC.

  // Set Alarm 1 to trigger at 12:00:05 AM.
  rtc.parseTime("12:00:05A", dateTime);
  Serial.print(F("Alarm1 set to: "));
  Serial.println(rtc.timeToString(dateTime, buf));
  rtc.setAlarm(dateTime, ALARM1_HOUR_MIN_SEC_MATCH);  // Configure Alarm 1 to match specified time exactly.

  // Set Alarm 2 to trigger every minute.
  Serial.println(F("Alarm2 set to: Once a Minute"));
  rtc.setAlarm(dateTime, ALARM2_EVERY_MINUTE);        // Configure Alarm 2 to trigger at every minute.

  // Enable both Alarms 1 & 2.
  rtc.enableAlarmInt(3, true);

}

void loop () {
  // Continuously read and print the current date and time.
  rtc.getDateTime(dateTime);
  Serial.print(rtc.dateToString(dateTime, buf));       // Print the current date.
  Serial.print(" ");
  Serial.print(rtc.timeToString(dateTime, buf));       // Print the current time.
#if 0
  // This block is disabled by default and would print the seconds since midnight.
  Serial.print(" : ");
  Serial.print(rtc.timeToSeconds(dateTime), DEC);
#endif
  // Check if any alarms have been triggered.
  byte alarms = rtc.getAlarms();
  if (alarms != 0) {
    // If an alarm has triggered, first clear the alarm flags.
    rtc.clearAlarms();
    if (alarms & 1) {
      Serial.print(F(" - Alarm1"));
      // If Alarm 1 triggered, reconfigure it to trigger again in 5 seconds.
      rtc.getAlarmTime(1, dateTime);
      rtc.addTime(dateTime, 5);
      Serial.print(F(" - Alarm1 advanced to: "));
      Serial.print(rtc.timeToString(dateTime, buf));
      rtc.setAlarm(dateTime, ALARM1_HOUR_MIN_SEC_MATCH);
    }
    if (alarms & 2) {
      Serial.print(F(" - Alarm2"));  // Simply note that Alarm 2 has triggered.
    }
  }
  Serial.println();  // Finish the line in the serial output.
  delay(1000);       // Wait for one second before repeating the loop.
}
