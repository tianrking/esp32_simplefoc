// #include "MotorTask.h"

// BLDCMotor* MotorTask::motor0 = nullptr;
// BLDCMotor* MotorTask::motor1 = nullptr;

// volatile struct GlobalState state = {0, 0};

// void MotorTask::init(BLDCMotor* m0, BLDCMotor* m1) {
//     motor0 = m0;
//     motor1 = m1;
// }

// void MotorTask::startTask(void* parameter) {
//     xTaskCreatePinnedToCore(
//         task,
//         "motor",
//         8192,
//         parameter,
//         2,
//         NULL,
//         1  // Core 1
//     );
// }

// void MotorTask::task(void* parameter) {
//     while(1) {
//         if(motor0 && motor1) {
//             // FOC控制循环
//             motor0->loopFOC();
//             motor1->loopFOC();
            
//             // 应用目标值
//             motor0->target = state.target0;
//             motor1->target = state.target1;
            
//             // 执行运动
//             motor0->move();
//             motor1->move();
//         }
//         vTaskDelay(1);
//     }
// }