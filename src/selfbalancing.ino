#include <Wire.h>
#include <MPU6050.h>
#include <PID_v1.h>

MPU6050 mpu;

/* Motor Pins */
#define IN1 4
#define IN2 5
#define IN3 6
#define IN4 7
#define ENA 10
#define ENB 11

/* PWM Config */
#define PWM_FREQ 20000
#define PWM_RESOLUTION 8

/* MPU variables */
int16_t ax, ay, az;
int16_t gx, gy, gz;

/* Angle calculation */
float accAngle;
float gyroRate;
float angle = 0;
float dt = 0.01;

/* Calibration */
float angleOffset = 1.5;   // 🔧 TUNE THIS

/* PID */
double setpoint = 0;
double input;
double output;

double Kp = 28;
double Ki = 0.8;
double Kd = 1.2;

PID pid(&input, &output, &setpoint, Kp, Ki, Kd, DIRECT);

/* Motor offsets */
int leftOffset = 0;
int rightOffset = 12;   // 🔧 TUNE THIS

void setup()
{
  Serial.begin(115200);

  Wire.begin(8, 9);
  mpu.initialize();

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  /* NEW ESP32 PWM setup */
  ledcAttach(ENA, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(ENB, PWM_FREQ, PWM_RESOLUTION);

  pid.SetMode(AUTOMATIC);
  pid.SetOutputLimits(-255, 255);
}

void loop()
{
  /* Read MPU */
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  /* Angle calculation */
  accAngle = atan2(ax, az) * 180 / PI;
  gyroRate = gy / 131.0;

  /* Complementary filter */
  angle = 0.98 * (angle + gyroRate * dt) + 0.02 * accAngle;

  /* Apply offset */
  input = angle - angleOffset;

  /* PID compute */
  pid.Compute();

  /* Dead zone */
  if (abs(output) < 10) output = 0;

  /* Drive motors */
  driveMotors(output);

  /* Debug */
  Serial.print("Angle: ");
  Serial.print(angle);
  Serial.print(" Output: ");
  Serial.println(output);

  delay(10);
}

/* Motor control */
void driveMotors(int speed)
{
  int leftSpeed = speed + leftOffset;
  int rightSpeed = speed + rightOffset;

  leftSpeed = constrain(leftSpeed, -255, 255);
  rightSpeed = constrain(rightSpeed, -255, 255);

  /* LEFT MOTOR */
  if (leftSpeed > 0)
  {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
  }
  else
  {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
  }

  /* RIGHT MOTOR */
  if (rightSpeed > 0)
  {
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  }
  else
  {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  }

  /* Apply PWM (NEW API) */
  ledcWrite(ENA, abs(leftSpeed));
  ledcWrite(ENB, abs(rightSpeed));
}
