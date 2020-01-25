#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include <MySQL_Encrypt_Sha1.h>
#include <MySQL_Packet.h>
#include <Ethernet.h>
#include <SPI.h>
#include <SD.h>
#include <max6675.h>
#include "Wire.h"
#define DS3231_I2C_ADDRESS 0x68

byte mac_addr[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}; //Ethernet board MAC address

IPAddress server_addr(XXX, XXX, XXX, XXX); //MySQL server IP address
IPAddress ip (XXX, XXX, XXX, XXX); //Ethernet board IP address
char user[] = "root"; //MySQL login username
char password[] = "root"; //MySQL login password
String sql_command[256];
char buffer[256];
String value;

EthernetClient client; //Ethernet connection
MySQL_Connection conn((Client *)&client); //MySQL connection
MySQL_Cursor cur = MySQL_Cursor(&conn);

int soPin = 5; //SO = Serial Out pin
int csPin = 6 ; //CS = Chip Select CS pin
int sckPin = 7; //CK = Serial Clock pin

int soPin_2 = 22;
int csPin_2 = 2;
int sckPin_2 = 3;

int soPin_3 = 23;
int csPin_3 = 24;
int sckPin_3 = 25;

int soPin_4 = 26;
int csPin_4 = 28;
int sckPin_4 = 30;

int soPin_5 = 32;
int csPin_5 = 34;
int sckPin_5 = 36;

int soPin_6 = 38;
int csPin_6 = 40;
int sckPin_6 = 42;

int soPin_7 = 44;
int csPin_7 = 46;
int sckPin_7 = 48;

int soPin_8 = 27;
int csPin_8 = 29;
int sckPin_8 = 31;

int soPin_9 = 33;
int csPin_9 = 35;
int sckPin_9 = 37;

int soPin_10 = 39;
int csPin_10 = 41;
int sckPin_10 = 43;

int soPin_11 = 45;
int csPin_11 = 47;
int sckPin_11 = 49;

byte decToBcd(byte val)
{
  return ( (val / 10 * 16) + (val % 10) );
}
byte bcdToDec(byte val)
{
  return ( (val / 16 * 10) + (val % 16) );
}

MAX6675 robojax(sckPin, csPin, soPin); //Create instance object of MAX6675
MAX6675 robojax2(sckPin_2, csPin_2, soPin_2);
MAX6675 robojax3(sckPin_3, csPin_3, soPin_3);
MAX6675 robojax4(sckPin_4, csPin_4, soPin_4);
MAX6675 robojax5(sckPin_5, csPin_5, soPin_5);
MAX6675 robojax6(sckPin_6, csPin_6, soPin_6);
MAX6675 robojax7(sckPin_7, csPin_7, soPin_7);
MAX6675 robojax8(sckPin_8, csPin_8, soPin_8);
MAX6675 robojax9(sckPin_9, csPin_9, soPin_9);
MAX6675 robojax10(sckPin_10, csPin_10, soPin_10);
MAX6675 robojax11(sckPin_11, csPin_11, soPin_11);

File tmp;

unsigned long old_time;
const long delay_time = 10000;

void setup()
{
  Wire.begin();
  //setDS3231time(00, 57, 16, 3, 22, 01, 20); //Clock update setting: sec, min, hour, days of week, day, month, year.
  Serial.begin(115200);
  Ethernet.begin(mac_addr, ip); //Ethernet begin

  while (!Serial); //wait for serial port to connect
  Serial.println("Connecting...");

  if (conn.connect(server_addr, 3310, user, password))
  {
    Serial.println("connected succesfully");
  }

  else {
    Serial.println("Connection failed.");
    delay(500);
    Serial.println("Searching SD card...");

    if (!SD.begin(4))
    {
      Serial.println("SD Card couldnt find.");
      Serial.println("Please Check SD card and Ethernet connection then press RESET");

      while (!SD.begin(4));
      Serial.println("SD card is ready.");
      delay(1000);
    }

    else
    {
      Serial.println("SD card is ready.");
      delay(500);
    }
    SD.mkdir("Data_Log"); //Create the folder for the measurements.
  }

}

void setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte
                   dayOfMonth, byte month, byte year)
{
  //Set time and date data to DS3231
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); //Set next input to start at the seconds register
  Wire.write(decToBcd(second)); //Set sec
  Wire.write(decToBcd(minute)); //Set min
  Wire.write(decToBcd(hour)); //Set hour
  Wire.write(decToBcd(dayOfWeek)); //Set days of week (1=Sunday, 7=Saturday)
  Wire.write(decToBcd(dayOfMonth)); //Set day (1 to 31)
  Wire.write(decToBcd(month)); //Set month
  Wire.write(decToBcd(year)); //Set year (0 to 99)
  Wire.endTransmission();
}

void readDS3231time(byte *second,
                    byte *minute,
                    byte *hour,
                    byte *dayOfWeek,
                    byte *dayOfMonth,
                    byte *month,
                    byte *year)
{
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); //Set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7); //Request seven bytes of data from DS3231 starting from register 00h
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
}

