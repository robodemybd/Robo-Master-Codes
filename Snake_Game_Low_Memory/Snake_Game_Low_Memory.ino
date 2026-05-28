#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>


U8G2_SSD1306_128X64_NONAME_1_HW_I2C display(U8G2_R0, U8X8_PIN_NONE);

uint16_t pos[400] = { 600, 400, 200, 0 };
byte foodx, foody;
byte dir = 3;
byte len = 4;
byte dl = 100;

bool eat = 1;
uint32_t st1;
bool sb1 = 1, sb2 = 1, sb3 = 1, sb4 = 1;


#define SW 2
#define up 3
#define down 4
#define left 5
#define right 6

void setup() {
  pinMode(SW, INPUT_PULLUP);
  pinMode(left, INPUT_PULLUP);
  pinMode(right, INPUT_PULLUP);
  pinMode(up, INPUT_PULLUP);
  pinMode(down, INPUT_PULLUP);
  randomSeed(analogRead(A0));
  display.begin();
  display.setFont(u8g2_font_ncenB14_tr);
}

void loop() {
  while (digitalRead(SW) == 1) {
    display.firstPage();  //display clear
    do {
      text("GAME MODE", 0, 16);
      text("EASY", 35, 38);
      text("HARD", 35, 60);
      (dl == 100) ? text(">>", 10, 38) : text(">>", 10, 60);
    } while (display.nextPage());
    if (!digitalRead(up)) dl = 100;
    if (!digitalRead(down)) dl = 50;
  }
  while (!digitalRead(SW)) delay(10);
  while (1) {
    if (eat) food();
    draw();
    if (millis() - st1 > dl) {
      motion();
      st1 = millis();
    }
    button();
    for (uint16_t i = 1; i < len; i++) {
      if (pos[0] == pos[i]) {
        game_over(len - 4);
        len = 4;
      }
    }
    if (!digitalRead(SW)) game_pause(len - 4);
  }
}

void draw() {
  display.firstPage();
  do {
    for (uint16_t i = 0; i < len; i++)
      display.drawBox((pos[i] / 100) * 3, (pos[i] % 100) * 3, 3, 3);

    for (uint16_t i = 0; i < len - 1; i++) {
      int deltaX = (pos[i + 1] / 100) - (pos[i] / 100);
      int deltaY = (pos[i + 1] % 100) - (pos[i] % 100);
      if (abs(deltaX) <= 2 && abs(deltaY) <= 2)
        display.drawBox(((pos[i] / 100) + (deltaX / 2)) * 3, ((pos[i] % 100) + (deltaY / 2)) * 3, 3, 3);
    }

    display.drawBox(foodx * 3, foody * 3, 3, 3);
  } while (display.nextPage());
}

void motion() {

  int posx = pos[0] / 100;
  int posy = pos[0] % 100;

  for (int i = len; i > 0; i--) pos[i] = pos[i - 1];

  if (dir == 1) posx -= 2;
  else if (dir == 3) posx += 2;
  else if (dir == 2) posy -= 2;
  else if (dir == 4) posy += 2;

  if (posx > 40) posx = 0;
  else if (posx < 0) posx = 40;
  if (posy > 20) posy = 0;
  else if (posy < 0) posy = 20;

  pos[0] = posx * 100 + posy;
  if (pos[0] == 100 * foodx + foody) {
    eat = 1;
    len++;
  }
}

void button() {
  if (!digitalRead(up)) {
    if (sb1) {
      if (dir != 4) dir = 2;
      sb1 = 0;
    }
    delay(10);
  } else sb1 = 1;

  if (!digitalRead(down)) {
    if (sb2) {
      if (dir != 2) dir = 4;
      sb2 = 0;
    }
    delay(10);
  } else sb2 = 1;

  if (!digitalRead(left)) {
    if (sb3) {
      if (dir != 3) dir = 1;
      sb3 = 0;
    }
    delay(10);
  } else sb3 = 1;

  if (!digitalRead(right)) {
    if (sb4) {
      if (dir != 1) dir = 3;
      sb4 = 0;
    }
    delay(10);
  } else sb4 = 1;
}

void food() {
  eat = 0;
generate_food:
  foodx = (random(20)) * 2;
  foody = (random(10)) * 2;
  for (uint16_t i = 0; i < len; i++)
    if (pos[i] == 100 * foodx + foody) goto generate_food;
}

void text(const char *t, byte x, byte y) {
  display.drawStr(x, y, t);
}

void game_over(int a) {
  display.firstPage();
  do {
    text("GAME OVER", 0, 24);
    char score[20];
    sprintf(score, "SCORE:%d", 50);
    text(score, 4, 48);
  } while (display.nextPage());
  while (digitalRead(SW)) delay(20);
  while (!digitalRead(SW)) delay(20);
}

void game_pause(int a) {
  while (!digitalRead(SW)) delay(20);
  display.firstPage();
  do {
    text("  PAUSED  ", 4, 24);
    char score[20];
    sprintf(score, "SCORE:%d", a);
    text(score, 4, 50);
  } while (display.nextPage());
  while (digitalRead(SW)) delay(20);
  while (!digitalRead(SW)) delay(20);
}
