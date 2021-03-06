/**
 * Comprehensive BLDC motor control example using encoder and the DRV8302 board
 * 
 * Using serial terminal user can send motor commands and configure the motor and FOC in real-time:
 * - configure PID controller constants
 * - change motion control loops
 * - monitor motor variabels
 * - set target values
 * - check all the configuration values 
 * 
 * check the https://docs.simplefoc.com for full list of motor commands
 * 
 */
#include <SimpleFOC.h>

// DRV8302 pins connections
// don't forget to connect the common ground pin
#define INH_A 3
#define INH_B 5
#define INH_C 9
#define INL_A 11
#define INL_B 6
#define INL_C 10

#define EN_GATE 7
#define M_PWM A1 
#define M_OC A2
#define OC_ADJ A3

// Motor instance
BLDCMotor motor = BLDCMotor(11);
BLDCDriver6PWM driver = BLDCDriver6PWM(INH_A,INH_A, INH_B,INH_B, INH_C,INL_C, EN_GATE);

// encoder instance
Encoder encoder = Encoder(2, 3, 8192);

// Interrupt routine intialisation
// channel A and B callbacks
void doA(){encoder.handleA();}
void doB(){encoder.handleB();}

void setup() {

  // initialize encoder sensor hardware
  encoder.init();
  encoder.enableInterrupts(doA, doB); 
  // link the motor to the sensor
  motor.linkSensor(&encoder);

  // DRV8302 specific code
  // M_OC  - enable overcurrent protection
  pinMode(M_OC,OUTPUT);
  digitalWrite(M_OC,LOW);
  // M_PWM  - disable 3pwm mode
  pinMode(M_PWM,OUTPUT);
  digitalWrite(M_PWM, LOW);
  // OD_ADJ - set the maximum overcurrent limit possible
  // Better option would be to use voltage divisor to set exact value
  pinMode(OC_ADJ,OUTPUT);
  digitalWrite(OC_ADJ,HIGH);

  // driver config
  // power supply voltage [V]
  driver.voltage_power_supply = 12;
  driver.init();
  // link the motor and the driver
  motor.linkDriver(&driver);

  // choose FOC modulation
  motor.foc_modulation = FOCModulationType::SpaceVectorPWM;

  // set control loop type to be used
  motor.controller = ControlType::voltage;

  // contoller configuration based on the controll type 
  motor.PID_velocity.P = 0.2;
  motor.PID_velocity.I = 20;
  // default voltage_power_supply
  motor.voltage_limit = 12;

  // velocity low pass filtering time constant
  motor.LPF_velocity.Tf = 0.01;

  // angle loop controller
  motor.P_angle.P = 20;
  // angle loop velocity limit
  motor.velocity_limit = 50;

  // use monitoring with serial for motor init
  // monitoring port
  Serial.begin(115200);
  // comment out if not needed
  motor.useMonitoring(Serial);

  // initialise motor
  motor.init();
  // align encoder and start FOC
  motor.initFOC();

  // set the inital target value
  motor.target = 2;


  Serial.println("Full control example: ");
  Serial.println("Run user commands to configure and the motor (find the full command list in docs.simplefoc.com) \n ");
  Serial.println("Initial motion control loop is voltage loop.");
  Serial.println("Initial target voltage 2V.");
  
  _delay(1000);
}


void loop() {
  // iterative setting FOC phase voltage
  motor.loopFOC();

  // iterative function setting the outter loop target
  // velocity, position or voltage
  // if tatget not set in parameter uses motor.target variable
  motor.move();

  // user communication
  motor.command(serialReceiveUserCommand());
}

// utility function enabling serial communication the user
String serialReceiveUserCommand() {
  
  // a string to hold incoming data
  static String received_chars;
  
  String command = "";

  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the string buffer:
    received_chars += inChar;

    // end of user input
    if (inChar == '\n') {
      
      // execute the user command
      command = received_chars;

      // reset the command buffer 
      received_chars = "";
    }
  }
  return command;
}

