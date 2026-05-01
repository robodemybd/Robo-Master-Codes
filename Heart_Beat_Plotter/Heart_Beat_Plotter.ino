#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MAX30105.h"
#include "heartRate.h"

MAX30105 particleSensor;

// ================= OLED =================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ================= BUZZER =================
#define BUZZER_PIN 13

// ================= HEART =================
const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;
int beatAvg = 0;

// ================= SPO2 =================
double avered = 0, aveir = 0;
double sumirrms = 0, sumredrms = 0;
double SpO2 = 0, ESpO2 = 90.0;

const double FSpO2 = 0.7;
const double frate = 0.95;

int sampleCount = 0;
const int Num = 30;

#define FINGER_ON 7000
#define MINIMUM_SPO2 90.0

// ================= GRAPH =================
#define GRAPH_WIDTH 128
#define GRAPH_HEIGHT 30
#define GRAPH_Y_OFFSET 34

uint8_t graph[GRAPH_WIDTH];   // Y values
int graphIndex = 0;

long irMin = 999999, irMax = 0;

// ================= FUNCTIONS =================

void resetValues() {
  for (byte i = 0; i < RATE_SIZE; i++) rates[i] = 0;
  rateSpot = 0;
  beatAvg = 0;
  lastBeat = 0;

  avered = aveir = 0;
  sumirrms = sumredrms = 0;
  SpO2 = 0;
  ESpO2 = 90.0;

  irMin = 999999;
  irMax = 0;
}

void updateGraph(long irValue) {
  // Track min/max for normalization
  if (irValue < irMin) irMin = irValue;
  if (irValue > irMax) irMax = irValue;

  // Avoid divide by zero
  if (irMax - irMin < 1000) return;

  int y = map(irValue, irMin, irMax, GRAPH_HEIGHT, 0);

  graph[graphIndex] = y;
  graphIndex = (graphIndex + 1) % GRAPH_WIDTH;
}

void drawGraph() {
  for (int x = 0; x < GRAPH_WIDTH - 1; x++) {
    int i1 = (graphIndex + x) % GRAPH_WIDTH;
    int i2 = (graphIndex + x + 1) % GRAPH_WIDTH;

    display.drawLine(x, GRAPH_Y_OFFSET + graph[i1],
                     x + 1, GRAPH_Y_OFFSET + graph[i2],
                     WHITE);
  }
}

void drawDisplay() {
  display.clearDisplay();

  // BPM text
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print(beatAvg);
  display.print(" BPM");

  // SpO2 text
  display.setCursor(0, 16);
  if (beatAvg > 30) display.print(String(ESpO2) + "%");
  else display.print("-- %");

  // Draw waveform
  drawGraph();

  display.display();
}

// ================= SETUP =================

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

// ================= LOOP =================

void loop() {
  long irValue = particleSensor.getIR();

  if (irValue < FINGER_ON) {
    resetValues();

    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(20, 20);
    display.println("No Finger");
    display.display();

    return;
  }

  // ===== HEART RATE =====
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

  // ===== SPO2 =====
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

  // ===== GRAPH UPDATE =====
  updateGraph(irValue);

  // ===== OUTPUT =====
  Serial.print("BPM: ");
  Serial.print(beatAvg);
  Serial.print("  SpO2: ");
  Serial.println(ESpO2);

  drawDisplay();
}