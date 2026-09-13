#include <Wire.h>

#define MPU 0x68

// Motor pins
#define IN1 4
#define IN2 5
#define IN3 6
#define IN4 7

#define ENA 10
#define ENB 11

int16_t AccX, AccY, AccZ;
int16_t GyroY;

float angle = 0;
float accAngle, gyroRate;

float dt;
unsigned long prevTime;

// PID
float Kp = 25;
float Ki = 0.0;
float Kd = 0.8;

float error, prevError = 0;
float integral = 0;
float output;

void setup() {

  Serial.begin(115200);
  Wire.begin(8,9);

  // wake MPU6050
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  pinMode(IN1,OUTPUT);
  pinMode(IN2,OUTPUT);
  pinMode(IN3,OUTPUT);
  pinMode(IN4,OUTPUT);

  // NEW PWM API
  ledcAttach(ENA, 1000, 8);
  ledcAttach(ENB, 1000, 8);

  prevTime = millis();
}

void loop() {

  // Read MPU
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU,14,true);

  AccX = Wire.read()<<8 | Wire.read();
  Wire.read(); Wire.read();
  AccZ = Wire.read()<<8 | Wire.read();

  Wire.read(); Wire.read(); // temp

  Wire.read(); Wire.read(); // gyroX
  GyroY = Wire.read()<<8 | Wire.read();

  // Time
  dt = (millis() - prevTime)/1000.0;
  prevTime = millis();

  // Angle
  accAngle = atan2(AccX,AccZ)*180/PI;
  gyroRate = GyroY/131.0;

  angle = 0.98*(angle + gyroRate*dt) + 0.02*accAngle;

  // PID
  error = angle;
  integral += error * dt;
  float derivative = (error - prevError)/dt;

  output = Kp*error + Ki*integral + Kd*derivative;
  prevError = error;

  controlMotor(output);

  Serial.println(angle);

  delay(5);
}

void controlMotor(float speed){

  int pwm = constrain(abs(speed), 0, 255);

  // minimum power to move motor
  if(pwm > 0 && pwm < 70) pwm = 70;

  if(speed > 0){
    digitalWrite(IN1,HIGH);
    digitalWrite(IN2,LOW);
    digitalWrite(IN3,HIGH);
    digitalWrite(IN4,LOW);
  }
  else{
    digitalWrite(IN1,LOW);
    digitalWrite(IN2,HIGH);
    digitalWrite(IN3,LOW);
    digitalWrite(IN4,HIGH);
  }

  ledcWrite(ENA, pwm);
  ledcWrite(ENB, pwm);
}
