/* Arduino temperature logger with SD card, DS18B20 sensor and DS3231 (DS1307) RTC.
 * Time, date and temperature are displayed on 20x4 LCD.
 * This is a free software with NO WARRANTY.
 * https://simple-circuit.com/
 */
 
#include <SD.h>               // include Arduino SD library
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal.h>    // include Arduino LCD library
#include <RTClib.h>           // include Adafruit RTC library
 
// LCD module connections (RS, E, D4, D5, D6, D7)
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);
 
// initialize RTC library
RTC_DS3231 rtc;
DateTime   now;
 
// buttons definition
#define ONE_WIRE_BUS A3
#define button1       A1   // button B1 is connected to Arduino pin A1
#define button2       A2   // button B2 is connected to Arduino pin A2
// define DS18B20 data pin
#define button4 A3
File dataLog;
boolean sd_ok = 0;

 // Setup a oneWire instance to communicate with any OneWire devices 
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

void setup()
{
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
 
  rtc.begin();          // initialize RTC chip
  lcd.begin(20, 4);     // initialize LCD module
  lcd.setCursor(0, 3);  // move cursor to column 0, row 3 (last row)
  lcd.print("Temp:");
 
  // open serial communications and wait for port to open:
  Serial.begin(9600);
  Serial.print("Initializing SD card...");
 
  // initialize the SD card
  if ( !SD.begin() )
    Serial.println("initialization failed!");  // initialization error
 
  else {   // initialization OK
    sd_ok = 1;
    Serial.println("initialization done.");
    if( SD.exists("Log.txt") == 0 )   // test if file with name 'Log.txt' already exists
    {  // create a text file named 'Log.txt'
      Serial.print("\r\nCreate 'Log.txt' file ... ");
      dataLog = SD.open("Log.txt", FILE_WRITE);   // create (&open) file Log.txt
      if(dataLog) {                               // if the file opened okay, write to it:
        Serial.println("OK");
        // write some texts to 'Log.txt' file
        dataLog.println("    DATE    |    TIME  | TEMPERATURE");
        dataLog.println("(dd-mm-yyyy)|(hh:mm:ss)|");
        dataLog.close();   // close the file
      }
      else
        Serial.println("error creating file.");
    }
  }
 
  Serial.println("\r\n    DATE    |    TIME  | TEMPERATURE");
  Serial.println("(dd-mm-yyyy)|(hh:mm:ss)|");
}

