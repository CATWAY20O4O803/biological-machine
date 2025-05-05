#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "LSM6DS3.h"
#include "MAX30105.h"

#define CS_PIN 2
#define USE_SERIAL true

LSM6DS3 myIMU(I2C_MODE, 0x6A);
MAX30105 particleSensor;

unsigned long recordCount = 0;
const unsigned long interval = 50;
unsigned long previousMillis = 0;
unsigned long startMillis = 0; // 記錄第一筆資料的時間戳
File dataFile;

bool ledState = false;
char filename[20];

// IMU 校正變數
#define CALIBRATION_SAMPLES 1000  // 減少樣本數，加快校正
#define ALPHA 0.90
float accelX_offset = 0, accelY_offset = 0, accelZ_offset = 0;
float gyroBiasX = 0, gyroBiasY = 0, gyroBiasZ = 0;

float accelX, accelY, accelZ, gyroX, gyroY, gyroZ;
long irValue;

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(500000);
    delay(1000);  // 避免 while(!Serial) 延遲

    Serial.println("初始化 LSM6DS3...");
    if (myIMU.begin() != 0) {
        Serial.println("LSM6DS3 初始化失敗!");
        while (1);
    } else {
        Serial.println("LSM6DS3 初始化成功!");
        

        float sumGx=0, sumGy=0, sumGz=0;
    for (int i=0; i<CALIBRATION_SAMPLES; i++) {
        sumGx += myIMU.readFloatGyroX();
        sumGy += myIMU.readFloatGyroY();
        sumGz += myIMU.readFloatGyroZ();
    }
    gyroBiasX = sumGx / CALIBRATION_SAMPLES;
    gyroBiasY = sumGy / CALIBRATION_SAMPLES;
    gyroBiasZ = sumGz / CALIBRATION_SAMPLES;
    if (USE_SERIAL) {
        Serial.println("IMU 校正完成!");
        Serial.print("Gyro Bias: ");
        Serial.print(gyroBiasX); Serial.print(", ");
        Serial.print(gyroBiasY); Serial.print(", ");
        Serial.println(gyroBiasZ);
    }



    }

    Serial.println("初始化 MAX30102...");
    if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
        Serial.println("MAX30102 初始化失敗!");
        return;
    }
    particleSensor.setup();
    particleSensor.setPulseAmplitudeRed(0x0A);
    particleSensor.setPulseAmplitudeIR(0x1F);

    Serial.println("初始化 SD 卡...");
    if (!SD.begin(CS_PIN)) {
        Serial.println("SD 卡初始化失敗!");
    } else {
        for (int i = 1; i < 1000; i++) {
            sprintf(filename, "data%03d.txt", i);
            if (!SD.exists(filename)) {
                dataFile = SD.open(filename, FILE_WRITE);
                break;
            }
        }

        if (!dataFile) {
            Serial.println("無法開啟 SD 卡文件!");
        } else {
            Serial.println("SD 卡初始化成功!");
            if (dataFile.size() == 0) {
                Serial.println("✅ 寫入標題行...");
                dataFile.println("stamp,時間(ms),AX,AY,AZ,GX,GY,GZ,IR");
                dataFile.flush();
            }
        }
    }

    previousMillis = millis();
    Serial.print("Initialization completed : ");
    Serial.println(previousMillis);
}

void loop() {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        recordCount++;
        if (recordCount == 1) {
            startMillis = currentMillis; // 設定起始時間
        }
        unsigned long elapsedTime = currentMillis - startMillis; // 真實經過時間

        ledState = !ledState;
        digitalWrite(LED_BUILTIN, ledState ? LOW : HIGH);

        irValue = particleSensor.getIR();

        float rawAccelX = myIMU.readFloatAccelX();
        float rawAccelY = myIMU.readFloatAccelY();
        float rawAccelZ = myIMU.readFloatAccelZ();
        accelX = rawAccelX - accelX_offset;
        accelY = rawAccelY - accelY_offset;
        accelZ = rawAccelZ + accelZ_offset;

        static float prevGyroX = 0, prevGyroY = 0, prevGyroZ = 0;
        float rawGyroX = myIMU.readFloatGyroX() - gyroBiasX;
        float rawGyroY = myIMU.readFloatGyroY() - gyroBiasY;
        float rawGyroZ = myIMU.readFloatGyroZ() - gyroBiasZ;
        gyroX = ALPHA * prevGyroX + (1 - ALPHA) * rawGyroX;
        gyroY = ALPHA * prevGyroY + (1 - ALPHA) * rawGyroY;
        gyroZ = ALPHA * prevGyroZ + (1 - ALPHA) * rawGyroZ;
        prevGyroX = gyroX; prevGyroY = gyroY; prevGyroZ = gyroZ;

        if (dataFile) {
            dataFile.print(currentMillis); dataFile.print(",");
            dataFile.print(elapsedTime);   dataFile.print(",");
            dataFile.print(accelX, 4);     dataFile.print(",");
            dataFile.print(accelY, 4);     dataFile.print(",");
            dataFile.print(accelZ, 4);     dataFile.print(",");
            dataFile.print(gyroX, 4);      dataFile.print(",");
            dataFile.print(gyroY, 4);      dataFile.print(",");
            dataFile.print(gyroZ, 4);      dataFile.print(",");
            dataFile.println(irValue);
            dataFile.flush();

            if (recordCount % 1000 == 0) {
                if (USE_SERIAL) Serial.println("正在寫入 SD 卡...");
                dataFile.flush();
                dataFile.close();
                dataFile = SD.open(filename, FILE_WRITE);
            }
        }

        if (USE_SERIAL) {
            Serial.print(currentMillis); Serial.print(",");
            Serial.print(elapsedTime);   Serial.print(",");
            Serial.print(accelX, 4);     Serial.print(",");
            Serial.print(accelY, 4);     Serial.print(",");
            Serial.print(accelZ, 4);     Serial.print(",");
            Serial.print(gyroX, 4);      Serial.print(",");
            Serial.print(gyroY, 4);      Serial.print(",");
            Serial.print(gyroZ, 4);      Serial.print(",");
            Serial.println(irValue);
        }
    }
}


