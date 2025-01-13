// #include "MonitorTask.h"
// #include "MotorTask.h"  // 为了访问全局state

// MPU6050* MonitorTask::imu = nullptr;
// BLDCMotor* MonitorTask::motor0 = nullptr;
// BLDCMotor* MonitorTask::motor1 = nullptr;

// void MonitorTask::init(MPU6050* _imu, BLDCMotor* m0, BLDCMotor* m1) {
//     imu = _imu;
//     motor0 = m0;
//     motor1 = m1;
// }

// void MonitorTask::startTask(void* parameter) {
//     xTaskCreatePinnedToCore(
//         task,
//         "monitor",
//         8192,
//         parameter,
//         1,
//         NULL,
//         0  // Core 0
//     );
// }

// void MonitorTask::processSerialCommand(const String& cmd) {
//     int commaIndex = cmd.indexOf(',');
//     if (commaIndex != -1) {
//         state.target0 = cmd.substring(0, commaIndex).toFloat();
//         state.target1 = cmd.substring(commaIndex + 1).toFloat();
//     }
// }

// void MonitorTask::task(void* parameter) {
//     String inputBuffer = "";
//     uint32_t lastPrint = 0;
//     const uint32_t printInterval = 100;
    
//     while(1) {
//         // 处理串口命令
//         while (Serial.available()) {
//             char c = Serial.read();
//             if (c == '\n') {
//                 processSerialCommand(inputBuffer);
//                 inputBuffer = "";
//             } else {
//                 inputBuffer += c;
//             }
//         }
        
//         // 定时打印状态
//         if (millis() - lastPrint >= printInterval) {
//             lastPrint = millis();
            
//             if(imu && motor0 && motor1) {
//                 imu->update();
//                 Serial.printf("Targets: %.2f,%.2f | Speeds: %.2f,%.2f | IMU: %.2f,%.2f\n",
//                     state.target0, state.target1,
//                     motor0->shaft_velocity, motor1->shaft_velocity,
//                     imu->getAngleY(), imu->getGyroY()
//                 );
//             }
//         }
//         vTaskDelay(1);
//     }
// }