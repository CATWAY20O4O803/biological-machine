#include "LSM6DS3.h"
#include "Wire.h"

#include <SPI.h>
#include <SD.h>
#define CS_PIN 2

LSM6DS3 myIMU(I2C_MODE, 0x6A);
unsigned long previousMillis = 0;
const unsigned long interval = 1000;
void setup() {
    Serial.begin(9600);
    while (!Serial);
    if (myIMU.begin() != 0) {
        Serial.println("Device error");
    } else {
        Serial.println("Device OK!");
    }


    Serial.println("Initializing SD card...");
  

    if (!SD.begin(CS_PIN)) {
    Serial.println("SD card initialization failed!");
    return;
  }
    Serial.println("SD card initialized.");
}

void loop() {

  
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        Serial.println("AX1 AY1 AZ1 GX1 GY1 GZ1 TEMP:");
        Serial.print(myIMU.readFloatAccelX(), 4);
        Serial.print(" ");
        Serial.print(myIMU.readFloatAccelY(), 4);
        Serial.print(" ");
        Serial.print(myIMU.readFloatAccelZ(), 4);
        Serial.print(" ");
        Serial.print(myIMU.readFloatGyroX(), 4);
        Serial.print(" ");
        Serial.print(myIMU.readFloatGyroY(), 4);
        Serial.print(" ");
        Serial.print(myIMU.readFloatGyroZ(), 4);
        Serial.print(" ");
        Serial.println(myIMU.readTempC(), 4);

        File dataFile = SD.open("test.txt", FILE_WRITE);

        if (dataFile) {
          dataFile.println(myIMU.readFloatAccelX(), 4);
          dataFile.println(" ");
          dataFile.println(myIMU.readFloatAccelY(), 4);
          dataFile.println(" ");
          dataFile.println(myIMU.readFloatAccelZ(), 4);
          dataFile.println(" ");
          dataFile.println(myIMU.readFloatGyroX(), 4);
          dataFile.println(" ");
          dataFile.println(myIMU.readFloatGyroY(), 4);
          dataFile.println(" ");
          dataFile.println(myIMU.readFloatGyroZ(), 4);
          dataFile.println(" ");
          dataFile.close();
          Serial.println("Data written to file.");
        }
        else {
          Serial.println("Error opening file.");
        }

        

    }
    
}
