#include <Servo.h>
Servo servo_x, servo_y;

const int joystick_x = A0, joystick_y = A1, button = 2, laser = 3;
const int threshold_min = 200, threshold_max = 800;
int x_pos = 90, y_pos = 90;

void setup() {
  pinMode(button, INPUT_PULLUP);
  pinMode(laser, OUTPUT);
  servo_x.attach(9);
  servo_y.attach(10);
  servo_x.write(x_pos);
  servo_y.write(y_pos);
}

void loop() {
  (digitalRead(button) == 0) ? digitalWrite(laser, 1) : digitalWrite(laser, 0);

  int x = analogRead(joystick_x);
  int y = analogRead(joystick_y);

  if (x < threshold_min && x_pos < 180) x_pos++;
  if (x > threshold_max && x_pos > 0) x_pos--;
  if (y < threshold_min && y_pos < 180) y_pos++;
  if (y > threshold_max && y_pos > 0) y_pos--;

  servo_x.write(x_pos);
  servo_y.write(y_pos);
  delay(10);
}
