// Define hardware connections
#define PIN_ANALOG_IN A0
#define RTC_I2C_ADDRESS 0x68 // // I2C adress of DS3231

//include libraries
#include <Wire.h>
#include <SD.h>

File myFile;

int counter;
int secondsCounted;

boolean RTCinit()
{
  Wire.begin();
  delay(10); // small delay to stabilize I2C bus voltage
  Wire.beginTransmission(RTC_I2C_ADDRESS);
  Wire.write(0);
  return (Wire.endTransmission()==0);
}

//Define digits
char HIGHDIGIT ( byte b ) {
   return '0' + (b>>4);  
}
char LOWDIGIT ( byte b ) {
   return '0' + (b&0x0F);  
}

byte bcdSec, bcdMin, bcdHr;

void setup()
{
  Serial.begin(9600);
  //SD
  Serial.print("Initializing SD card...");
  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin 
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output 
  // or the SD library functions will not work. 
  pinMode(10, OUTPUT);
 
  if (!SD.begin(10)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
  
  //RTC Initialization
  if (RTCinit()) Serial.println(F("RTC OK"));
  else { 
    Serial.println(F("RTC/I2C ERROR"));
    while (true) { ; } // Useless to continue 
  }
  Wire.beginTransmission(RTC_I2C_ADDRESS);
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(RTC_I2C_ADDRESS, 1);
  if (Wire.read() & 0x80) {
    Serial.println(F("RTC WAS STOPPED, STARTING AT 00:00"));
    Wire.beginTransmission(RTC_I2C_ADDRESS);
    Wire.write(0);
    Wire.write(bcdSec);
    Wire.write(bcdMin);
    Wire.write(bcdHr);
    Wire.endTransmission();  
  }
}

//millis
long previousMillis = 0;
const long measuringInterval = 100;

//state change detection for save function
int lastValue = HIGHDIGIT(bcdSec);

//make array for measurements


void loop()
{
  int value;
  unsigned long currentMillis = millis();
  // Check the envelope input
  value = analogRead(PIN_ANALOG_IN);
  // Convert envelope value into a message
  if (currentMillis - previousMillis >= measuringInterval) {
    
      previousMillis = currentMillis;
      
      //if(value > 10) {
      //  ++counter;
      //}
      
      counter += value;
      
  
  //Print RTC Value
  Wire.beginTransmission(RTC_I2C_ADDRESS);
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(RTC_I2C_ADDRESS, 3);
  bcdSec = Wire.read();
  bcdMin = Wire.read();
  bcdHr = Wire.read();
  
  if(HIGHDIGIT(bcdSec) != lastValue) {
    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    myFile = SD.open("test.txt", FILE_WRITE);
    
    //write variable counter and timestamp to card
    if (myFile) {
    Serial.print("Writing to test.txt...");
    myFile.print(counter);
    myFile.print(';');
    myFile.write(HIGHDIGIT(bcdHr));
    myFile.write(LOWDIGIT(bcdHr));
    myFile.write(':');
    myFile.write(HIGHDIGIT(bcdMin));
    myFile.write(LOWDIGIT(bcdMin));
    myFile.write(':');
    myFile.write(HIGHDIGIT(bcdSec));
    myFile.write(LOWDIGIT(bcdSec));
    myFile.println();
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
    
    //set back counter
    counter = 0;
    //secondsCounted = 0;
    lastValue = HIGHDIGIT(bcdSec);
  }
  
  //print counter
  Serial.print(counter);
  Serial.print('-');
  //print time
  Serial.write(HIGHDIGIT(bcdHr));
  Serial.write(LOWDIGIT(bcdHr));
  Serial.write(':');
  Serial.write(HIGHDIGIT(bcdMin));
  Serial.write(LOWDIGIT(bcdMin));
  Serial.write(':');
  Serial.write(HIGHDIGIT(bcdSec));
  Serial.write(LOWDIGIT(bcdSec));
  //print seconds counted
  Serial.print('-');
  Serial.print(secondsCounted);
  //print line break
  Serial.println();
  }
}
