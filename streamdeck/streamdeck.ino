#include <WiFi.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <ArduinoJson.h>
#include <Preferences.h>

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
Preferences prefs;

const int buttonPins[NUM_BUTTONS] = {16, 17, 12, 25, 21, 14, 32, 34, 13};
bool buttonStates[NUM_BUTTONS] = {HIGH};
bool lastButtonStates[NUM_BUTTONS] = {HIGH};
unsigned long lastDebounceTimes[NUM_BUTTONS] = {0};
unsigned long buttonPressStart[NUM_BUTTONS] = {0};
const unsigned long debounceDelay = 50;

WiFiManager wm;
WiFiManagerParameter custom_host("host", "Adresse IP du serveur", "192.168.0.164", 32);
WiFiManagerParameter custom_port("port", "Port du serveur", "8080", 6);
String serverHost;
int serverPort;

String menus[] = {"1","2","3","4","5","6","7","8"};
int currentMenu = -1;
bool monitoringActive = false;

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < NUM_BUTTONS; i++) pinMode(buttonPins[i], INPUT_PULLUP);
  tft.begin();
  tft.setRotation(1);
  connectToWiFi();
  showMainMenu();
}

void loop() {
  handleButtons();

  static unsigned long lastRefresh = 0;
  if (monitoringActive && millis() - lastRefresh > 5000) {
    updateMetricsFromServer();
    lastRefresh = millis();
  }

  static unsigned long lastWiFiCheck = 0;
  if (millis() - lastWiFiCheck > 30000) {
    checkWiFi();
    lastWiFiCheck = millis();
  }
}

void connectToWiFi() {
  prefs.begin("config", true);
  String savedHost = prefs.getString("host", "");
  int savedPort = prefs.getInt("port", 0);
  prefs.end();

  if (savedHost != "" && savedPort > 0) {
    WiFi.mode(WIFI_STA);
    WiFi.begin();
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 15000) delay(250);
    if (WiFi.status() == WL_CONNECTED) {
      serverHost = savedHost;
      serverPort = savedPort;
      return;
    }
  }
  wm.addParameter(&custom_host);
  wm.addParameter(&custom_port);
  wm.setConfigPortalTimeout(180);
  WiFi.mode(WIFI_STA);
  if (!wm.autoConnect("ESP_Config", "admin123")) ESP.restart();
  serverHost = custom_host.getValue();
  serverPort = atoi(custom_port.getValue());
  prefs.begin("config", false);
  prefs.putString("host", serverHost);
  prefs.putInt("port", serverPort);
  prefs.end();
}

void checkWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi d\u00e9connect\u00e9. Tentative de reconnexion...");
    WiFi.disconnect();
    WiFi.begin();
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
      delay(500);
      Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED) Serial.println("Reconnect\u00e9 !");
    else Serial.println("\u00c9chec reconnexion !");
  }
}

void handleButtons() {
  unsigned long now = millis();
  for (int i = 0; i < NUM_BUTTONS; i++) {
    int reading = digitalRead(buttonPins[i]);
    if (reading != lastButtonStates[i]) lastDebounceTimes[i] = now;
    if ((now - lastDebounceTimes[i]) > debounceDelay) {
      if (reading != buttonStates[i]) {
        buttonStates[i] = reading;
        if (buttonStates[i] == LOW) {
          if (currentMenu == -1 && i < 8 && buttonPins[i] != 13 && buttonPins[i] != 34) {
            currentMenu = i;
            showMenu(i);
            return;
          }
          if (currentMenu != -1 && buttonPins[i] != 34) buttonPressStart[i] = now;
          if (buttonPins[i] == 13) {
            buttonPressStart[i] = now;
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
          if (monitoringActive && buttonPins[i] == 13 && duration >= 1000) {
            prefs.begin("config", false);
            prefs.clear();
            prefs.end();
            wm.resetSettings();
            ESP.restart();
          }
          if (currentMenu != -1 && buttonPins[i] == 13) backToMain();
        }
      }
    }
    lastButtonStates[i] = reading;
  }
}

void showMainMenu() {
  tft.fillScreen(ILI9341_BLACK);
  for (int i = 0; i < 8 && i < 9; i++) {
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

void drawImageFromURL(const char* filename, int x, int y, int width, int height) {
  char url[128];
  sprintf(url, "http://%s:%d/images/%s", serverHost.c_str(), serverPort, filename);
  Serial.printf("Image: %s\n", url);
  HTTPClient http;
  http.setTimeout(5000);
  http.begin(url);
  http.addHeader("Connection", "close");
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
    Serial.printf("Erreur HTTP image: %d\n", httpCode);
    tft.drawRect(x, y, width, height, ILI9341_WHITE);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(2);
    tft.setCursor(x + 10, y + 10);
    tft.print("404");
  }
  http.end();
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
    if (!deserializeJson(doc, payload)) {
      showSystemMonitor(doc["cpu"], doc["gpu"], doc["ram"], doc["disk"], doc["temp"]);
    }
  } else {
    Serial.printf("Erreur HTTP metrics: %d\n", httpCode);
  }
  http.end();
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
