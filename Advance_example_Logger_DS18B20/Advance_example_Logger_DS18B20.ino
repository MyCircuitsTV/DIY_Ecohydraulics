/*
 *  ADVANCE EXAMPLE OF TEMPERATURE LOGGER
 *
 *  This sketch integrates an ESP32 with a DS18B20 temperature sensor, includes SD card logging,
 *  web server functionality, and RTC for timestamping sensor readings. Data logging and 
 *  remote monitoring are supported over WiFi.
 *
 *  Author: Juan Francisco Fuentes Pérez - My Circuits 2024
 *  Last Edit: 15/04/24
 *
 */

#include <WiFi.h>                // Built-in library for handling WiFi functionality
#include <ESP32WebServer.h>      // ESP32 Web Server library
#include <ESPmDNS.h>
#include <SD.h> 
#include <SPI.h>
#include <DS3231.h>              // RTC library for handling the DS3231 real-time clock
#include "CSS.h"                 // CSS for web server styling
#include "sensor_setup.h"      // Modify this for adding more sensors

/* PIN DEFINITIONS */
#define SD_pin 5                 // SD card module CS pin
#define on_pin 13                // Pin to trigger device ON/OFF
#define w_mode 14                // Work mode pin: logger or webserver
#define VBATPIN 12               // Battery voltage pin
#define redLed 27                // Red LED indicator pin

/* NETWORK SETTINGS */
#define SERVERNAME "MCserver"    // Name for your server
#define WIFINAME "MyCircuits"    // Name for your WiFi connection
#define PASSWORDWIFI "12345678"  // Password for your WiFi connection

/* SERVER INSTANCE */
ESP32WebServer server(80);

/* VARIABLES */
bool SD_present = false;         // Controls if the SD card is present or not
String dataString = "";          // For saving data

// Sensor/SD Variables:
float Bat;
String sRate = "60";             // Default sleep time (s)
String sName = "MC006";          // Default Sensor name
String lActive = "No";           // Default logger activation
String lFileName = "DATALOG.TXT";// Default Data logging file 
String dStart = "";              // Delay start time
String VBATCOMP = "0.00";        // Battery level
String alarmDelay = "0";         // Delay start in seconds 

int w_mode_act = 0;              // Track the previous presence of magnet

DS3231 rtc;                      // RTC variable
byte time_1[7];                  // Temp buffer used to hold BCD time/date values
char buf[12];                    // Buffer used to convert time[] values to ASCII strings

/*********  SETUP  **********/

void setup(void) {  
    Serial.begin(115200);        // Initialize serial communication at 115200 baud rate
  
    // Initialize Digital Pins
    pinMode(on_pin, OUTPUT);
    pinMode(w_mode, INPUT);
    pinMode(redLed, OUTPUT);
    digitalWrite(redLed, LOW);

	initializeSensors();
    
    // SD Card Initialization
    Serial.print(F("Initializing SD card..."));
    if (!SD.begin(SD_pin)) { 
        Serial.println(F("Card failed or not present, no SD Card data logging possible..."));
        SD_present = false;
    } else {
        Serial.println(F("Card initialised... file access enabled..."));
        SD_present = true;
        File config = SD.open("/CONFIG.TXT");
        if (config) {
            config.close();    // Close the file to free resources
            readConf();
        } else {
            updateConf(sRate, sName, lActive, lFileName, "", VBATCOMP, alarmDelay);
        }
    }

    analogReadResolution(12); // Set the ADC resolution to 12 bits
    newBatRead();             // Read the new battery level

    // Activate the power ON
    digitalWrite(on_pin, HIGH);

    // Initialize RTC
    rtc.begin();
    rtc.clearAlarms();
    rtc.enableAlarmInt(3, false);

    Serial.println("Init Finish");

    // Setup MDNS and WiFi AP if webserver mode is selected
    if (digitalRead(w_mode) == 1) {
        digitalWrite(redLed, HIGH);
        WiFi.softAP(WIFINAME, PASSWORDWIFI); // Network and password for the access point generated by ESP32

        if (!MDNS.begin(SERVERNAME)) {          
            Serial.println(F("Error setting up MDNS responder!"));
            ESP.restart(); // Restart the device if MDNS setup fails
        }

        // Server Commands
        server.on("/",         SD_dir);
        server.on("/upload",   File_Upload);
        server.on("/fupload",  HTTP_POST,[](){ server.send(200);}, handleFileUpload);
        server.on("/configuration", updateConfiguration);
    
        server.begin();
        Serial.println("HTTP server started");
    }
}

