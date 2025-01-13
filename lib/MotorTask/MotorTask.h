#pragma once

#include <Arduino.h>
#include <SimpleFOC.h>

class MotorTask {
public:
    static void init(BLDCMotor* m0, BLDCMotor* m1);
    static void startTask(void* parameter);
    
private:
    static BLDCMotor* motor0;
    static BLDCMotor* motor1;
    static void task(void* parameter);
};

// 全局状态变量声明
extern volatile struct GlobalState {
    float target0;
    float target1;
} state;