#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(128, 64, &Wire);

int upBtn = 2, downBtn = 3, setBtn = 4;
int buzzer = 8;

int timeSet = 0;  // seconds
unsigned long lastTime = 0;
bool running = false;
bool alarm = false;

// Button press detection function
byte press(byte pin) {
  if (digitalRead(pin) == 0) {
    delay(50);
    int timer = 0;
    bool long_press = 0;
    while (digitalRead(pin) == 0) {
      delay(50);
      timer += 50;
      if (timer > 1000 && long_press == 0) long_press = 1;
    }
    if (long_press == 1) return 2;
    else return 1;
  }
  return 0;
}

void time_show() {
  // Display
  display.clearDisplay();
  display.setCursor(0, 24);
  display.print(timeSet);
  display.print(" sec");
  display.display();
}

void setup() {
  pinMode(upBtn, INPUT_PULLUP);
  pinMode(downBtn, INPUT_PULLUP);
  pinMode(setBtn, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextSize(2);
  display.setTextColor(1);
}

void loop() {

  byte setPress = press(setBtn);

  // RESET (long press)
  if (setPress == 2) {
    timeSet = 0;
    running = false;
    alarm = false;
  }

  // START / PAUSE (short press)
  if (setPress == 1) {
    if (timeSet != 0) running = !running;
    alarm = false;
    lastTime = millis();
  }

  // Adjust time only when not running
  if (!running && !alarm) {
    if (press(upBtn) >= 1) {
      timeSet += 10;
      time_show();
    }
    if (press(downBtn) >= 1 && timeSet >= 10) {
      timeSet -= 10;
      time_show();
    }
  }

  // Countdown using millis()
  if (running && timeSet > 0) {
    if (millis() - lastTime >= 1000) {
      lastTime = millis();
      timeSet--;
      time_show();
    }
  }

  // Time up
  if (timeSet == 0 && running) {
    running = false;
    alarm = true;
    display.clearDisplay();
    display.setCursor(0, 24);
    display.print("Time Up!");
    display.display();
  }

  // Alarm beeping
  if (alarm) {
    digitalWrite(buzzer, HIGH);
    delay(100);
    digitalWrite(buzzer, LOW);
    delay(100);
  }
}