#include <Servo.h>
Servo servo;

const int left_ldr = 4;
const int right_ldr = 5;

int servo_angle = 90;

void setup() {
  servo.attach(10);
  servo.write(servo_angle);
  pinMode(left_ldr, INPUT);
  pinMode(right_ldr, INPUT);
}

void loop() {
  if (digitalRead(left_ldr) == 1 && servo_angle < 180) servo_angle++;
  if (digitalRead(right_ldr) == 1 && servo_angle > 0) servo_angle--;
  servo.write(servo_angle);
  delay(10);
}