void displayTime()
{
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year; // Retrieve data from DS3231
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year); //Send it to the serial monitor
  tmp.print(hour, DEC); //Convert the byte variable to a decimal number when displayed
  tmp.print(":");

  if (minute < 10)
  {
    tmp.print("0");
  }
  tmp.print(minute, DEC);
  tmp.print(":");

  if (second < 10)
  {
    tmp.print("0");
  }
  tmp.print(second, DEC);
  tmp.print(",");
  tmp.print(dayOfMonth, DEC);
  tmp.print("/");
  tmp.print(month, DEC);
  tmp.print("/");
  tmp.print(year, DEC);
  tmp.print(",");

  switch (dayOfWeek) {
    case 1:
      tmp.print("Sunday");
      break;
    case 2:
      tmp.print("Monday");
      break;
    case 3:
      tmp.print("Tuesday");
      break;
    case 4:
      tmp.print("Wednesday");
      break;
    case 5:
      tmp.print("Thursday");
      break;
    case 6:
      tmp.print("Friday");
      break;
    case 7:
      tmp.print("Saturday");
      break;
  }
}

void loop()
{
  unsigned long now_time = millis();

  if (now_time - old_time >= delay_time)
  {
    old_time = now_time ;

    float c1 = robojax.readCelsius();
    float c2 = robojax2.readCelsius();
    float c3 = robojax3.readCelsius();
    float c4 = robojax4.readCelsius();
    float c5 = robojax5.readCelsius();
    float c6 = robojax6.readCelsius();
    float c7 = robojax7.readCelsius();
    float c8 = robojax8.readCelsius();
    float c9 = robojax9.readCelsius();
    float c10 = robojax10.readCelsius();
    float c11 = robojax11.readCelsius();

    if (conn.connect(server_addr, 3310, user, password)) {
      delay(500);
      Serial.println("Recording data.");

      sql_command[0] = "insert into datalogger.TemperatureLog (Channel1, Channel2, Channel3, Channel4, Channel5, Channel6, Channel7, Channel8, Channel9, Channel10, Channel11) values ('";
      sql_command[1] = c1;
      sql_command[2] = "', '";
      sql_command[3] = c2;
      sql_command[4] = "', '";
      sql_command[5] = c3;
      sql_command[6] = "', '";
      sql_command[7] = c4;
      sql_command[8] = "', '";
      sql_command[9] = c5;
      sql_command[10] = "', '";
      sql_command[11] = c6;
      sql_command[12] = "', '";
      sql_command[13] = c7;
      sql_command[14] = "', '";
      sql_command[15] = c8;
      sql_command[16] = "', '";
      sql_command[17] = c9;
      sql_command[18] = "', '";
      sql_command[19] = c10;
      sql_command[20] = "', '";
      sql_command[21] = c11;
      sql_command[22] = "')";
      sql_command[23] = ";";

      value = sql_command[0] + sql_command[1] + sql_command[2] + sql_command[3] + sql_command[4] + sql_command[5] + sql_command[6] + sql_command[7] + sql_command[8] + sql_command[9] + sql_command[10] + sql_command[11] + sql_command[12] + sql_command[13] + sql_command[14] + sql_command[15] + sql_command[16] + sql_command[17] + sql_command[18] + sql_command[19] + sql_command[20] + sql_command[21] + sql_command[22] + sql_command[23];

      value.toCharArray(buffer, 256);
      delay(500);
      Serial.println(buffer);
      MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
      cur_mem->execute(buffer); //Since there are no results, we do not need to read any data. Deleting the cursor also frees up memory used
      delete cur_mem;
    }

    else {
      Serial.println("Connection failed.");
      delay(500);
      Serial.println("Searching SD card...");

      if (!SD.begin(4))
      {
        Serial.println("SD Card couldnt find.");
        Serial.println("Please insert an SD card.");
      }

      else {
        (!SD.begin(4));
        Serial.println("SD card is ready.");
        delay(500);
      }

      tmp = SD.open("Data_Log/Data_Log.csv", FILE_WRITE);

      if (tmp)
      {
        displayTime();
        tmp.print(",");
        tmp.print(robojax.readCelsius());
        tmp.print(",");
        Serial.println( robojax.readCelsius());

        tmp.print(robojax2.readCelsius());
        tmp.print(",");
        Serial.println(robojax2.readCelsius());

        tmp.print(robojax3.readCelsius());
        tmp.print(",");
        Serial.println( robojax3.readCelsius());

        tmp.print(robojax4.readCelsius());
        tmp.print(",");
        Serial.println(robojax4.readCelsius());

        tmp.print(robojax5.readCelsius());
        tmp.print(",");
        Serial.println(robojax5.readCelsius());

        tmp.print(robojax6.readCelsius());
        tmp.print(",");
        Serial.println(robojax6.readCelsius());

        tmp.print(robojax7.readCelsius());
        tmp.print(",");
        Serial.println(robojax7.readCelsius());

        tmp.print(robojax8.readCelsius());
        tmp.print(",");
        Serial.println(robojax8.readCelsius());

        tmp.print(robojax9.readCelsius());
        tmp.print(",");
        Serial.println(robojax9.readCelsius());

        tmp.print(robojax10.readCelsius());
        tmp.print(",");
        Serial.println(robojax10.readCelsius());

        tmp.print(robojax11.readCelsius());
        tmp.print(",");
        Serial.println(robojax11.readCelsius());
        tmp.println();
      }
      tmp.close();
    }
  }
}
