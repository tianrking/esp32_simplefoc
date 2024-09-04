#include "simplefoc_control.h"

// 组件定义
MagneticSensorI2C sensor = MagneticSensorI2C(AS5600_I2C);
TwoWire I2Cone = TwoWire(0);
BLDCMotor motor = BLDCMotor(7);
BLDCDriver3PWM driver = BLDCDriver3PWM(32, 33, 25, 22);

// 控制变量
float target_angle = 0;
bool direction = true;

// 性能监控变量
unsigned long loopStartTime, loopEndTime, loopDuration;
unsigned int loopCount = 0;

void setupSimpleFOC() {
    // I2C和传感器初始化
    I2Cone.begin(19, 18, 400000);
    sensor.init(&I2Cone);
    
    // 电机和驱动器设置
    motor.linkSensor(&sensor);
    
    driver.voltage_power_supply = 12;
    driver.init();
    motor.linkDriver(&driver);
    
    // 电机控制参数设置
    motor.foc_modulation = FOCModulationType::SpaceVectorPWM;
    motor.controller = MotionControlType::angle;
    
    motor.PID_velocity.P = 0.1f;
    motor.PID_velocity.I = 1;
    motor.PID_velocity.D = 0;
    motor.voltage_limit = 12;
    motor.LPF_velocity.Tf = 0.01f;
    motor.P_angle.P = 20;
    motor.velocity_limit = 50;
    
    // 串口监控设置
    motor.useMonitoring(Serial);
    
    // 电机初始化和FOC
    motor.init();
    motor.initFOC();

    Serial.println(F("Motor initialized and ready."));
    Serial.println(F("Rotating between 0 and 90 degrees every 3 seconds."));
    Serial.println(F("Type 'angle [value]' to set target angle or 'status' for motor status."));
}

void motorFOCTask(void *pvParameters) {
    while (true) {
        loopStartTime = micros();
        motor.loopFOC();
        loopEndTime = micros();
        loopDuration = loopEndTime - loopStartTime;
        loopCount++;
        
        if (loopCount >= 100) {
            // Serial.print(F("loopFOC() average execution time: "));
            // Serial.print(loopDuration / 100.0);
            // Serial.println(F(" microseconds"));
            loopCount = 0;
        }
        
        motor.move();
    }
}

void motorControlTask(void *pvParameters) {
    while (true) {
        // if (direction) {
        //     target_angle = 0;
        // } else {
        //     target_angle = 3.14 / 2;
        // }
        // direction = !direction;
        
        // motor.target = target_angle;
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// void parseSerialCommand(String command) {
//     command.trim();
//     if (command.startsWith("angle")) {
//         float angle = command.substring(5).toFloat();
//         motor.target = angle;
//         Serial.println("Target angle set to: " + String(angle));
//     } else if (command == "status") {
//         Serial.println("Current angle: " + String(motor.shaft_angle));
//         Serial.println("Target angle: " + String(motor.target));
//     } else {
//         Serial.println("Unknown command. Available commands:");
//         Serial.println("angle [value] - Set target angle");
//         Serial.println("status - Get current motor status");
//     }
// }

void setMotorTargetAngle(float angle) {
    motor.target = angle;
}

String getMotorStatus() {
    return "Current angle: " + String(motor.shaft_angle) + "\n"
           "Target angle: " + String(motor.target);
}