// main loop
void loop()
{
  // call sensors.requestTemperatures() to issue a global temperature
 // request to all devices on the bus
  Serial.print(" Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");
  Serial.print("Temperature for Device 1 is: ");
  Serial.println(sensors.getTempCByIndex(0)); // Why "byIndex"? 
  Serial.print("Temperature for Device 2 is: ");
  Serial.println(sensors.getTempCByIndex(1)); // Why "byIndex"? 
  Serial.print("Temperature for Device 3 is: ");
  Serial.println(sensors.getTempCByIndex(2)); // Why "byIndex"? 
  Serial.print("Temperature for Device 4 is: ");
  Serial.println(sensors.getTempCByIndex(3)); // Why "byIndex"?
  
  now = rtc.now();  // read current time and date from the RTC chip
  RTC_display();    // diaplay time & calendar
 
  if( !digitalRead(button1) )  // if B1 is pressed
  if( debounce() )             // call debounce function (make sure B1 is pressed)
  {
    while( debounce() );  // call debounce function (wait for B1 to be released)
 
    byte hour   = edit( now.hour() );         // edit hours
    byte minute = edit( now.minute() );       // edit minutes
    byte day    = edit( now.day() );          // edit date
    byte month  = edit( now.month() );        // edit month
    byte year   = edit( now.year() - 2000 );  // edit year
 
    // write time & date data to the RTC chip
    rtc.adjust(DateTime(2000 + year, month, day, hour, minute, 0));
 
    while(debounce());  // call debounce function (wait for button B1 to be released)
  }
 
  static byte p_second;
  if( (now.second() % 10 == 0) && (p_second != now.second()) )
  {   // read & print temperature value from sensor every 10 seconds
    unsigned int ds18b20_temp;
    char buffer1[12], buffer2[26];
    bool sensor_ok = 0;
    p_second = now.second();
   /* if( ds18b20_read(&ds18b20_temp) )
    {
      sensor_ok = 1;
      if (ds18b20_temp & 0x8000)           // if temperature < 0
      {
        ds18b20_temp = ~ds18b20_temp + 1;  // change temperature value to positive form
        sprintf(buffer1, "-%02u.%04u%cC", (ds18b20_temp/16) % 100, (ds18b20_temp & 0x0F) * 625, 223);
      }
      else
      {      // otherwise (temperature >= 0)
        if (ds18b20_temp/16 >= 100)          // if temperature >= 100.0 °C
          sprintf(buffer1, "%03u.%04u%cC", ds18b20_temp/16, (ds18b20_temp & 0x0F) * 625, 223);
        else      // otherwise ( 0 <= temperature < 100.0)
          sprintf(buffer1, " %02u.%04u%cC", ds18b20_temp/16, (ds18b20_temp & 0x0F) * 625, 223);
      }
    } */
//    else
   //   sprintf(buffer1, "   ERROR  ");
 
    lcd.setCursor(5, 3);
    lcd.print(buffer1);
 
    sprintf( buffer2, " %02u-%02u-%04u | %02u:%02u:%02u | ", now.day(), now.month(), now.year(),
                      now.hour(), now.minute(), now.second() );
    if(sensor_ok) {
      buffer1[8] = 194;   // put degree symbol
      buffer1[9] = 176;
      buffer1[10] = 'C';  // put 'C' letter
      buffer1[11] = '\0'; // put string terminator
    }
    // print data on PC serial monitor
    Serial.print(buffer2);
    Serial.println(buffer1);
    // write data to SD card
    if(sd_ok)
    {  // if the SD card was successfully initialized
      // open Log.txt file with write permission
      dataLog = SD.open("Log.txt", FILE_WRITE);
      dataLog.print( buffer2 );
      dataLog.println( buffer1 );
      dataLog.close();   // close the file
    }
 
  }
 
  delay(100);   // wait 100ms
}

 
//////////////////////////////////////// RTC functions ////////////////////////////////////////
void RTC_display()
{
  char _buffer[17];
  char dow_matrix[7][10] = {" SUNDAY  ", " MONDAY  ", " TUESDAY ", "WEDNESDAY",
                             "THURSDAY ", " FRIDAY  ", "SATURDAY "};
    // print date
    char s[5];
    dtostrf(sensors.getTempCByIndex(0),4,1,s);
  lcd.setCursor(0, 0);
  lcd.print("T1:");
  lcd.setCursor(3,0);
  lcd.print(s);
  
  char s2[5];
  dtostrf(sensors.getTempCByIndex(1),4,1,s2);
  lcd.setCursor(8, 0);
  lcd.print("T2:");
  lcd.setCursor(11,0);
  lcd.print(s2);
 
  
  char s3[5];
  dtostrf(sensors.getTempCByIndex(2),4,1,s3);
  //sprintf(_buffer, "T3:%s", s3);
  lcd.setCursor(0, 1);
  lcd.print("T3:");
  lcd.setCursor(3,1);
  lcd.print(s3);
 
  
  char s4[5];
   dtostrf(sensors.getTempCByIndex(3),4,1,s4);
  //sprintf(_buffer, "T4:%s", s4);
  lcd.setCursor(8, 1);
  lcd.print("T4:");
  lcd.setCursor(11,1);
  lcd.print(s4);
 
  
 // sprintf( _buffer, "DATE: %02u-%02u-%04u", now.day(), now.month(), now.year() );
 //lcd.setCursor(8, 1);
  //lcd.print(_buffer);
  //lcd.setCursor(4, 0);
  //lcd.print( dow_matrix[now.dayOfTheWeek()] );
  // print time
 // sprintf( _buffer, "TIME: %02u:%02u:%02u", now.hour(), now.minute(), now.second() );
  //lcd.setCursor(0, 1);
  //lcd.print(_buffer);
}
 
