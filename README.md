# Self-Balancing Robot

A two-wheel self-balancing robot developed using an ESP32-S3 microcontroller, MPU6050 IMU, PID control and PWM-based motor control.

## Overview

The robot is designed as an inverted pendulum system. Since the robot naturally tends to fall, the controller continuously measures its tilt angle and adjusts the motor speed and direction to maintain balance.

The project combines sensor data acquisition, orientation estimation, PID control and motor actuation.

## Hardware

* ESP32-S3
* MPU6050 IMU
* L298N motor driver
* Two DC geared motors
* Two-wheel chassis
* Battery supply

## Software and Technologies

* C/C++
* Arduino framework
* ESP32-S3
* MPU6050
* PID control
* PWM motor control
* Complementary filtering

## System Working

The basic control loop is:

MPU6050
↓
Sensor data acquisition
↓
Orientation estimation
↓
PID controller
↓
Motor speed and direction calculation
↓
L298N motor driver
↓
DC motors
↓
Robot movement

The feedback loop continuously repeats this process to correct the robot's tilt.

## Orientation Estimation

The MPU6050 provides accelerometer and gyroscope measurements.

A complementary filter is used to combine the measurements and obtain a more stable estimate of the robot's orientation.

The accelerometer provides a reference for the long-term angle estimate, while the gyroscope provides faster short-term changes.

## PID Control

The estimated tilt angle is compared with the desired upright position.

The error is given by:

`error = desired_angle - measured_angle`

The PID controller uses proportional, integral and derivative terms to calculate the required motor correction.

`PID output = Kp × error + Ki × integral(error) + Kd × derivative(error)`

The controller output is then converted into motor direction and PWM control signals.

## Motor Control

The ESP32-S3 generates PWM signals to control the motor driver.

The motor driver controls:

* Motor direction
* Motor speed
* Forward and reverse correction

Motor corrections are continuously updated based on the PID output.

## Results

The implemented system demonstrates closed-loop balance control using real-time IMU feedback and PID-based motor correction.

The controller was tested by observing the robot's response to changes in its tilt and adjusting the PID parameters to improve stability.

