/*
  Example code for the pH to I2C module v2.0
  Works in Arduino IDE 1.6.7
  http://www.cyber-plant.com
  by CyberPlant LLC, 14 November 2015
  This example code is in the public domain.
  upd. 27.01.2016
*/

#include "Wire.h"
#include <EEPROM.h>
#define pHtoI2C 0x48
#define T 273.15                    // degrees Kelvin

float data, voltage, pH, temp;
int tempManual = 25;

const unsigned long Interval = 250;  // 1/4 second
long previousMillis = 0;
unsigned long Time;

int incomingByte = 0;

float IsoP;
float AlphaL;
float AlphaH;

void setup()
{
  Wire.begin();
  Serial.begin(9600);
  Read_EE();

  Serial.println("Calibrate commands:");
  Serial.println("pH :");
  Serial.println("      Cal. pH 4.00 ---- 4");
  Serial.println("      Cal. pH 7.00 ---- 7");
  Serial.println("      Cal. pH 10.00 --- 9");
  Serial.println("      Reset pH ---------8");
  Serial.println("  ");
  temp = tempManual;
  Time = millis();
}

struct MyObject {
  float IsoP;
  float AlphaL;
  float AlphaH;
};

void Read_EE()
{
  int eeAddress = 0;
  MyObject customVar;
  EEPROM.get(eeAddress, customVar);
  IsoP = (customVar.IsoP);
  AlphaL = (customVar.AlphaL);
  AlphaH = (customVar.AlphaH);
}

void SaveSet()
{
  int eeAddress = 0;
  MyObject customVar = {
    IsoP,
    AlphaL,
    AlphaH
  };
  EEPROM.put(eeAddress, customVar);
}

void pH_read() // read ADS
{
  byte highbyte, lowbyte, configRegister;
  Wire.requestFrom(pHtoI2C, 3, sizeof(byte) * 3);
  while (Wire.available()) // ensure all the data comes in
  {
    highbyte = Wire.read(); // high byte * B11111111
    lowbyte = Wire.read(); // low byte
    configRegister = Wire.read();
  }
  data = highbyte * 256;
  data = data + lowbyte;
  voltage = data * 2.048 ;
  voltage = voltage / 32768; // mV

  if (voltage > 0)
    pH = IsoP - AlphaL * (T + temp) * voltage;
  else if (voltage < 0)
    pH = IsoP - AlphaH * (T + temp) * voltage;
}

void cal_sensors()
{

  switch (incomingByte)
  {
    case 56:
      Serial.print("Reset pH ...");
      IsoP = 7.00;
      AlphaL = 0.08;
      AlphaH = 0.08;
      SaveSet();
      Serial.println(" complete");
      break;

    case 52:
      Serial.print("Cal. pH 4.00 ...");
      AlphaL = (IsoP - 4) / voltage / (T + tempManual);
      SaveSet();
      Serial.println(" complete");
      break;

    case 55:
      Serial.print("Cal. pH 7.00 ...");
      IsoP = (IsoP - pH + 7.00);
      SaveSet();
      Serial.println(" complete");
      break;

    case 57:
      Serial.print("Cal. pH 10.00 ...");
      AlphaH = (IsoP - 10) / voltage / (T + tempManual);
      SaveSet();
      Serial.println(" complete");
      break;
  }
}

void showResults ()
{
  Serial.print("  pH ");
  Serial.println(pH);
}

void loop()
{
  if (millis() - Time >= Interval)
  {
    Time = millis();
    pH_read();
    showResults();
  }

  if (Serial.available() > 0) //  function of calibration
  {
    incomingByte = Serial.read();
    cal_sensors();
  }
}
