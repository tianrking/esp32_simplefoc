#include <SimpleFOC.h>

MagneticSensorI2C sensor = MagneticSensorI2C(AS5600_I2C);
TwoWire I2Cone = TwoWire(0);
BLDCMotor motor = BLDCMotor(7);
BLDCDriver3PWM driver = BLDCDriver3PWM(32, 33, 25, 22);


float target_velocity = 0;      // 初始目标角度为0
Commander command = Commander(Serial);    
void doTarget(char* cmd) { command.scalar(&target_velocity, cmd); }    // 串口控制指令：目标值
void onMotor(char* cmd){ command.motor(&motor,cmd); }      // 串口控制指令：电机


void setup() {
  // 编码器设置
    I2Cone.begin(19, 18, 400000);
    sensor.init(&I2Cone);
  motor.linkSensor(&sensor);

  // 驱动设置
  driver.voltage_power_supply = 12;
  driver.init();
  motor.linkDriver(&driver);

  // 选择调制方式为SVPWM
  motor.foc_modulation = FOCModulationType::SpaceVectorPWM;
  // 控制模式为角度模式
  motor.controller = MotionControlType::velocity;
  
  // PID参数
  motor.PID_velocity.P = 0.3;
  motor.PID_velocity.I = 10;
  motor.PID_velocity.D = 0;
  
  //其他参数
  motor.voltage_limit = 6;    //最大电压
  motor.velocity_limit = 20;   //最大速度，rad/s
  motor.LPF_velocity.Tf = 0.01;  //速度的滤波时间常数

  // 串口设置
  Serial.begin(115200);
  motor.useMonitoring(Serial);  //使用串口监视器
  //初始化
  motor.init();
  motor.initFOC();

  // 添加串口命令
  command.add('T', doTarget, "target angle");
  command.add('M',onMotor,"my motor"); 
  
  Serial.println(F("Motor ready."));
  _delay(1000);
}

void loop() {  
  // main FOC algorithm function
  // the faster you run this function the better
  // Arduino UNO loop  ~1kHz
  // Bluepill loop ~10kHz 
  motor.loopFOC();

  // Motion control function
  // velocity, position or voltage (defined in motor.controller)
  // this function can be run at much lower frequency than loopFOC() function
  // You can also use motor.move() and set the motor.target in the code
  motor.move(target_velocity);

  // function intended to be used with serial plotter to monitor motor variables
  // significantly slowing the execution down!!!!
  motor.monitor();    //使用simpleFOC Studio上位机设置的时候，这句一定要打开。但是会影响程序执行速度
  
  // user communication
  command.run();
}