/*********  LOOP  **********/

void loop(void)
{
  //Server
  while(digitalRead(w_mode)==1)//if magnet on
  {
    server.handleClient(); //Listen for client connections
    //Serial.println(digitalRead(w_mode));
    w_mode_act = 1;
  }

  if (w_mode_act == 1) //Allow undisrupted magnet removal
  {
    delay(1000);
    digitalWrite(redLed, HIGH);
  }
  
  //When magnet is removed or not present:
  
  digitalWrite(redLed, LOW);
  
  //1. We get data from sensor
  
  //Time:
  rtc.getDateTime(time_1);
  delay(1000);
  
  dataString = "";
  dataString += rtc.dateToString(time_1, buf);
  dataString += ",";
  dataString += rtc.timeToString(time_1, buf);
  
  dataString += createDataString(); // get sensor data from sensor_setup.h 
  
  dataString += ",";  
  dataString += Bat; //Battery level
  
  Serial.println(dataString); //just for debuging

  //2. We save data into a file
  
  File dataF = SD.open("/" +lFileName, FILE_APPEND);
  dataF.println(dataString);
  dataF.close();
 
  //3. We set the new alarm
  
  rtc.clearAlarms(); //Clear alarms
  
  if (lActive.indexOf("Y") >= 0)
  {  
	  if (dStart.indexOf("T") >= 0) //It will be nice to display the configured variables in the conf file... //delayStart
	  {
  		String newTime = dStart.substring(5,7)+"/"+dStart.substring(8,10)+"/"+dStart.substring(2,4)+" "+dStart.substring(11,16)+":"+alarmDelay; //+":00"
  
  		char *cstr = new char[newTime.length() + 1];
  		strcpy(cstr, newTime.c_str());
  		
  		if (rtc.parseDateTime(cstr, time_1))
  		{
  		  //Maybe correct timing 
  		  rtc.setAlarm(time_1, ALARM1_DATE_HOUR_MIN_SEC_MATCH); //Set Alarm
  		}  
	  }
	  else //Not in perfect seconds but it will work... 
	  {
  		if (w_mode_act == 1)//Si ha entrado en web mode...
  		{
  		  //Aqui tengo que implementar la hora actual más alarma... sRate.toInt()
  		  String newAlarm = rtc.timeToString(time_1, buf);
  		  int minutos = newAlarm.substring(3,5).toInt();
  		  int segundo = newAlarm.substring(6).toInt();
  		  
  		  newAlarm = newAlarm.substring(0,2)+":00:00";
  				
  		  int secondsNew = ceil((minutos*60+segundo)/sRate.toInt())*sRate.toInt()+sRate.toInt();
  		  Serial.print("secondsNew: ");Serial.println(secondsNew);
  		  char *cstr = new char[newAlarm.length() + 1];
  		  strcpy(cstr, newAlarm.c_str());
  		  rtc.parseTime(cstr, time_1);
  		  rtc.addTime(time_1, secondsNew+alarmDelay.toInt());
  		  
  		  Serial.print("New alarm: ");Serial.println(rtc.timeToString(time_1, buf));
  		  
  		  rtc.setAlarm(time_1, ALARM1_HOUR_MIN_SEC_MATCH); //Set Alarm
  		 
  		}
  		else
  		{
  		  rtc.getAlarmTime(1, time_1);
  		  rtc.addTime(time_1, sRate.toInt());
  		  rtc.setAlarm(time_1, ALARM1_HOUR_MIN_SEC_MATCH); //Set Alarm
  		} 
	  }
	  
	  rtc.enableAlarmInt(1, true); //Activation of the alarm 1 
	  Serial.println(rtc.timeToString(time_1, buf));	
  }
  //5. We switch off the sensor
  digitalWrite(on_pin, LOW); //it just need to be triggered
  delay(5000);

}

/*********  SUPPORT FUNCTIONS  **********/

//Download a file from the SD, it is called in void SD_dir()
void SD_file_download(String filename)
{
  if (SD_present) 
  { 
    File download = SD.open("/"+filename);
    if (download) 
    {
      server.sendHeader("Content-Type", "text/text");
      server.sendHeader("Content-Disposition", "attachment; filename="+filename);
      server.sendHeader("Connection", "close");
      server.streamFile(download, "application/octet-stream");
      download.close();
    } else ReportFileNotPresent("download"); 
  } else ReportSDNotPresent();
}

