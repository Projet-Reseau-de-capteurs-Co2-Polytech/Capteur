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
void log(String message = "", bool ln = false, bool bypassVerbose = false){
  if(bypassVerbose || VERBOSE){
    if(ln){
      Serial.println(message);
    }else{
      Serial.print(message);
    }
  }
}

void log(float number, bool ln = false, bool bypassVerbose = false){
  if(bypassVerbose || VERBOSE){
    if(ln){
      Serial.println(number);
    }else{
      Serial.print(number);
    }
  }
}

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

void readMeasurementSCD(){
  uint16_t error;
  char errorMessage[256];

  uint16_t co2;
  float temperature;
  float humidity;
  error = scd4x.readMeasurement(co2, temperature, humidity);
  if (error) {
    log("Error trying to execute readMeasurement(): ", LN);
    errorToString(error, errorMessage, 256);
    log(errorMessage, LN);
  } else if (co2 == 0) {
    log("Invalid sample detected, skipping.", LN);
  } else {
    printMeasurementSCD(co2, temperature, humidity);
  }
}

//--------------------------------------------------------------

//----------------------------- BME280 functions ---------------

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

void readMeasurementBME280(){
  float temp = 0;
  float pressure = 0;
  float altitude = 0;
  uint32_t humidity = 0;

  temp = bme280.getTemperature();
  pressure = bme280.getPressure();
  altitude = bme280.calcAltitude(pressure);
  humidity = bme280.getHumidity();

  printMeasurementBME280(temp,pressure,humidity,altitude);

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

    
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(5000);
  readMeasurementBME280();
  readMeasurementSCD();
}
