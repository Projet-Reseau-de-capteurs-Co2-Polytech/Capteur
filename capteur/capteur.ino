#include <Arduino.h>
#include <SensirionI2CScd4x.h>
#include "Seeed_BME280.h"
#include <Wire.h>
#include <EEPROM.h>

SensirionI2CScd4x scd4x;
BME280 bme280;

uint16_t serial0;
uint16_t serial1;
uint16_t serial2;


//---------------------------- Parameters -----------------------
/**
 * @brief true if you want to log eveything, false otherwise
 * useful for debug
 * false will only log the messages that bypass this
 */
#define VERBOSE false

/**
 * @brief leave empty or set to "null" to use the serial number. Else, use it to have a custom sensor name
 * Cannot have more than 15 characters 
 */
String SENSOR_NAME = "";

/**
 * @brief true if you want to change the name, false if you want to use the saved name
 * please only put true when you need to change the name, and put it back to false afterward, because it uses the EEPROM and it has a limited rewrite capacity
 */
#define NEW_NAME false

/**
 * @brief number of minutes to wait between two measure (will send the data at the end of this period
 */
#define MINUTES_TO_WAIT_TOTAL 10


/**
 * @brief Green led minimum level
 */
#define GREEN_LED_MIN_PPM 0

/**
 * @brief Orange led minimum level
 */
#define ORANGE_LED_MIN_PPM 800

/**
 * @brief Red led minimum level
 */
#define RED_LED_MIN_PPM 1000
//---------------------------------------------------------------


//---------------------------- Constant -------------------------

/**
 * Both constant used to say if you want a new line or not
 */
#define LN true
#define NO_LN false

#define SECOND 1000
#define MINUTE 60000

/**
 * Max amount of character (not including the \0)
 */
#define SENSOR_NAME_MAX_LENGTH 12

//---------------------------------------------------------------

//--------------------------- Custom log ------------------------

/**
 * @brief Print a message on the serial port
 * 
 * @param message the message to print
 * @param ln if true, print a new line at the end of the message
 * @param bypassVerbose if true, bypass the verbose mode and print the message
 */
void log(const String & message = "", bool ln = false, bool bypassVerbose = false) {
  if(bypassVerbose || VERBOSE){
    if(ln){
      Serial.println(message);
    }else{
      Serial.print(message);
    }
  }
}

/**
 * @brief Print a message on the serial port
 * 
 * @param number the number to print
 * @param ln if true, print a new line at the end of the message
 * @param bypassVerbose if true, bypass the verbose mode and print the message
 */
void log(float number, bool ln = false, bool bypassVerbose = false) {
  if(bypassVerbose || VERBOSE){
    if(ln){
      Serial.println(number);
    }else{
      Serial.print(number);
    }
  }
}

/**
 * @brief Print a message on the serial port
 * 
 * @param number the number to print
 * @param ln if true, print a new line at the end of the message
 * @param bypassVerbose if true, bypass the verbose mode and print the message
 */
void log(int number, bool ln = false, bool bypassVerbose = false) {
  if(bypassVerbose || VERBOSE){
    if(ln){
      Serial.println(number);
    }else{
      Serial.print(number);
    }
  }
}



//---------------------------------------------------------------


//---------------------------- EEPROM ---------------------------
/**
 * @brief write a string to the EEPROM
 * @param addrOffset the memory address
 * @param strToWrite the string to write
 */
void writeStringToEEPROM(int addrOffset, const String & strToWrite)
{
  byte len = strToWrite.length();
  EEPROM.write(addrOffset, len);
  for (int i = 0; i < len; i++)
  {
    EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
  }
}

/**
 * @brief read a string from the EEPROM
 * @param addrOffset the memory address
 * @return the string stored
 */
String readStringFromEEPROM(int addrOffset)
{
  int newStrLen = EEPROM.read(addrOffset);
  char data[newStrLen + 1];
  for (int i = 0; i < newStrLen; i++)
  {
    data[i] = EEPROM.read(addrOffset + 1 + i);
  }
  data[newStrLen] = '\0';
  return String(data);
}

//---------------------------------------------------------------


//---------------------------- SCD Functions --------------------
void printUint16Hex(uint16_t & value) {
    log(value < 4096 ? "0" : "",NO_LN);
    log(value < 256 ? "0" : "",NO_LN);
    log(value < 16 ? "0" : "",NO_LN);
    if(VERBOSE){
      Serial.print(value, HEX);
    }
}