byte edit(byte parameter)
{
  static byte i = 0, y_pos,
              x_pos[5] = {6, 9, 6, 9, 14};
  char text[3];
  sprintf(text,"%02u", parameter);
 
  if(i < 2)
    y_pos = 1;
  else
    y_pos = 2;
 
  while( debounce() );   // call debounce function (wait for B1 to be released)
 
  while(true) {
    while( !digitalRead(button2) ) {  // while B2 is pressed
      parameter++;
      if(i == 0 && parameter > 23)    // if hours > 23 ==> hours = 0
        parameter = 0;
      if(i == 1 && parameter > 59)    // if minutes > 59 ==> minutes = 0
        parameter = 0;
      if(i == 2 && parameter > 31)    // if day > 31 ==> day = 1
        parameter = 1;
      if(i == 3 && parameter > 12)    // If month > 12 ==> month = 1
        parameter = 1;
      if(i == 4 && parameter > 99)    // If year > 99 ==> year = 0
        parameter = 0;
 
      sprintf(text,"%02u", parameter);
      lcd.setCursor(x_pos[i], y_pos);
      lcd.print(text);
      delay(200);       // wait 200ms
    }
 
    lcd.setCursor(x_pos[i], y_pos);
    lcd.print("  ");
    unsigned long previous_m = millis();
    while( (millis() - previous_m < 250) && digitalRead(button1) && digitalRead(button2) ) ;
    lcd.setCursor(x_pos[i], y_pos);
    lcd.print(text);
    previous_m = millis();
    while( (millis() - previous_m < 250) && digitalRead(button1) && digitalRead(button2) ) ;
 
    if(!digitalRead(button1))
    {                     // if button B1 is pressed
      i = (i + 1) % 5;    // increment 'i' for the next parameter
      return parameter;   // return parameter value and exit
    }
  }
}
 
// a small function for button1 (B1) debounce
bool debounce ()
{
  byte count = 0;
  for(byte i = 0; i < 5; i++)
  {
    if ( !digitalRead(button1) )
      count++;
    delay(10);
  }
 
  if(count > 2)  return 1;
  else           return 0;
}
////////////////////////////////////// end RTC functions //////////////////////////////////////
 
////////////////////////////////// DS18B20 sensor functions ///////////////////////////////////
/*
bool ds18b20_start()
{
  bool ret = 0;
  digitalWrite(DS18B20_PIN, LOW);  // send reset pulse to the DS18B20 sensor
  pinMode(DS18B20_PIN, OUTPUT);
  delayMicroseconds(500);          // wait 500 us
  pinMode(DS18B20_PIN, INPUT);
  delayMicroseconds(100);          // wait to read the DS18B20 sensor response
  if (!digitalRead(DS18B20_PIN))
  {
    ret = 1;                  // DS18B20 sensor is present
    delayMicroseconds(400);   // wait 400 us
  }
  return(ret);
} */

 /*
void ds18b20_write_bit(bool value)
{
  digitalWrite(DS18B20_PIN, LOW);
  pinMode(DS18B20_PIN, OUTPUT);
  delayMicroseconds(2);
  digitalWrite(DS18B20_PIN, value);
  delayMicroseconds(80);
  pinMode(DS18B20_PIN, INPUT);
  delayMicroseconds(2);
} */
 /*
void ds18b20_write_byte(byte value)
{
  byte i;
  for(i = 0; i < 8; i++)
    ds18b20_write_bit(bitRead(value, i));
} */

 /*
bool ds18b20_read_bit(void)
{
  bool value;
  digitalWrite(DS18B20_PIN, LOW);
  pinMode(DS18B20_PIN, OUTPUT);
  delayMicroseconds(2);
  pinMode(DS18B20_PIN, INPUT);
  delayMicroseconds(5);
  value = digitalRead(DS18B20_PIN);
  delayMicroseconds(100);
  return value;
} */
 /*
byte ds18b20_read_byte(void)
{
  byte i, value;
  for(i = 0; i < 8; i++)
    bitWrite(value, i, ds18b20_read_bit());
  return value;
}
 
bool ds18b20_read(int *raw_temp_value)
{
  if (!ds18b20_start())  // send start pulse
    return(0);
  ds18b20_write_byte(0xCC);   // send skip ROM command
  ds18b20_write_byte(0x44);   // send start conversion command
  while(ds18b20_read_byte() == 0);  // wait for conversion complete
  if (!ds18b20_start())             // send start pulse
    return(0);                      // return 0 if error
  ds18b20_write_byte(0xCC);         // send skip ROM command
  ds18b20_write_byte(0xBE);         // send read command
 
  // read temperature LSB byte and store it on raw_temp_value LSB byte
  *raw_temp_value = ds18b20_read_byte();
  // read temperature MSB byte and store it on raw_temp_value MSB byte
  *raw_temp_value |= (unsigned int)(ds18b20_read_byte() << 8);
 
  return(1);  // OK --> return 1
}
*/
//////////////////////////////////// end DS18B20 functions ////////////////////////////////////
 
// end of code.
