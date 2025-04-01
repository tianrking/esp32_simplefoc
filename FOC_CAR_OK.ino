#include <Arduino.h>
#include <SimpleFOC.h>
#include <MPU6050_tockn.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

//若车轮方向不对，可取反这里的宏定义
#define M0 -1
#define M1 1

// 蓝牙参数
BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

TwoWire I2Cone = TwoWire(0);
TwoWire I2Ctwo = TwoWire(1);
MagneticSensorI2C sensor0 = MagneticSensorI2C(AS5600_I2C);
MagneticSensorI2C sensor1 = MagneticSensorI2C(AS5600_I2C);

MPU6050 mpu6050(I2Ctwo);

BLDCMotor motor0 = BLDCMotor(7);
BLDCDriver3PWM driver0 = BLDCDriver3PWM(32,33,25,22);

BLDCMotor motor1 = BLDCMotor(7);
BLDCDriver3PWM driver1 = BLDCDriver3PWM(26,27,14,12);

// 自稳PID
PIDController pid_stb = PIDController(0.8, 5.5, 0, 100000, 8);
// 速度PID
PIDController pid_vel = PIDController(1.5, 0, 0.01, 10000, _PI/4);
// 速度控制滤波
LowPassFilter lpf_pitch_cmd = LowPassFilter(0.07);
// 油门和转向滤波
LowPassFilter lpf_throttle = LowPassFilter(0.5);
LowPassFilter lpf_steering = LowPassFilter(0.1);

String resStr;
String chipId;

float steering = 0;
float throttle = 0;
float new_steering;
float new_throttle;
float max_throttle = 80; // 初始值20 rad/s 
float max_steering = 1; //  1V
float Offset_parameters = -2; //偏置参数
int Checkcomma; //判断是否含有逗号分隔

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      // 使用 std::string 接收特征值
      std::string rxValue = pCharacteristic->getValue().c_str();
      
      if (rxValue.length() > 0) {
        for (int i = 0; i < rxValue.length(); i++){
          resStr += rxValue[i];
        }
        Checkcomma = resStr.indexOf(',');
        if(Checkcomma != -1){
          new_steering = resStr.substring(0,Checkcomma).toInt();
          new_throttle = resStr.substring(Checkcomma+1,resStr.length()).toInt();
          steering = max_steering * new_steering/50;
          throttle = max_throttle * -new_throttle/50;
        }
        else{
          steering = 0;
          throttle = 0;
        }
        // 清空数据准备下次接收
        resStr = "";
      }
    }
};

//I2C初始化
void I2C_init();

void setup(){
    chipId = String((uint32_t)ESP.getEfuseMac(), HEX);
    chipId.toUpperCase();
    
    //  初始化蓝牙 蓝牙名称
    BLEDevice::init("Blue Control");

    // 开启一个蓝牙服务
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService *pService = pServer->createService(SERVICE_UUID);

    pTxCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID_TX,
                        BLECharacteristic::PROPERTY_NOTIFY
                      );

    pTxCharacteristic->addDescriptor(new BLE2902());

    BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_RX,
        BLECharacteristic::PROPERTY_WRITE
                                          );

    pRxCharacteristic->setCallbacks(new MyCallbacks());

    pService->start();

    pServer->getAdvertising()->start();

    I2C_init();

    mpu6050.begin();
    mpu6050.calcGyroOffsets(true);

    // link the motor to the sensor
    motor0.linkSensor(&sensor0);
    motor1.linkSensor(&sensor1);

    // 速度环PID参数
    motor1.PID_velocity.P = 0.01;
    motor1.PID_velocity.I = 0.1;
    motor1.PID_velocity.D = 0;

    motor0.PID_velocity.P = 0.01;
    motor0.PID_velocity.I = 0.1;
    motor0.PID_velocity.D = 0;

    motor0.voltage_sensor_align = 2;
    driver0.voltage_power_supply = 12;
    driver0.init();
    motor0.linkDriver(&driver0);
    motor1.voltage_sensor_align = 2;
    driver1.voltage_power_supply = 12;
    driver1.init();
    motor1.linkDriver(&driver1);

    motor0.torque_controller = TorqueControlType::voltage;
    motor1.torque_controller = TorqueControlType::voltage;
    motor0.controller = MotionControlType::torque;
    motor1.controller = MotionControlType::torque;

    // initialise motor
    motor1.init();
    motor0.init();
    // align encoder and start FOC
    motor1.initFOC();
    motor0.initFOC();
}

void loop(){
    // 未连接
    if (!deviceConnected && oldDeviceConnected) {
      delay(500); 
      pServer->startAdvertising(); // restart advertising
      oldDeviceConnected = deviceConnected;
    }
    // 连接成功
    if (deviceConnected && !oldDeviceConnected) {
      oldDeviceConnected = deviceConnected;
    }
    
    motor0.loopFOC();
    motor1.loopFOC();
    motor0.move();
    motor1.move();

    mpu6050.update();
    double mpu_pitch = mpu6050.getAngleY();// tockn的getangle，通过一阶置信计算
    float target_pitch = lpf_pitch_cmd(pid_vel((M0 * motor0.shaft_velocity + M1 * motor1.shaft_velocity) / 2 - lpf_throttle(throttle)));
    float voltage_control = pid_stb(Offset_parameters - mpu_pitch + target_pitch);
    float steering_adj = lpf_steering(steering);
  
    motor0.target = M0 * voltage_control - steering_adj;
    motor1.target = M1 * voltage_control + steering_adj;
}

void I2C_init(){
  pinMode(32, INPUT_PULLUP);
  pinMode(33, INPUT_PULLUP);
  pinMode(25, INPUT_PULLUP);
  pinMode(26, INPUT_PULLUP);
  pinMode(27, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);

  // 编码器设置
  I2Cone.begin(19,18, 400000UL); 
  I2Ctwo.begin(23,5, 400000UL); 
  sensor0.init(&I2Cone);
  sensor1.init(&I2Ctwo);
}