//Upload a file to the SD
void File_Upload()
{
  append_page_header();
  webpage += F("<h3>Select File to Upload</h3>"); 
  webpage += F("<FORM action='/fupload' method='post' enctype='multipart/form-data'>");
  webpage += F("<input class='buttons' style='width:25%' type='file' name='fupload' id = 'fupload' value=''>");
  webpage += F("<button class='buttons' style='width:10%' type='submit'>Upload File</button><br><br>");
  webpage += F("</FORM>");
  webpage += F("<a href='/'>[Back]</a><br><br>");

  append_page_footer();
  server.send(200, "text/html",webpage);
}

//Handles the file upload a file to the SD
File UploadFile;

/*  UPDATE CONFIGURATION */
void updateConfiguration()
{
  if (server.args() > 0 ) //Arguments were received, ignored if there are not arguments
  { 
    //Configuration parameters
    Serial.println(server.arg(0));
    Serial.println(server.arg(1));
    Serial.println(server.arg(2));
    Serial.println(server.arg(3));
    Serial.println(server.arg(4)); //Battery compensation
    Serial.println(server.arg(5)); //Alarm delay
    Serial.println(server.arg(6)); //New Time
    Serial.println(server.arg(7)); //?

    //Update time 2022-12-06T18:31:15.356Z - 05/16/17 13:59:55
    
    byte time_2[7];   // Temp buffer used to hold BCD time/date values
    char buf_2[12];   // Buffer used to convert time[] values to ASCII strings 

    String newTime = server.arg(7);
    newTime = newTime.substring(5,7)+"/"+newTime.substring(8,10)+"/"+newTime.substring(2,4)+" "+newTime.substring(11,19);
    Serial.println(newTime);
     
    char *cstr = new char[newTime.length() + 1];
    strcpy(cstr, newTime.c_str());
    
    if (rtc.parseDateTime(cstr, time_2)) 
    {
      rtc.setDateTime(time_2);
      Serial.println("Time Uploaded");
    }
    
    updateConf (server.arg(0), server.arg(1), server.arg(2), server.arg(3), server.arg(6), server.arg(4), server.arg(5));
    dStart=server.arg(6); //update dsart 

    if (dStart.indexOf("T") < 0) //If we don not insert a delay, alarm to previous hour
    {
      newTime = newTime.substring(0,12)+"00:00";

      char *cstr = new char[newTime.length() + 1];
      strcpy(cstr, newTime.c_str());
    
      if (rtc.parseDateTime(cstr, time_1))
      {
        //Maybe correct timing 
        rtc.setAlarm(time_1, ALARM1_HOUR_MIN_SEC_MATCH); //Set Alarm
        Serial.print("New alarm time: ");Serial.println(newTime); Serial.println();
      }    
    }
  }

  readConf();

  webpage = "";  
  append_page_header();

  //Battery level:
  
  webpage += F("<h3>Battery level</h3>");
  //I can add other parameters
  webpage += Bat;
  webpage += F(" V <br>");
  
  //Configuration form
  webpage += F("<h3>Update Configuration</h3>"); 
  webpage += F("<FORM action='/configuration' method='post' onsubmit=\"document.getElementById('inputTime').value=new Date().toJSON();btn=document.getElementById('btnSubmit'); btn.value='(please wait)'; btn.disabled = true;\">");

  //0. Sampling time
  webpage += F("<label for=\"fname\">Sampling rate (s): </label>");
  webpage += "<input type=\"text\" id=\"fname\" name=\"fname\" value=\""+sRate+"\"><br><br>";

  //1. Sensor Name
  webpage += F("<label for=\"sname\">Sensor name: </label>");
  webpage += "<input type=\"text\" id=\"sname\" name=\"sname\" value=\""+sName+"\"><br><br>";
  
  //2. Logger Activated
  webpage += F("<label for=\"radio\">Logger activated: </label>");
  
  if (lActive.indexOf("Y") >= 0)
  {
    webpage += F("<input type=\"radio\" id=\"html\" name=\"log_active\" value=\"Yes\" checked=\"checked\">");
    webpage += F("<label for=\"Yes\">Yes</label>");
    webpage += F("<input type=\"radio\" id=\"css\" name=\"log_active\" value=\"No\">");
    webpage += F("<label for=\"No\">No</label><br><br>");
  }
  else
  {
    webpage += F("<input type=\"radio\" id=\"html\" name=\"log_active\" value=\"Yes\" >");
    webpage += F("<label for=\"Yes\">Yes</label>");
    webpage += F("<input type=\"radio\" id=\"css\" name=\"log_active\" value=\"No\" checked=\"checked\">");
    webpage += F("<label for=\"No\">No</label><br><br>");
  }

  //3. Log file
  webpage += F("<label for=\"lFile\">Log file name: </label>");
  webpage += "<input type=\"text\" id=\"lFile\" name=\"lFile\" value=\""+lFileName+"\"><br><br>";

  //6. Battery compensation
  webpage += F("<label for=\"VBATCOMP\">Battery compensation (V): </label>");
  webpage += "<input type=\"text\" id=\"VBATCOMP\" name=\"VBATCOMP\" value=\""+VBATCOMP+"\"><br><br>";

  //7. Alarm Delay
  webpage += F("<label for=\"alarmDelay\">Alarm Delay (s): </label>");
  webpage += "<input type=\"text\" id=\"alarmDelay\" name=\"alarmDelay\" value=\""+alarmDelay+"\"><br><br>";

  //4. Delay start
  webpage += F("<label for=\"delayStart\">Delay start (date and time - GMT):</label>");
  webpage += F("<input type=\"datetime-local\" id=\"delayStart\" name=\"delayStart\"><br><br>");

  //5. Time
  webpage += F("<input  id='inputTime' name='time' type='hidden'>");
  webpage += F("<input id='btnSubmit' type='submit' class='sm' value='Update'><br><br>");

  webpage += F("</FORM>");

  webpage += F("<a href='/'>[Back]</a><br>");

  if (server.args() > 0 ) //Arguments were received, ignored if there are not arguments
  {
    webpage += F("<h3 style='color:blue'>Configuration parameters and time were successfully uploaded</h3>"); 
  }
  append_page_footer();

  SendHTML_Content();
}

