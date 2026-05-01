#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MAX30105.h"
#include "heartRate.h"

MAX30105 particleSensor;

// ================== OLED ==================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ================== BUZZER ==================
#define BUZZER_PIN 13

// ================== HEART RATE ==================
const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;
int beatAvg = 0;

// ================== SPO2 ==================
double avered = 0, aveir = 0;
double sumirrms = 0, sumredrms = 0;
double SpO2 = 0, ESpO2 = 90.0;

const double FSpO2 = 0.7;
const double frate = 0.95;

int sampleCount = 0;
const int Num = 30;

#define FINGER_ON 7000
#define MINIMUM_SPO2 90.0

// ================== ICONS ==================
static const unsigned char PROGMEM heartSmall[] = {
  0x03,0xC0,0xF0,0x06,0x71,0x8C,0x0C,0x1B,
  0x06,0x18,0x0E,0x02,0x10,0x0C,0x03,0x10,
  0x04,0x01,0x10,0x04,0x01,0x10,0x40,0x01,
  0x10,0x40,0x01,0x10,0xC0,0x03,0x08,0x88,
  0x02,0x08,0xB8,0x04,0xFF,0x37,0x08,0x01,
  0x30,0x18,0x01,0x90,0x30,0x00,0xC0,0x60,
  0x00,0x60,0xC0,0x00,0x31,0x80,0x00,0x1B,
  0x00,0x00,0x0E,0x00,0x00,0x04,0x00
};

static const unsigned char PROGMEM O2_icon[] = {
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xf0,
  0x00,0x3f,0xc3,0xf8,0x00,0xff,0xf3,0xfc,
  0x03,0xff,0xff,0xfe,0x07,0xff,0xff,0xfe,
  0x0f,0xff,0xff,0xfe,0x0f,0xff,0xff,0x7e,
  0x1f,0x80,0xff,0xfc,0x1f,0x00,0x7f,0xb8,
  0x3e,0x3e,0x3f,0xb0,0x3e,0x3f,0x3f,0xc0,
  0x3e,0x3f,0x1f,0xc0,0x3e,0x3f,0x1f,0xc0,
  0x3e,0x3f,0x1f,0xc0,0x3e,0x3e,0x2f,0xc0,
  0x3e,0x3f,0x0f,0x80,0x1f,0x1c,0x2f,0x80,
  0x1f,0x80,0xcf,0x80,0x1f,0xe3,0x9f,0x00,
  0x0f,0xff,0x3f,0x00,0x07,0xfe,0xfe,0x00,
  0x0b,0xfe,0x0c,0x00,0x1d,0xff,0xf8,0x00,
  0x1e,0xff,0xe0,0x00,0x1f,0xff,0x00,0x00,
  0x1f,0xf0,0x00,0x00,0x1f,0xe0,0x00,0x00,
  0x0f,0xe0,0x00,0x00,0x07,0x80,0x00,0x00
};

// ================== FUNCTIONS ==================

void resetValues() {
  for (byte i = 0; i < RATE_SIZE; i++) rates[i] = 0;
  rateSpot = 0;
  beatAvg = 0;
  lastBeat = 0;

  avered = aveir = 0;
  sumirrms = sumredrms = 0;
  SpO2 = 0;
  ESpO2 = 90.0;
}

void drawDisplay() {
  display.clearDisplay();

  // Heart
  display.drawBitmap(5, 5, heartSmall, 24, 21, WHITE);
  display.setTextSize(2);
  display.setCursor(42, 10);
  display.print(beatAvg);
  display.println(" BPM");

  // SpO2
  display.drawBitmap(0, 35, O2_icon, 32, 32, WHITE);
  display.setCursor(42, 40);

  if (beatAvg > 30) display.print(String(ESpO2) + "%");
  else display.print("---- %");

  display.display();
}

// ================== SETUP ==================

void setup() {
  Serial.begin(115200);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30102 not found!");
    while (1);
  }

  particleSensor.setup(127, 4, 2, 800, 215, 16384);
  particleSensor.setPulseAmplitudeRed(0x0A);
  particleSensor.setPulseAmplitudeGreen(0);
}

// ================== LOOP ==================

void loop() {
  long irValue = particleSensor.getIR();

  if (irValue < FINGER_ON) {
    resetValues();

    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(30, 10);
    display.println("Finger");
    display.setCursor(30, 35);
    display.println("Please");
    display.display();

    noTone(BUZZER_PIN);
    return;
  }

  // ================= HEART RATE =================
  if (checkForBeat(irValue)) {
    long delta = millis() - lastBeat;
    lastBeat = millis();

    float bpm = 60 / (delta / 1000.0);

    if (bpm > 20 && bpm < 255) {
      rates[rateSpot++] = (byte)bpm;
      rateSpot %= RATE_SIZE;

      beatAvg = 0;
      for (byte i = 0; i < RATE_SIZE; i++) beatAvg += rates[i];
      beatAvg /= RATE_SIZE;
    }

    tone(BUZZER_PIN, 1000);
    delay(10);
    noTone(BUZZER_PIN);
  }

  // ================= SPO2 =================
  particleSensor.check();

  if (particleSensor.available()) {
    double ir = particleSensor.getFIFOIR();
    double red = particleSensor.getFIFORed();

    aveir = aveir * frate + ir * (1.0 - frate);
    avered = avered * frate + red * (1.0 - frate);

    sumirrms += (ir - aveir) * (ir - aveir);
    sumredrms += (red - avered) * (red - avered);

    sampleCount++;

    if (sampleCount % Num == 0) {
      double R = (sqrt(sumirrms) / aveir) / (sqrt(sumredrms) / avered);
      SpO2 = -23.3 * (R - 0.4) + 100;

      ESpO2 = FSpO2 * ESpO2 + (1.0 - FSpO2) * SpO2;

      if (ESpO2 > 100) ESpO2 = 99.9;
      if (ESpO2 < MINIMUM_SPO2) ESpO2 = MINIMUM_SPO2;

      sumirrms = sumredrms = 0;
    }

    particleSensor.nextSample();
  }

  // ================= OUTPUT =================
  Serial.print("BPM: ");
  Serial.print(beatAvg);
  Serial.print("  SpO2: ");
  Serial.println(ESpO2);

  drawDisplay();
}