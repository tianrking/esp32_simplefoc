#include <Arduino.h>
#include <SimpleFOC.h>
#include <MPU6050_tockn.h>

// 硬件定义
TwoWire I2Cone = TwoWire(0);
TwoWire I2Ctwo = TwoWire(1);
MagneticSensorI2C sensor0 = MagneticSensorI2C(AS5600_I2C);
MagneticSensorI2C sensor1 = MagneticSensorI2C(AS5600_I2C);
MPU6050 mpu6050(I2Ctwo);
BLDCMotor motor0 = BLDCMotor(7);
BLDCMotor motor1 = BLDCMotor(7);
BLDCDriver3PWM driver0 = BLDCDriver3PWM(32, 33, 25, 22);
BLDCDriver3PWM driver1 = BLDCDriver3PWM(14, 27, 26, 12);

// 全局状态变量
volatile struct {
    float target0 = 0;
    float target1 = 0;
} state;

// 串口与IMU任务 - Core 0
void monitorTask(void * parameter) {
    String inputBuffer = "";
    uint32_t lastPrint = 0;
    const uint32_t printInterval = 100;  // 100ms打印间隔
    
    while(1) {
        // 处理串口命令
        while (Serial.available()) {
            char c = Serial.read();
            if (c == '\n') {
                int commaIndex = inputBuffer.indexOf(',');
                if (commaIndex != -1) {
                    // 直接解析两个目标值
                    state.target0 = inputBuffer.substring(0, commaIndex).toFloat();
                    state.target1 = inputBuffer.substring(commaIndex + 1).toFloat();
                }
                inputBuffer = "";
            } else {
                inputBuffer += c;
            }
        }
        
        // 定时打印状态
        if (millis() - lastPrint >= printInterval) {
            lastPrint = millis();
            
            mpu6050.update();
            Serial.printf("Targets: %.2f,%.2f | Speeds: %.2f,%.2f | IMU: %.2f,%.2f\n",
                state.target0, state.target1,
                motor0.shaft_velocity, motor1.shaft_velocity,
                mpu6050.getAngleY(), mpu6050.getGyroY()
            );
        }
        
        vTaskDelay(1);  // 给其他任务运行时间
    }
}

// 电机控制任务 - Core 1
void motorTask(void * parameter) {
    while(1) {
        // FOC控制循环
        motor0.loopFOC();
        motor1.loopFOC();
        
        // 应用目标值
        motor0.target = state.target0;
        motor1.target = state.target1;
        
        // 执行运动
        motor0.move();
        motor1.move();
        
        vTaskDelay(1);
    }
}

void I2C_init() {
    pinMode(32, INPUT_PULLUP);
    pinMode(33, INPUT_PULLUP);
    pinMode(25, INPUT_PULLUP);
    pinMode(26, INPUT_PULLUP);
    pinMode(27, INPUT_PULLUP);
    pinMode(14, INPUT_PULLUP);
    
    I2Cone.begin(19, 18, 400000UL);
    I2Ctwo.begin(23, 5, 400000UL);
    
    sensor0.init(&I2Cone);
    sensor1.init(&I2Ctwo);
}

void setup() {
    // 初始化串口
    Serial.begin(115200);
    delay(750);
    
    // 初始化硬件
    I2C_init();
    mpu6050.begin();
    mpu6050.calcGyroOffsets(true);
    
    // 初始化电机0
    motor0.linkSensor(&sensor0);
    motor0.voltage_sensor_align = 2;
    driver0.voltage_power_supply = 12;
    driver0.init();
    motor0.linkDriver(&driver0);
    motor0.torque_controller = TorqueControlType::voltage;
    motor0.controller = MotionControlType::torque;
    motor0.init();
    motor0.initFOC();
    
    delay(500);
    
    // 初始化电机1 
    motor1.linkSensor(&sensor1);
    motor1.voltage_sensor_align = 2;
    driver1.voltage_power_supply = 12;
    driver1.init();
    motor1.linkDriver(&driver1);
    motor1.torque_controller = TorqueControlType::voltage;
    motor1.controller = MotionControlType::torque;
    motor1.init();
    motor1.initFOC();
    
    Serial.println("Ready!");
    Serial.println("Command format: target1,target2");
    Serial.println("Example: 2,-2");
    
    // 创建任务
    xTaskCreatePinnedToCore(
        monitorTask,   // 监控任务(串口和IMU)
        "monitor",
        8192,
        NULL,
        1,
        NULL,
        0             // Core 0
    );
    
    xTaskCreatePinnedToCore(
        motorTask,     // 电机控制任务
        "motor",
        8192,
        NULL,
        2,
        NULL,
        1             // Core 1
    );
}

void loop() {
    vTaskDelay(portMAX_DELAY);
}