void newBatRead ()
{
    Bat = analogRead(VBATPIN)*6.6/(1024.00*3.75)+VBATCOMP.toFloat();
}

void readConf ()
{
  //1. Read configuration File
  File dataF = SD.open("/CONFIG.TXT");
  
  String row = dataF.readStringUntil('\n');
  row.replace("Sampling rate (sec): ","");
  row.replace("\"","");
  row.replace("\n","");

  sRate = row;

  row = dataF.readStringUntil('\n');
  row.replace("Sensor name: ","");
  row.replace("\"","");
  row.replace("\n","");
  
  sName = row;
  
  row = dataF.readStringUntil('\n');
  row.replace("Logger activated: ","");
  row.replace("\"","");
  row.replace("\n","");
  
  lActive = row;

  row = dataF.readStringUntil('\n');
  row.replace("Log file name: ","");
  row.replace("\"","");
  row.replace("\n","");
  
  lFileName = row;

  row = dataF.readStringUntil('\n');
  row.replace("Bat compensation (V): ","");
  row.replace("\"","");
  row.replace("\n","");
  
  VBATCOMP = row;

  row = dataF.readStringUntil('\n');
  row.replace("Alarm delay (s): ","");
  row.replace("\"","");
  row.replace("\n","");
  
  alarmDelay = row;
  
  dataF.close();
}

void updateConf (String sRate_new, String sName_new, String lActive_new, String lFileName_new, String dStart_new, String VBATCOMP_new,String alarmDelay_new)
{
  Serial.println("Updating configuration file...");

  SD.remove("/CONFIG.TXT");
  File dataF = SD.open("/CONFIG.TXT", FILE_WRITE);
   
  dataF.print("Sampling rate (sec): \"");dataF.print(sRate_new);dataF.println("\"");
  dataF.print("Sensor name: \"");dataF.print(sName_new);dataF.println("\"");
  dataF.print("Logger activated: \"");dataF.print(lActive_new);dataF.println("\"");
  dataF.print("Log file name: \"");dataF.print(lFileName_new);dataF.println("\"");
  dataF.print("Bat compensation (V): \"");dataF.print(VBATCOMP_new);dataF.println("\"");
  dataF.print("Alarm delay (s): \"");dataF.print(alarmDelay_new);dataF.println("\"");
  dataF.close(); 
//  delay(200);

//  Serial.println(SD.open("/" +lFileName_new));

  if (!SD.open("/" +lFileName_new))
  {
    Serial.println("Creating a new logging file: " + lFileName_new);
    
    File dataF = SD.open("/" +lFileName_new, FILE_WRITE);
    
    dataF.print("Sensor: ");dataF.println(sName_new);
    
    byte time_2[7];   // Temp buffer used to hold BCD time/date values
    char buf_2[12];   // Buffer used to convert time[] values to ASCII strings 
    rtc.getDateTime(time_2);
    dataF.print("File created (GMT): ");dataF.print(rtc.dateToString(time_2, buf_2));dataF.print(" ");dataF.println(rtc.timeToString(time_2, buf_2));

    dataF.println("Date,Time,Distance,Bat");
	dataF.println(getColumnNames());

    dataF.close();
  }
  readConf ();
}

