#include <Arduino.h>
#include <SensirionI2CScd4x.h>
#include "Seeed_BME280.h"
#include <Wire.h>

SensirionI2CScd4x scd4x;
BME280 bme280;

//---------------------------- Constant -------------------------
#define VERBOSE true
#define LN true
#define NO_LN false


//---------------------------------------------------------------

//--------------------------- Custom log ------------------------

/**
 * @brief Print a message on the serial port
 * 
 * @param message the message to print
 * @param ln if true, print a new line at the end of the message
 * @param bypassVerbose if true, bypass the verbose mode and print the message
 */
void log(String message = "", bool ln = false, bool bypassVerbose = false){
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
void log(float number, bool ln = false, bool bypassVerbose = false){
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
void log(int number, bool ln = false, bool bypassVerbose = false){
  if(bypassVerbose || VERBOSE){
    if(ln){
      Serial.println(number);
    }else{
      Serial.print(number);
    }
  }
}



//---------------------------------------------------------------



//---------------------------- SCD Functions --------------------
void printUint16Hex(uint16_t value) {
    log(value < 4096 ? "0" : "",NO_LN);
    log(value < 256 ? "0" : "",NO_LN);
    log(value < 16 ? "0" : "",NO_LN);
    if(VERBOSE){
      Serial.print(value, HEX);
    }
}

void printSerialNumber(uint16_t serial0, uint16_t serial1, uint16_t serial2) {
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

  uint16_t serial0;
  uint16_t serial1;
  uint16_t serial2;
  error = scd4x.getSerialNumber(serial0, serial1, serial2);
  if (error) {
      log("Error trying to execute getSerialNumber(): ", NO_LN);
      errorToString(error, errorMessage, 256);
      log(errorMessage, LN);
  } else {
      printSerialNumber(serial0, serial1, serial2);
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
void printMeasurementSCD( uint16_t & co2, float & temperature, float & humidity){
  log("Co2:");
  log(String(co2));
  log("\t");
  log("Temperature:");
  log(temperature);
  log("\t");
  log("Humidity:");
  log(humidity, LN);
}

/**
 * @brief Read the SCD4x measurements from the sensor
 * 
 * @param co2 the CO2 concentration variable to fill
 * @param temperature the temperature variable to fill
 * @param humidity the humidity variable to fill
 * @return true if the measurements are valid
 * @return false if the measurements are invalid
 */
bool readMeasurementSCD(uint16_t & co2, float & temperature, float & humidity){
  uint16_t error;
  char errorMessage[256];

  error = scd4x.readMeasurement(co2, temperature, humidity);
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
void updatePressureSCD(uint16_t pressure){
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
void printMeasurementBME280(float & temp, float & pressure, uint32_t & humidity, float & altitude){
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
 * @brief read the temperature, pressure and humidity from the BME280 sensor. Will print the data on the serial port
 * 
 * @param pressure variable to store the pressure data
 */
void readMeasurementBME280(float & pressure){

  float temp;
  uint32_t humidity;
  float altitude;

  temp = bme280.getTemperature();
  pressure = bme280.getPressure();
  altitude = bme280.calcAltitude(pressure);
  humidity = bme280.getHumidity();

  printMeasurementBME280(temp,pressure,humidity,altitude);

}
//--------------------------------------------------------------

//-------------------------- LEDS ------------------------------

/**
 * @brief Setup the LED pins
 */
void setupLEDS(){
  pinMode(PD2,OUTPUT);
}

/**
 * @brief Update the state of the LEDs based on the current co2 value
 * 
 * @param co2 the CO2 concentration
 */
void updateLEDS(float co2){
  if(co2 >= 1000){
    digitalWrite(PD2, HIGH);
  } else {
    digitalWrite(PD2, LOW);
  }
}
//--------------------------------------------------------------


void setup() {

  Serial.begin(115200);
  while (!Serial) {
      delay(100);
  }
  log("starting setup..", LN);

  Wire.begin();

  if(!bme280.init()){
    log("Device BME280 error!", LN);
  }

  setupSCD();
  setupLEDS();
    
}

void loop() {
  delay(5000);

  // bme280 variables
  float pressureBME = 0;

  // scd variables
  uint16_t co2SCD = 0;
  float tempSCD = 0;
  float humiditySCD = 0;

  bool success = false;

  // Will also print it
  readMeasurementBME280(pressureBME);

  // We update the pressure
  if(pressureBME != 0){
    pressureBME = pressureBME / 100;
    updatePressureSCD(pressureBME);
  }

  success = readMeasurementSCD(co2SCD,tempSCD,humiditySCD);

  if(success){
    if (co2SCD == 0) {
      log("Invalid sample detected, skipping.", LN);
    } else {
      printMeasurementSCD(co2SCD, tempSCD, humiditySCD);
      updateLEDS(co2SCD);
    }
  }

}
