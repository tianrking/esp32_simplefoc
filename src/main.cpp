#include <Arduino.h>
#include "simplefoc_control.h"
#include "uart_protocol.h"

TaskHandle_t motorFOCTaskHandle = NULL;
TaskHandle_t motorControlTaskHandle = NULL;

TaskHandle_t uartTaskHandle = NULL;

void uartTask(void *pvParameters) {
        // 初始化 UART 协议
    uartProtocol.init(115200);

    // 注册命令
    uartProtocol.registerCommand("AT+ANGLE", [](const String& params) -> String {
        if (params.isEmpty()) {
            return String("ERROR: Missing angle parameter");
        }
        float angle = params.toFloat();
        setMotorTargetAngle(angle);
        String response = "OK: Target angle set to ";
        response += String(angle);
        return response;
    });

    uartProtocol.registerCommand("AT+STATUS", [](const String&) -> String {
        String response = "OK: ";
        response += getMotorStatus();
        return response;
    });

    uartProtocol.registerCommand("AT+SETMODE", [](const String& params) -> String {
        if (params.isEmpty()) {
            return String("ERROR: Missing mode parameter");
        }
        int mode = params.toInt();
        switch (mode) {
            case 1:
                // 设置模式1的逻辑
                return String("OK: Mode set to 1");
            case 2:
                // 设置模式2的逻辑
                return String("OK: Mode set to 2");
            default:
                return String("ERROR: Invalid mode");
        }
    });
    
    for (;;) {
        uartProtocol.handle();
        vTaskDelay(pdMS_TO_TICKS(10)); // 短暂延迟以避免占用过多CPU时间
    }
}

void setup() {
    Serial.begin(115200);
    
    setupSimpleFOC();

    xTaskCreatePinnedToCore(motorFOCTask, "MotorFOC", 4096, NULL, 1, &motorFOCTaskHandle, 1); //simplefoc scheduler
    xTaskCreatePinnedToCore(motorControlTask, "MotorControl", 4096, NULL, 1, &motorControlTaskHandle, 0); 
    xTaskCreatePinnedToCore(uartTask, "UARTTask", 4096, NULL, 1, &uartTaskHandle, 0);
}

void loop() {
    // if (Serial.available()) {
    //     String command = Serial.readStringUntil('\n');
    //     parseSerialCommand(command);
    // }
    vTaskDelay(portMAX_DELAY);
}