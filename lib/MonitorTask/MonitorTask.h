// #pragma once

// #include <Arduino.h>
// #include <SimpleFOC.h>
// #include <MPU6050_tockn.h>

// class MonitorTask {
// public:
//     static void init(MPU6050* imu, BLDCMotor* m0, BLDCMotor* m1);
//     static void startTask(void* parameter);

// private:
//     static MPU6050* imu;
//     static BLDCMotor* motor0;
//     static BLDCMotor* motor1;
//     static void task(void* parameter);
//     static void processSerialCommand(const String& cmd);
// };