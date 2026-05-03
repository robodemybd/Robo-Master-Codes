#include <Servo.h>
Servo servo[4];

const int left_joyX_pin = 0;
const int left_joyY_pin = 1;
const int right_joyX_pin = 2;
const int right_joyY_pin = 3;
const int servo_pin[4] = { 3, 5, 6, 9 };
int dir[4] = { 0, 0, 0, 0 };
int pos[4] = { 90, 90, 90, 90 };
int pos_min[4] = { 0, 0, 0, 0 };
int pos_max[4] = { 180, 180, 180, 180 };

int left_joystickX, right_joystickX, left_joystickY, right_joystickY;

void setup() {
  Serial.begin(9600);
  for (byte i = 0; i < 4; i++) {
    servo[i].attach(servo_pin[i]);
    servo[i].write(pos[i]);
  }
}

void loop() {
  read_joystick();
  move_servo();
}

void read_joystick() {
  left_joystickX = analogRead(left_joyX_pin);
  left_joystickY = analogRead(left_joyY_pin);
  right_joystickX = analogRead(right_joyX_pin);
  right_joystickY = analogRead(right_joyY_pin);

  if (left_joystickX < 200) dir[0] = -1;
  else if (left_joystickX > 800) dir[0] = 1;
  else dir[0] = 0;

  if (left_joystickY < 200) dir[1] = -1;
  else if (left_joystickY > 800) dir[1] = 1;
  else dir[1] = 0;

  if (right_joystickY < 200) dir[2] = -1;
  else if (right_joystickY > 800) dir[2] = 1;
  else dir[2] = 0;

  if (right_joystickX < 200) dir[3] = -1;
  else if (right_joystickX > 800) dir[3] = 1;
  else dir[3] = 0;
}

void move_servo() {
  for (byte i = 0; i < 4; i++) {
    if (dir[i] != 0) {
      pos[i] += dir[i];
      pos[i] = constrain(pos[i], pos_min[i], pos_max[i]); //this to limit the servo movement
    }
    Serial.print(String(pos[i]) + " ");
  }
  Serial.println();
  delay(10);  //for smooth movement
}