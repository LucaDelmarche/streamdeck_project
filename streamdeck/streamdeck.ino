// === Librairies ===
#include <WiFi.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <ArduinoJson.h>

// === Configuration ===
#define TFT_CS   15
#define TFT_DC   4
#define TFT_RST  22
#define IMG_WIDTH 100
#define IMG_HEIGHT 75
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define BUTTON_W 100
#define BUTTON_H 80
#define NUM_BUTTONS 9

#define COLOR_CPU  ILI9341_BLUE
#define COLOR_GPU  ILI9341_CYAN
#define COLOR_RAM  ILI9341_MAGENTA
#define COLOR_DISK ILI9341_ORANGE
#define COLOR_TEMP ILI9341_RED

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// === Boutons ===
const int buttonPins[NUM_BUTTONS] = {16, 17, 12, 25, 21, 14, 32, 34, 13};
bool buttonStates[NUM_BUTTONS] = {HIGH};
bool lastButtonStates[NUM_BUTTONS] = {HIGH};
unsigned long lastDebounceTimes[NUM_BUTTONS] = {0};
unsigned long buttonPressStart[NUM_BUTTONS] = {0};
const unsigned long debounceDelay = 50;

// === WiFi Manager ===
WiFiManager wm;
WiFiManagerParameter custom_host("host", "Adresse IP du serveur", "192.168.0.22", 32);
WiFiManagerParameter custom_port("port", "Port du serveur", "8080", 6);

String serverHost;
int serverPort;
String wifiName = "ESP_Config";
String wifiPassword = "admin123";

String menus[] = {"1","2","3","4","5","6","7","8"};
const int NUM_MENUS = sizeof(menus) / sizeof(menus[0]);
int currentMenu = -1;
bool monitoringActive = false;

// === Fonctions utilitaires ===
void drawLoadingIndicator(int step) {
  const char* chars[] = {"/", "-", "\\", "|"};
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
  tft.setCursor(SCREEN_WIDTH - 20, 10);
  tft.print(chars[step % 4]);
}

void connectToWiFi() {
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("Connexion WiFi...");

  wm.addParameter(&custom_host);
  wm.addParameter(&custom_port);
  wm.setConfigPortalTimeout(180);
  WiFi.mode(WIFI_STA);

  unsigned long startTime = millis();
  while (!wm.autoConnect(wifiName.c_str(), wifiPassword.c_str())) {
    if (millis() - startTime > 20000) {
      tft.fillScreen(ILI9341_BLACK);
      tft.setTextColor(ILI9341_RED);
      tft.setCursor(10, 60);
      tft.setTextSize(2);
      tft.println("Connexion echouee!");
      delay(3000);
      ESP.restart();
    }
    drawLoadingIndicator((millis() / 250) % 4);
    delay(250);
  }

  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_GREEN);
  tft.setCursor(10, 10);
  tft.setTextSize(2);
  tft.println("WiFi connecte!");
  tft.setTextSize(1);
  tft.setCursor(10, 40); tft.print("IP : "); tft.println(WiFi.localIP());
  tft.setCursor(10, 60); tft.println("SSID : " + wifiName);
  tft.setCursor(10, 75); tft.println("MDP  : " + wifiPassword);
  delay(3000);

  serverHost = custom_host.getValue();
  serverPort = atoi(custom_port.getValue());
}

