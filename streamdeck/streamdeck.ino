#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
#include "secrets.h"

#define TFT_CS   15
#define TFT_DC   4
#define TFT_RST  22
#define TOUCH_CS 26

#define IMG_WIDTH 100
#define IMG_HEIGHT 75

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define BUTTON_W 100
#define BUTTON_H 80
#define NUM_BUTTONS 9

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
XPT2046_Touchscreen ts(TOUCH_CS);

const int buttonPins[NUM_BUTTONS] = {16, 17, 12, 25, 21, 14, 32, 34, 13};
unsigned long lastDebounceTimes[NUM_BUTTONS] = {0};
const unsigned long debounceDelay = 50;

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;
const char* host = "192.168.0.22";
const int port = 8080;

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
int currentMenu = -1;

Menu menus[] = {
  {
    "spotify", ILI9341_GREEN,
    {
      {"Play/Pause", "spotify_playpause"}, {"Next", "spotify_next"}, {"Prev", "spotify_prev"},
      {"Vol+", "spotify_volup"}, {"Vol-", "spotify_voldown"}, {"Like", "spotify_like"},
      {"Shuffle", "spotify_shuffle"}, {"Repeat", "spotify_repeat"}, {"Retour", "menu_main"}
    }
  },
  {
    "VSCode", ILI9341_BLUE,
    {
      {"Build", "vscode_build"}, {"Run", "vscode_run"}, {"Stop", "vscode_stop"},
      {"Terminal", "vscode_terminal"}, {"Find", "vscode_find"}, {"Replace", "vscode_replace"},
      {"Comment", "vscode_comment"}, {"Format", "vscode_format"}, {"Retour", "menu_main"}
    }
  },
  {
    "VSCode", ILI9341_BLUE,
    {
      {"Build", "vscode_build"}, {"Run", "vscode_run"}, {"Stop", "vscode_stop"},
      {"Terminal", "vscode_terminal"}, {"Find", "vscode_find"}, {"Replace", "vscode_replace"},
      {"Comment", "vscode_comment"}, {"Format", "vscode_format"}, {"Retour", "menu_main"}
    }
  },
  {
    "VSCode", ILI9341_BLUE,
    {
      {"Build", "vscode_build"}, {"Run", "vscode_run"}, {"Stop", "vscode_stop"},
      {"Terminal", "vscode_terminal"}, {"Find", "vscode_find"}, {"Replace", "vscode_replace"},
      {"Comment", "vscode_comment"}, {"Format", "vscode_format"}, {"Retour", "menu_main"}
    }
  },
  {
    "VSCode", ILI9341_BLUE,
    {
      {"Build", "vscode_build"}, {"Run", "vscode_run"}, {"Stop", "vscode_stop"},
      {"Terminal", "vscode_terminal"}, {"Find", "vscode_find"}, {"Replace", "vscode_replace"},
      {"Comment", "vscode_comment"}, {"Format", "vscode_format"}, {"Retour", "menu_main"}
    }
  },
  {
    "VSCode", ILI9341_BLUE,
    {
      {"Build", "vscode_build"}, {"Run", "vscode_run"}, {"Stop", "vscode_stop"},
      {"Terminal", "vscode_terminal"}, {"Find", "vscode_find"}, {"Replace", "vscode_replace"},
      {"Comment", "vscode_comment"}, {"Format", "vscode_format"}, {"Retour", "menu_main"}
    }
  },
  {
    "VSCode", ILI9341_BLUE,
    {
      {"Build", "vscode_build"}, {"Run", "vscode_run"}, {"Stop", "vscode_stop"},
      {"Terminal", "vscode_terminal"}, {"Find", "vscode_find"}, {"Replace", "vscode_replace"},
      {"Comment", "vscode_comment"}, {"Format", "vscode_format"}, {"Retour", "menu_main"}
    }
  },
  {
    "abc", ILI9341_BLUE,
    {
      {"Build", "vscode_build"}, {"Run", "vscode_run"}, {"Stop", "vscode_stop"},
      {"Terminal", "vscode_terminal"}, {"Find", "vscode_find"}, {"Replace", "vscode_replace"},
      {"Comment", "vscode_comment"}, {"Format", "vscode_format"}, {"Retour", "menu_main"}
    }
  }
};

const int NUM_MENUS = sizeof(menus) / sizeof(Menu);

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < NUM_BUTTONS; i++) pinMode(buttonPins[i], INPUT_PULLUP);

  tft.begin();
  ts.begin();
  ts.setRotation(1);
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("Connexion WiFi...");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connecté !");
  tft.println("WiFi OK !");
  delay(500);
  
  showMainMenu();
}

void drawImageFromURL(const char* filename, int x, int y) {
  char url[128];
  sprintf(url, "http://%s:%d/%s", host, port, filename);
  HTTPClient http;
  http.setTimeout(5000);
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    WiFiClient* stream = http.getStreamPtr();
    uint16_t line[IMG_WIDTH];

    tft.startWrite();
    for (int row = 0; row < IMG_HEIGHT; row++) {
      for (int col = 0; col < IMG_WIDTH; col++) {
        while (!stream->available());
        uint8_t hi = stream->read();
        while (!stream->available());
        uint8_t lo = stream->read();
        line[col] = (hi << 8) | lo;
      }
      tft.setAddrWindow(x, y + row, x + IMG_WIDTH - 1, y + row);
      tft.writePixels(line, IMG_WIDTH, true);
    }
    tft.endWrite();

    Serial.println("Image affichée !");
  } else {
    Serial.print("Erreur HTTP : ");
    Serial.println(httpCode);

    // Dessine un rectangle de fond rouge
    tft.fillRect(x, y, IMG_WIDTH, IMG_HEIGHT, ILI9341_RED);

    // Écrit "404" en blanc au centre
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(2);
    int16_t tx = x + (IMG_WIDTH / 2) - (3 * 6); // approx. centré pour "404"
    int16_t ty = y + (IMG_HEIGHT / 2) - 8;
    tft.setCursor(tx, ty);
    tft.print("404");
  }

  http.end();
}




void showMainMenu() {
  tft.fillScreen(ILI9341_BLACK);
  for (int i = 0; i < NUM_MENUS && i < 9; i++) {
    int col = i % 3;
    int row = i / 3;
    int x = col * IMG_WIDTH;
    int y = row * IMG_HEIGHT;

    char filename[32];
    sprintf(filename, "%s.rgb", menus[i].name);  // correspond au nom du fichier
    drawImageFromURL(filename, x, y);
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

void handleButtons() {
  unsigned long currentTime = millis();
  for (int i = 0; i < NUM_BUTTONS; i++) {
    if (digitalRead(buttonPins[i]) == LOW) {
      if (currentTime - lastDebounceTimes[i] > debounceDelay) {
        lastDebounceTimes[i] = currentTime;

        if (currentMenu == -1 && i < NUM_MENUS) {
          if(buttonPins[i]!=34){
          currentMenu = i;
          showMenu(i);
          }
        } else if (currentMenu != -1) {
          if (buttonPins[i] == 13) {
            backToMain();
            break;
          }
          if(buttonPins[i]!=34){
          const char* cmd = menus[currentMenu].actions[i].command;
          Serial.println(String("Bouton: ") + buttonPins[i] + " => " + cmd);
          }

        }
      }
    }
  }
}

void backToMain() {
  currentMenu = -1;
  showMainMenu();
}

void loop() {
  handleButtons();
}
