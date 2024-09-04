#ifndef SIMPLEFOC_CONTROL_H
#define SIMPLEFOC_CONTROL_H

#include <Arduino.h>
#include <SimpleFOC.h>

void setupSimpleFOC();
void motorFOCTask(void *pvParameters);
void motorControlTask(void *pvParameters);
// void parseSerialCommand(String command);

void setMotorTargetAngle(float angle);
String getMotorStatus();

#endif // SIMPLEFOC_CONTROL_H