// === Setup ===
void setup() {
  Serial.begin(115200);
  for (int i = 0; i < NUM_BUTTONS; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
  tft.begin();
  tft.setRotation(1);
  connectToWiFi();
  showMainMenu();
}

// === Loop ===
void loop() {
  handleButtons();

  static unsigned long lastRefresh = 0;
  if (monitoringActive && millis() - lastRefresh > 5000) {
    updateMetricsFromServer();
    lastRefresh = millis();
  }
}

// === Gestion des boutons ===
void handleButtons() {
  unsigned long now = millis();

  for (int i = 0; i < NUM_BUTTONS; i++) {
    int reading = digitalRead(buttonPins[i]);
    if (reading != lastButtonStates[i]) lastDebounceTimes[i] = now;

    if ((now - lastDebounceTimes[i]) > debounceDelay) {
      if (reading != buttonStates[i]) {
        buttonStates[i] = reading;

        if (buttonStates[i] == LOW) {
          if (currentMenu == -1 && i < NUM_MENUS && buttonPins[i] != 13 && buttonPins[i] != 34) {
            currentMenu = i;
            showMenu(i);
            buttonPressStart[i] = 0;
            return;
          }
          if (currentMenu != -1 && buttonPins[i] != 34) {
            buttonPressStart[i] = now;
          }
          if (buttonPins[i] == 13) {
            if (currentMenu == -1) {
              monitoringActive = !monitoringActive;
              if (!monitoringActive) backToMain();
              else updateMetricsFromServer();
              return;
            }
          }
        } else if (buttonStates[i] == HIGH) {
          if (buttonPressStart[i] == 0) continue;
          unsigned long duration = now - buttonPressStart[i];
          buttonPressStart[i] = 0;

          if (currentMenu != -1 && buttonPins[i] != 34) {
            String cmd = String(currentMenu) + "_" + String(i);
            if (duration >= 600) cmd += "_long";
            Serial.println(cmd);
          }
          if (currentMenu != -1 && buttonPins[i] == 13) {
            backToMain();
          }
        }
      }
    }
    lastButtonStates[i] = reading;
  }
}

// === Menus ===
void showMainMenu() {
  tft.fillScreen(ILI9341_BLACK);
  for (int i = 0; i < NUM_MENUS && i < 9; i++) {
    int col = i % 3;
    int row = i / 3;
    int x = col * IMG_WIDTH;
    int y = row * IMG_HEIGHT;
    char filename[32];
    sprintf(filename, "menu/%s.rgb", menus[i]);
    drawImageFromURL(filename, x, y, IMG_WIDTH, IMG_HEIGHT);
    tft.drawRect(x, y, BUTTON_W, BUTTON_H, ILI9341_WHITE);
  }
}

void showMenu(int index) {
  tft.fillScreen(ILI9341_BLACK);
  for (int i = 0; i < 9; i++) {
    int col = i % 3;
    int row = i / 3;
    int x = col * BUTTON_W;
    int y = row * BUTTON_H;
    char filename[64];
    sprintf(filename, "sous_menu/%s/%d.rgb", menus[index], i);
    drawImageFromURL(filename, x, y, IMG_WIDTH, IMG_HEIGHT);
    tft.drawRect(x, y, BUTTON_W, BUTTON_H, ILI9341_WHITE);
  }
}

void backToMain() {
  currentMenu = -1;
  showMainMenu();
}

// === Images ===
void drawImageFromURL(const char* filename, int x, int y, int width, int height) {
  char url[128];
  sprintf(url, "http://%s:%d/images/%s", serverHost.c_str(), serverPort, filename);

  HTTPClient http;
  http.setTimeout(5000);
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    WiFiClient* stream = http.getStreamPtr();
    uint16_t* lineBuffer = (uint16_t*)malloc(width * 2);
    if (!lineBuffer) return;

    tft.startWrite();
    for (int row = 0; row < height; row++) {
      for (int col = 0; col < width; col++) {
        while (stream->available() < 2);
        uint8_t hi = stream->read();
        uint8_t lo = stream->read();
        lineBuffer[col] = (hi << 8) | lo;
      }
      tft.setAddrWindow(x, y + row, x + width - 1, y + row);
      tft.writePixels(lineBuffer, width, true);
    }
    tft.endWrite();
    free(lineBuffer);
  } else {
    tft.drawRect(x, y, width, height, ILI9341_WHITE);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(2);
    tft.setCursor(x + 10, y + 10);
    tft.print("404");
  }
  http.end();
}

// === Monitoring ===
void drawBar(int x, int y, int width, int height, int value, uint16_t color, const char* label) {
  int filled = map(value, 0, 100, 0, height);
  tft.drawRect(x, y, width, height, ILI9341_WHITE);
  tft.fillRect(x, y + (height - filled), width, filled, color);
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_WHITE);
  int textX = x + (width / 2) - (strlen(label) * 3);
  tft.setCursor(textX, y + height + 5);
  tft.print(label);
}

void showSystemMonitor(int cpu, int gpu, int ram, int disk, int temp) {
  tft.fillScreen(ILI9341_BLACK);
  int startX = 20, spacing = 55, barWidth = 30, barHeight = 100, topY = 60;
  drawBar(startX + spacing * 0, topY, barWidth, barHeight, cpu, COLOR_CPU, "CPU");
  drawBar(startX + spacing * 1, topY, barWidth, barHeight, gpu, COLOR_GPU, "GPU");
  drawBar(startX + spacing * 2, topY, barWidth, barHeight, ram, COLOR_RAM, "RAM");
  drawBar(startX + spacing * 3, topY, barWidth, barHeight, disk, COLOR_DISK, "DISK");
  drawBar(startX + spacing * 4, topY, barWidth, barHeight, temp, COLOR_TEMP, "TEMP");
}

void updateMetricsFromServer() {
  HTTPClient http;
  char url[128];
  sprintf(url, "http://%s:%d/metrics", serverHost.c_str(), serverPort);
  http.setTimeout(1500);
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    StaticJsonDocument<256> doc;
    DeserializationError err = deserializeJson(doc, payload);
    if (!err) {
      showSystemMonitor(
        doc["cpu"] | 0,
        doc["gpu"] | 0,
        doc["ram"] | 0,
        doc["disk"] | 0,
        doc["temp"] | 0
      );
    }
  } else {
    Serial.print("HTTP Error: ");
    Serial.println(httpCode);
  }
  http.end();
}