void printSerialNumber() {
    log("Serial: 0x", NO_LN);
    printUint16Hex(serial0);
    printUint16Hex(serial1);
    printUint16Hex(serial2);
    log();
}

/**
 * @brief Setup the SCD4x, print the serial number and start the periodic measurements
 */
void setupSCD() {
  uint16_t error;
  char errorMessage[256];

  scd4x.begin(Wire);

  // stop potentially previously started measurement
  error = scd4x.stopPeriodicMeasurement();
  if (error) {
      log("Error trying to execute stopPeriodicMeasurement(): ", NO_LN);
      errorToString(error, errorMessage, 256);
      log(errorMessage, LN);
  }
  
  error = scd4x.getSerialNumber(serial0, serial1, serial2);
  if (error) {
      log("Error trying to execute getSerialNumber(): ", NO_LN);
      errorToString(error, errorMessage, 256);
      log(errorMessage, LN);
  } else {
      printSerialNumber();
  }

  // Start Measurement
  error = scd4x.startPeriodicMeasurement();
  if (error) {
      log("Error trying to execute startPeriodicMeasurement(): ", NO_LN);
      errorToString(error, errorMessage, 256);
      log(errorMessage, LN);
  }

  log("Waiting for first measurement... (5 sec)", LN);
}

/**
 * @brief Print the SCD4x measurements
 * 
 * @param co2 the CO2 concentration
 * @param temperature the temperature
 * @param humidity the humidity
 */
void printMeasurementSCD( uint16_t & co2) {
  log("Co2:");
  log(String(co2), LN);
}

/**
 * @brief Read the SCD4x measurements from the sensor
 * 
 * @param co2 the CO2 concentration variable to fill
 * @return true if the measurements are valid
 * @return false if the measurements are invalid
 */
bool readMeasurementSCD(uint16_t & co2) {
  uint16_t error;
  char errorMessage[256];
  float temp = 0;
  float hum = 0;

  error = scd4x.readMeasurement(co2, temp, hum);
  if (error) {
    log("Error trying to execute readMeasurement(): ", LN);
    errorToString(error, errorMessage, 256);
    log(errorMessage, LN);
    return false;
  }
  return true;
}

/**
 * @brief Update the pressure for the SCD4x sensor
 * 
 * @param pressure the new pressure
 */
void updatePressureSCD(uint16_t pressure) {
  uint16_t error;
  char errorMessage[256];

  error = scd4x.setAmbientPressure(pressure);
  if (error) {
    log("Error trying to execute setAmbientPressure(): ", LN);
    errorToString(error, errorMessage, 256);
    log(errorMessage, LN);
  }

}

//--------------------------------------------------------------

//----------------------------- BME280 functions ---------------

/**
 * @brief Print the data of the BME280 sensor
 * 
 * @param temp Temperature
 * @param pressure Pressure
 * @param humidity Humidity
 * @param altitude Altitude
 */
void printMeasurementBME280(float & temp, float & pressure, uint32_t & humidity, float & altitude) {
  //get and print temperatures
  log("Temp: ");
  log(temp);
  log("C\t");//The unit for  Celsius because original arduino don't support speical symbols
  
  //get and print atmospheric pressure data
  log("Pressure: ");
  log(pressure);
  log("Pa\t");

  //get and print altitude data
  log("Altitude: ");
  log(altitude);
  log("m\t");
  

  //get and print humidity data
  log("Humidity: ");
  log(float(humidity));
  log("%\t", LN);
}

/**
 * @brief Print the pressure of the BME280 sensor
 * 
 * @param pressure Pressure
 */
void printPressureBME280(float & pressure) {
  //print atmospheric pressure data
  log("Pressure: ");
  log(pressure);
  log("Pa\t");
}

/**
 * @brief read the temperature, pressure and humidity from the BME280 sensor. Will print the data on the serial port
 * 
 * @param pressure variable to store the pressure data
 */
void readMeasurementBME280(float & pressure) {
  pressure = bme280.getPressure();
}
//--------------------------------------------------------------

//-------------------------- LEDS ------------------------------

/**
 * @brief Setup the LED pins
 */
