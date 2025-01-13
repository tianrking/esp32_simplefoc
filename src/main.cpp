#include <Arduino.h>
#include <SimpleFOC.h>
#include <MPU6050_tockn.h>

// 队列句柄定义
QueueHandle_t motorCommandQueue;

// 电机命令结构体
struct MotorCommand {
    int motorNumber;
    float voltage;
};

// 全局变量定义
TwoWire I2Cone = TwoWire(0);
TwoWire I2Ctwo = TwoWire(1);
MagneticSensorI2C sensor0 = MagneticSensorI2C(AS5600_I2C);
MagneticSensorI2C sensor1 = MagneticSensorI2C(AS5600_I2C);
MPU6050 mpu6050(I2Ctwo);
BLDCMotor motor0 = BLDCMotor(7);
BLDCMotor motor1 = BLDCMotor(7);
BLDCDriver3PWM driver0 = BLDCDriver3PWM(32, 33, 25, 22);
BLDCDriver3PWM driver1 = BLDCDriver3PWM(14, 27, 26, 12);

// 串口命令处理函数
void processSerialCommand(const String& command) {
    int commaIndex = command.indexOf(',');
    if (commaIndex != -1) {
        String motorStr = command.substring(0, commaIndex);
        String voltageStr = command.substring(commaIndex + 1);
        
        MotorCommand cmd;
        cmd.motorNumber = motorStr.toInt();
        cmd.voltage = voltageStr.toFloat();
        
        // 发送命令到队列
        xQueueSend(motorCommandQueue, &cmd, 0);
    }
}

// 串口和IMU任务 - 运行在Core 0
void serialAndImuTask(void * parameter) {
    static unsigned long lastPrintTime = 0;
    const int printInterval = 100;
    String inputBuffer = "";
    
    while(1) {
        // 非阻塞串口读取
        while (Serial.available()) {
            char c = Serial.read();
            if (c == '\n') {
                processSerialCommand(inputBuffer);
                inputBuffer = "";
            } else {
                inputBuffer += c;
            }
        }
        
        // 更新IMU数据
        mpu6050.update();
        
        // 定时打印IMU数据
        if (millis() - lastPrintTime >= printInterval) {
            lastPrintTime = millis();
            
            // 打印IMU数据
            Serial.printf("Angle: X=%.2f Y=%.2f Z=%.2f || Gyro: X=%.2f Y=%.2f Z=%.2f\n",
                mpu6050.getAngleX(), mpu6050.getAngleY(), mpu6050.getAngleZ(),
                mpu6050.getGyroX(), mpu6050.getGyroY(), mpu6050.getGyroZ());
        }
        
        // 延时1ms避免占用过多CPU
        vTaskDelay(1);
    }
}

// FOC控制任务 - 运行在Core 1
void focControlTask(void * parameter) {
    MotorCommand cmd;
    
    while(1) {
        // 检查是否有新的电机命令
        if (xQueueReceive(motorCommandQueue, &cmd, 0) == pdTRUE) {
            if (cmd.motorNumber == 0) {
                motor0.target = cmd.voltage;
            } else if (cmd.motorNumber == 1) {
                motor1.target = cmd.voltage;
            }
        }
        
        // FOC控制循环
        motor0.loopFOC();
        motor1.loopFOC();
        motor0.move();
        motor1.move();
        
        // 给其他任务一些运行时间
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
    Serial.begin(115200);
    delay(750);
    
    // 创建电机命令队列
    motorCommandQueue = xQueueCreate(10, sizeof(MotorCommand));
    
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
    
    // 创建任务并分配到不同核心
    xTaskCreatePinnedToCore(
        serialAndImuTask,     // 任务函数
        "SerialAndIMU",       // 任务名称
        8192,                 // 堆栈大小
        NULL,                 // 任务参数
        1,                    // 优先级
        NULL,                 // 任务句柄
        0                     // 运行核心 (Core 0)
    );
    
    xTaskCreatePinnedToCore(
        focControlTask,       // 任务函数
        "FOCControl",         // 任务名称
        8192,                 // 堆栈大小
        NULL,                 // 任务参数
        2,                    // 优先级
        NULL,                 // 任务句柄
        1                     // 运行核心 (Core 1)
    );
}

void loop() {
    // 主循环为空，所有工作都在FreeRTOS任务中完成
    vTaskDelay(portMAX_DELAY);
}