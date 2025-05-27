// === Projet Stream Deck ESP32 (WROOM) avec écran tactile et boutons ===
// Envoie des commandes texte via Serial au PC, à interpréter par un script Python

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>

#define TFT_CS   15
#define TFT_DC   2
#define TFT_RST  4
#define TOUCH_CS 21

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
XPT2046_Touchscreen ts(TOUCH_CS);

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define BUTTON_W 100
#define BUTTON_H 80

#define NUM_BUTTONS 9
const int buttonPins[NUM_BUTTONS] = {16, 5, 2, 13, 13, 12, 13, 14, 27};

struct Action {
  const char* label;
  const char* command;
};

struct Menu {
  const char* name;
  const uint16_t color;
  Action actions[9];
};

void backToMain();

Menu menus[] = {
  {
    "Spotify", ILI9341_GREEN,
    {
      {"Play/Pause", "spotify_playpause"},
      {"Next", "spotify_next"},
      {"Prev", "spotify_prev"},
      {"Vol+", "spotify_volup"},
      {"Vol-", "spotify_voldown"},
      {"Like", "spotify_like"},
      {"Shuffle", "spotify_shuffle"},
      {"Repeat", "spotify_repeat"},
      {"Retour", "menu_main"}
    }
  },
  {
    "VSCode", ILI9341_BLUE,
    {
      {"Build", "vscode_build"},
      {"Run", "vscode_run"},
      {"Stop", "vscode_stop"},
      {"Terminal", "vscode_terminal"},
      {"Find", "vscode_find"},
      {"Replace", "vscode_replace"},
      {"Comment", "vscode_comment"},
      {"Format", "vscode_format"},
      {"Retour", "menu_main"}
    }
  }
};

const int NUM_MENUS = sizeof(menus) / sizeof(Menu);
int currentMenu = -1;

void backToMain() {
  currentMenu = -1;
  showMainMenu();
}

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < NUM_BUTTONS; i++) pinMode(buttonPins[i], INPUT_PULLUP);
  
  pinMode(SWITCH_PIN, INPUT_PULLUP);  // <-- Ici pour G16

  tft.begin();
  ts.begin();
  ts.setRotation(1);
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  showMainMenu();
}

void showMainMenu() {
  tft.fillScreen(ILI9341_BLACK);
  for (int i = 0; i < NUM_MENUS && i < 9; i++) {
    int col = i % 3;
    int row = i / 3;
    int x = col * BUTTON_W;
    int y = row * BUTTON_H;

    tft.fillRect(x + 5, y + 5, BUTTON_W - 10, BUTTON_H - 10, menus[i].color);
    tft.setCursor(x + 20, y + 30);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(2);
    tft.print(menus[i].name);
  }
}

void showMenu(int index) {
  tft.fillScreen(ILI9341_BLACK);
  for (int i = 0; i < 9; i++) {
    int col = i % 3;
    int row = i / 3;
    int x = col * BUTTON_W;
    int y = row * BUTTON_H;

    tft.drawRect(x, y, BUTTON_W, BUTTON_H, ILI9341_WHITE);
    if (menus[index].actions[i].label[0] != '\0') {
      tft.setCursor(x + 10, y + 30);
      tft.setTextColor(ILI9341_YELLOW);
      tft.setTextSize(2);
      tft.print(menus[index].actions[i].label);
    }
  }
}

void sendCommand(const char* cmd) {
  if (strcmp(cmd, "menu_main") == 0) {
    backToMain();
  } else {
    Serial.println(cmd);
  }
}

void handleTouch() {
  if (!ts.touched()) return;
  TS_Point p = ts.getPoint();
  int x = p.y; 
  int y = SCREEN_HEIGHT - p.x;
  int col = x / BUTTON_W;
  int row = y / BUTTON_H;
  int index = row * 3 + col;

  if (index >= 0 && index < 9) {
    if (currentMenu == -1) {
      if (index < NUM_MENUS) {
        currentMenu = index;
        showMenu(index);
      }
    } else {
      const char* cmd = menus[currentMenu].actions[index].command;
      if (cmd) sendCommand(cmd);
    }
  }
  delay(250);
}

void handleButtons() {
  for (int i = 0; i < NUM_BUTTONS; i++) {
    if (digitalRead(buttonPins[i]) == LOW) {
      delay(200);
      if (currentMenu == -1 && i < NUM_MENUS) {
        currentMenu = i;
        Serial.println("Menu " + String(menus[i].name) + " activé");

        showMenu(i);
      } else if (currentMenu != -1) {
        const char* cmd = menus[currentMenu].actions[i].command;
        if (cmd) sendCommand(cmd);
      }
    Serial.println(buttonPins[i]);
    }
  }
}
void loop() {
  handleTouch();
  handleButtons();

}
