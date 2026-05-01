#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
Adafruit_SSD1306 oled(128, 64, &Wire, -1);
#define SW 2
#define up 3
#define down 4
#define left 5
#define right 6

short int pos[800] = { 600, 400, 200, 000 };
short int foodx, foody, dir = 3, len = 4, dl = 100;
bool eat = 1;
uint32_t st1;
bool sb1 = 1, sb2 = 1, sb3 = 1, sb4 = 1;

void setup() {
  pinMode(SW, INPUT_PULLUP);
  pinMode(left, INPUT_PULLUP);
  pinMode(right, INPUT_PULLUP);
  pinMode(up, INPUT_PULLUP);
  pinMode(down, INPUT_PULLUP);
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  oled.clearDisplay();
  oled.setTextColor(1);
  oled.setTextSize(2);
  snake_game();
}

void loop() {
}

void snake_game() {
  bool menu = 1;
  //difficulty selection
  while (digitalRead(SW)) {
    oled.clearDisplay();
    text("GAME MODE?", 04, 0);
    text("   EASY   ", 04, 25);
    text("   HARD   ", 04, 50);
    (dl == 100) ? text(">>      <<", 04, 25) : text(">>      <<", 04, 50);
    oled.display();
    if (!digitalRead(up)) dl = 100;
    else if (!digitalRead(down)) dl = 50;
  }

  while (!digitalRead(SW))
    ;  //after menu selection, wait for the button to be released before proceed

  while (1) {
r:
    if (eat == 1) food();  //if the snake eats the food, redraw a new food in empty area
    draw();                //draw the snake

    //move the snake forward after a fixed interval
    if (millis() - st1 > dl) {
      motion();
      st1 = millis();
    }

    button();  //take button input to change directions

    for (short int i = 1; i < len; i++)
      if (pos[0] == pos[i]) {  //if the head touches any part of the body -> Game Over
        game_over(len - 4);    //pass the score to the game over screen
        len = 4;               //start again from the little snake
      }

    if (!digitalRead(SW)) game_pause(len - 4);
  }
}

void draw() {
  //fill the each coordinate value by 3*3 cube
  for (short i = 0; i < len; i++) oled.fillRect((pos[i] / 100) * 3, (pos[i] % 100) * 3, 3, 3, 1);

  //this is to fill up the corner bits
  for (short i = 0; i < len - 1; i++) {
    int deltaX = (pos[i + 1] / 100) - (pos[i] / 100);
    int deltaY = (pos[i + 1] % 100) - (pos[i] % 100);
    if (abs(deltaX) <= 2 && abs(deltaY) <= 2)
      oled.fillRect(((pos[i] / 100) + (deltaX / 2)) * 3, ((pos[i] % 100) + (deltaY / 2)) * 3, 3, 3, 1);
  }

  //draw food
  oled.fillRect(foodx * 3, foody * 3, 3, 3, 1);
  oled.display();
  oled.clearDisplay();
}

void motion() {
  //extract the x and y coordinate from a single value
  int posx = pos[0] / 100;
  int posy = pos[0] % 100;

  for (int i = len; i > 0; i--) pos[i] = pos[i - 1];  //move the entire body forward except the head

  //head pointing in direction
  if (dir == 1) posx -= 2;
  else if (dir == 3) posx += 2;
  else if (dir == 2) posy -= 2;
  else if (dir == 4) posy += 2;

  //if it goes out of the screen, than reappear from the other side
  if (posx > 40) posx = 0;
  else if (posx < 0) posx = 40;
  if (posy > 20) posy = 0;
  else if (posy < 0) posy = 20;

  //save the head coordinate
  pos[0] = posx * 100 + posy;

  //determine if the head has eaten the food
  if (pos[0] == 100 * foodx + foody) {
    eat = 1;  //flag for eating
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
f:
  foodx = (rand() % 20) * 2;
  foody = (rand() % 10) * 2;
  for (short int i = 0; i < len; i++)
    if (pos[i] == 100 * foodx + foody) goto f;  //if the food lands on snake body, try another random position
}

void game_over(int a) {
  oled.clearDisplay();
  oled.setTextSize(2);
  text("GAME OVER!", 04, 12);
  text("SCORE:" + String(a), 04, 48);
  oled.display();
  while (!digitalRead(SW))
    ;
}

void game_pause(int a) {
  oled.clearDisplay();
  oled.clearDisplay();
  text("GAME PAUSE", 04, 12);
  text("SCORE:" + String(a), 04, 48);
  oled.display();
  while (!digitalRead(SW))
    ;  //wait until the button is released
  while (digitalRead(SW))
    ;  //waiting for the button to be pressed again to exit the pause screen
  while (!digitalRead(SW))
    ;  //wait until the button is released to avoid double press
}

void text(String t, byte x, byte y) {
  oled.setCursor(x, y);
  oled.print(t);
}