/*  UPLOAD A NEW FILE TO THE FILING SYSTEM */
void handleFileUpload()
{ 
  HTTPUpload& uploadfile = server.upload(); //See https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WebServer/srcv
                                            //For further information on 'status' structure, there are other reasons such as a failed transfer that could be used
  if(uploadfile.status == UPLOAD_FILE_START)
  {
    String filename = uploadfile.filename;
    if(!filename.startsWith("/")) filename = "/"+filename;
    Serial.print("Upload File Name: "); Serial.println(filename);
    SD.remove(filename);                         //Remove a previous version, otherwise data is appended the file again
    UploadFile = SD.open(filename, FILE_WRITE);  //Open the file for writing in SD (create it, if doesn't exist)
    filename = String();
  }
  else if (uploadfile.status == UPLOAD_FILE_WRITE)
  {
    if(UploadFile) UploadFile.write(uploadfile.buf, uploadfile.currentSize); // Write the received bytes to the file
  } 
  else if (uploadfile.status == UPLOAD_FILE_END)
  {
    if(UploadFile)          //If the file was successfully created
    {                                    
      UploadFile.close();   //Close the file again
      Serial.print("Upload Size: "); Serial.println(uploadfile.totalSize);
      webpage = "";
      append_page_header();
      webpage += F("<h3>File was successfully uploaded</h3>"); 
      webpage += F("<h2>Uploaded File Name: "); webpage += uploadfile.filename+"</h2>";
      webpage += F("<h2>File Size: "); webpage += file_size(uploadfile.totalSize) + "</h2><br><br>"; 
      webpage += F("<a href='/'>[Back]</a><br><br>");
      append_page_footer();
      server.send(200,"text/html",webpage);
    } 
    else
    {
      ReportCouldNotCreateFile("upload");
    }
  }
}

/*  INITIAL PAGE OF THE SERVER WEB, list directory and give you the chance of deleting and uploading */
void SD_dir()
{
  if (SD_present) 
  {
    //Action acording to post, dowload or delete, by MC 2022
    if (server.args() > 0 ) //Arguments were received, ignored if there are not arguments
    { 
      Serial.println(server.arg(0));
  
      String Order = server.arg(0);
      Serial.println(Order);
      
      if (Order.indexOf("download_")>=0)
      {
        Order.remove(0,9);
        SD_file_download(Order);
        Serial.println(Order);
      }
  
      if ((server.arg(0)).indexOf("delete_")>=0)
      {
        Order.remove(0,7);
        SD_file_delete(Order);
        Serial.println(Order);
      }
    }

    File root = SD.open("/");
    if (root) {
      root.rewindDirectory();
      SendHTML_Header();    
      webpage += F("<table align='center'>");
      webpage += F("<tr><th>Name/Type</th><th style='width:20%'>Type File/Dir</th><th>File Size</th></tr>");
      printDirectory("/",0);
      webpage += F("</table>");
      SendHTML_Content();
      root.close();
    }
    else 
    {
      SendHTML_Header();
      webpage += F("<h3>No Files Found</h3>");
    }
    append_page_footer();
    SendHTML_Content();
    SendHTML_Stop();   //Stop is needed because no content length was sent
  } else ReportSDNotPresent();
}