void setupLEDS() {
  //PDX <=> PortD X <=> DX on the board
  pinMode(PD2,OUTPUT);
  pinMode(PD3,OUTPUT);
  pinMode(PD4,OUTPUT);

  //Built-in led for some error display
  pinMode(LED_BUILTIN, OUTPUT);

}

/**
 * @brief Update the state of the LEDs based on the current co2 value
 * 
 * @param co2 the CO2 concentration
 */
void updateLEDS(const float & co2) {
  digitalWrite(PD2, LOW);
  digitalWrite(PD3, LOW);
  digitalWrite(PD4, LOW);


  if(co2 >= RED_LED_MIN_PPM){
    digitalWrite(PD4, HIGH);
  } else if(co2 >= ORANGE_LED_MIN_PPM){
    digitalWrite(PD3, HIGH);
  } else if(co2 >= GREEN_LED_MIN_PPM){
    digitalWrite(PD2, HIGH);
  }
}
//--------------------------------------------------------------

//-------------------------- XBee ------------------------------

void printUint16HexXBee(uint16_t & value) {
    Serial.print(value < 4096 ? "0" : "");
    Serial.print(value < 256 ? "0" : "");
    Serial.print(value < 16 ? "0" : "");
    Serial.print(value, HEX);
}

/**
 * @brief Send the data to the XBee using the serial port
 * 
 * Data will use the following format:
 * SensorID;CO2
 * 
 * Currently uses the serial port to send the data, but the port might change in the future to a software serial port
 * @param co2 
 */
void sendData(uint16_t & co2) {

  if(SENSOR_NAME.equals("")){ 
    printUint16HexXBee(serial0);
    printUint16HexXBee(serial1);
    printUint16HexXBee(serial2);
  }else{
    Serial.print(SENSOR_NAME);
  }
  

  Serial.print(";");
  
  Serial.println(co2);
}

//--------------------------------------------------------------

void setup() {

  Serial.begin(115200);
  while (!Serial) {
      delay(100);
  }
  log("starting setup..", LN);

  if(NEW_NAME){
    if(SENSOR_NAME.length() > SENSOR_NAME_MAX_LENGTH){
        log("Error : new SENSOR_NAME cannot have more than 15 characters");
        while(true){
          digitalWrite(LED_BUILTIN, HIGH);
          delay(500);
          digitalWrite(LED_BUILTIN, LOW);
          delay(500);
        }
    }

    if(SENSOR_NAME.length() > 0 && !SENSOR_NAME.equals("null")){
      writeStringToEEPROM(0,SENSOR_NAME);
    }else{
      writeStringToEEPROM(0,"null");
      SENSOR_NAME = "";
    }

  }else{
    String eeprom_name = readStringFromEEPROM(0);
    if(eeprom_name.equals("null")){
      SENSOR_NAME = "";
    }else{
      SENSOR_NAME = eeprom_name;
    }
  }

  

  Wire.begin();

  if(!bme280.init()){
    log("Device BME280 error!", LN);
  }

  setupSCD();
  setupLEDS();
  // delay needed to be sure scd is set up
  delay(5000);
}


void loop() {
  int minutePassed = 9;

  // bme280 variables
  float pressureBME;

  // scd variables
  uint16_t co2SCD;

  bool success;


  do{
    pressureBME = 0;
    co2SCD = 0;
    success = false;

    readMeasurementBME280(pressureBME);

    // We update the pressure
    if(pressureBME != 0){
      pressureBME = pressureBME / 100;
      printPressureBME280(pressureBME);
      updatePressureSCD(pressureBME);
    }

    success = readMeasurementSCD(co2SCD);

    if(!success){
      log("Error trying to read CO2 level, skipping.", LN);
      continue; // we don't increment the minutePassed because we don't want to send wrong data
    }else{
      printMeasurementSCD(co2SCD);
      if (co2SCD == 0) {
        log("Invalid sample detected, skipping.", LN);
        continue; // we don't increment the minutePassed because we don't want to send wrong data
      }
      
    }

    updateLEDS(co2SCD);
    minutePassed++;

    if(minutePassed >= MINUTES_TO_WAIT_TOTAL){
      sendData(co2SCD);
    }

    delay(MINUTE);
    
  }while( minutePassed < MINUTES_TO_WAIT_TOTAL );
  
}
