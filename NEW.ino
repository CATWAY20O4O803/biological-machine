#include <Wire.h>
#include <SPI.h>
#include <SD.h>

#include "LSM6DS3.h"
#include "MAX30105.h"
#include "heartRate.h"

#define CS_PIN 2  // SD 卡片選擇腳位
LSM6DS3 myIMU(I2C_MODE, 0x6A);
MAX30105 particleSensor;

const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];  // 心率存储数组
byte rateSpot = 0;
long lastBeat = 0;

float beatsPerMinute;
int beatAvg;
unsigned long previousMillis = 0;
const unsigned long interval = 1000;  // 1 秒采集一次数据

void setup() {
    Serial.begin(115200);
    // 初始化 LSM6DS3 传感器
    if (myIMU.begin() != 0) {
        Serial.println("LSM6DS3 初始化失败!");
    } else {
        Serial.println("LSM6DS3 初始化成功!");
    }

    // 初始化 MAX30105 传感器
    Serial.println("Initializing MAX30105...");
    if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
        Serial.println("MAX30105 初始化失败! 请检查连接.");
        while (1);
    }
    Serial.println("请用手指按住传感器.");
    particleSensor.setup();
    particleSensor.setPulseAmplitudeRed(0x0A);
    particleSensor.setPulseAmplitudeGreen(0);

    // 初始化 SD 卡
    Serial.println("初始化 SD 卡...");
    if (!SD.begin(CS_PIN)) {
        Serial.println("SD 卡初始化失败!");
        return;
    }
    Serial.println("SD 卡初始化成功.");
}

void loop() {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;

        // 读取 LSM6DS3 数据
        float accelX = myIMU.readFloatAccelX();
        float accelY = myIMU.readFloatAccelY();
        float accelZ = myIMU.readFloatAccelZ();
        float gyroX = myIMU.readFloatGyroX();
        float gyroY = myIMU.readFloatGyroY();
        float gyroZ = myIMU.readFloatGyroZ();
        float tempC = myIMU.readTempC();

        // 读取 MAX30105 数据
        long irValue = particleSensor.getIR();

        if (checkForBeat(irValue) == true) {
            long delta = millis() - lastBeat;
            lastBeat = millis();

            beatsPerMinute = 60 / (delta / 1000.0);
            if (beatsPerMinute < 255 && beatsPerMinute > 20) {
                rates[rateSpot++] = (byte)beatsPerMinute;
                rateSpot %= RATE_SIZE;

                beatAvg = 0;
                for (byte x = 0; x < RATE_SIZE; x++) {
                    beatAvg += rates[x];
                }
                beatAvg /= RATE_SIZE;
            }
        }

        // 打印数据到串口
        Serial.print("AX="); Serial.print(accelX, 4);
        Serial.print(" AY="); Serial.print(accelY, 4);
        Serial.print(" AZ="); Serial.print(accelZ, 4);
        Serial.print(" GX="); Serial.print(gyroX, 4);
        Serial.print(" GY="); Serial.print(gyroY, 4);
        Serial.print(" GZ="); Serial.print(gyroZ, 4);
        Serial.print(" TEMP="); Serial.print(tempC, 4);
        Serial.print(" IR="); Serial.print(irValue);
        Serial.print(" BPM="); Serial.print(beatsPerMinute);
        Serial.print(" Avg BPM="); Serial.print(beatAvg);
        if (irValue < 50000) Serial.print(" (No finger detected)");
        Serial.println();

        // 将数据写入 SD 卡
        File dataFile = SD.open("data.txt", FILE_WRITE);
        if (dataFile) {
            dataFile.print(accelX, 4); dataFile.print(",");
            dataFile.print(accelY, 4); dataFile.print(",");
            dataFile.print(accelZ, 4); dataFile.print(",");
            dataFile.print(gyroX, 4); dataFile.print(",");
            dataFile.print(gyroY, 4); dataFile.print(",");
            dataFile.print(gyroZ, 4); dataFile.print(",");
            dataFile.print(tempC, 4); dataFile.print(",");
            dataFile.print(irValue); dataFile.print(",");
            dataFile.print(beatsPerMinute); dataFile.print(",");
            dataFile.println(beatAvg);
            dataFile.close();
            Serial.println("数据已存储至 SD 卡.");
        } else {
            Serial.println("打开 SD 卡文件失败!");
        }
    }
}