/* PRINTS THE DIRECTORY, IT IS CALLED IN void SD_dir() */
void printDirectory(const char * dirname, uint8_t levels)
{
  File root = SD.open(dirname);

  if(!root){
    return;
  }
  if(!root.isDirectory()){
    return;
  }
  File file = root.openNextFile();

  int i = 0;
  while(file){
    if (webpage.length() > 1000) {
      SendHTML_Content();
    }
    if(file.isDirectory()){
//      webpage += "<tr><td>"+String(file.isDirectory()?"Dir":"File")+"</td><td>"+String(file.name())+"</td><td></td></tr>";
//      printDirectory(file.name(), levels-1);
    }
    else
    {
      webpage += "<tr><td>"+String(file.name())+"</td>";
      webpage += "<td>"+String(file.isDirectory()?"Dir":"File")+"</td>";
      int bytes = file.size();
      String fsize = "";
      if (bytes < 1024)                     fsize = String(bytes)+" B";
      else if(bytes < (1024 * 1024))        fsize = String(bytes/1024.0,3)+" KB";
      else if(bytes < (1024 * 1024 * 1024)) fsize = String(bytes/1024.0/1024.0,3)+" MB";
      else                                  fsize = String(bytes/1024.0/1024.0/1024.0,3)+" GB";
      webpage += "<td>"+fsize+"</td>";
      webpage += "<td>";
      webpage += F("<FORM action='/' method='post'>"); 
      webpage += F("<button type='submit' name='download'"); 
      webpage += F("' value='"); webpage +="download_"+String(file.name()); webpage +=F("'>Download</button>");
      webpage += "</td>";
      webpage += "<td>";
      webpage += F("<FORM action='/' method='post'>"); 
      webpage += F("<button type='submit' name='delete'"); 
      webpage += F("' value='"); webpage +="delete_"+String(file.name()); webpage +=F("'>Delete</button>");
      webpage += "</td>";
      webpage += "</tr>";

    }
    file = root.openNextFile();
    i++;
  }
  file.close();

}

/* DELETE A FILE FROM THE SD, IT IS CALLED IN void SD_dir() */
void SD_file_delete(String filename) 
{ 
  if (SD_present) { 
    SendHTML_Header();
    File dataFile = SD.open("/"+filename, FILE_READ); //Now read data from SD Card 
    if (dataFile)
    {
      if (SD.remove("/"+filename)) {
        Serial.println(F("File deleted successfully"));
        webpage += "<h3>File '"+filename+"' has been erased</h3>"; 
        webpage += F("<a href='/'>[Back]</a><br><br>");
      }
      else
      { 
        webpage += F("<h3>File was not deleted - error</h3>");
        webpage += F("<a href='/'>[Back]</a><br><br>");
      }
    } else ReportFileNotPresent("delete");
    append_page_footer(); 
    SendHTML_Content();
    SendHTML_Stop();
  } else ReportSDNotPresent();
} 

/* SEND HTML HEADER */
void SendHTML_Header()
{
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate"); 
  server.sendHeader("Pragma", "no-cache"); 
  server.sendHeader("Expires", "-1"); 
  server.setContentLength(CONTENT_LENGTH_UNKNOWN); 
  server.send(200, "text/html", ""); //Empty content inhibits Content-length header so we have to close the socket ourselves. 
  append_page_header();
  server.sendContent(webpage);
  webpage = "";
}

/* SEND HTML CONTENT */
void SendHTML_Content()
{
  server.sendContent(webpage);
  webpage = "";
}

/* SEND HTML STOP */
void SendHTML_Stop()
{
  server.sendContent("");
  server.client().stop(); //Stop is needed because no content length was sent
}

/* REPORT SD NOT PRESENT */
void ReportSDNotPresent()
{
  SendHTML_Header();
  webpage += F("<h3>No SD Card present</h3>"); 
  webpage += F("<a href='/'>[Back]</a><br><br>");
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}

/* REPORT FILE NOT PRESENT */
void ReportFileNotPresent(String target)
{
  SendHTML_Header();
  webpage += F("<h3>File does not exist</h3>"); 
  webpage += F("<a href='/"); webpage += target + "'>[Back]</a><br><br>";
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}

/* REPORT COULD NOT CREATE FILE */
void ReportCouldNotCreateFile(String target)
{
  SendHTML_Header();
  webpage += F("<h3>Could Not Create Uploaded File (write-protected?)</h3>"); 
  webpage += F("<a href='/"); webpage += target + "'>[Back]</a><br><br>";
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}

/* FILE SIZE CONVERSION */
String file_size(int bytes)
{
  String fsize = "";
  if (bytes < 1024)                 fsize = String(bytes)+" B";
  else if(bytes < (1024*1024))      fsize = String(bytes/1024.0,3)+" KB";
  else if(bytes < (1024*1024*1024)) fsize = String(bytes/1024.0/1024.0,3)+" MB";
  else                              fsize = String(bytes/1024.0/1024.0/1024.0,3)+" GB";
  return fsize;
}
