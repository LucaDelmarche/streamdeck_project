#include <WiFi.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>

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

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

const int buttonPins[NUM_BUTTONS] = {16, 17, 12, 25, 21, 14, 32, 34, 13};
unsigned long lastDebounceTimes[NUM_BUTTONS] = {0};
const unsigned long debounceDelay = 50;
WiFiManager wm;

// Valeurs par défaut
WiFiManagerParameter custom_host("host", "Adresse IP du serveur", "192.168.0.22", 32);
WiFiManagerParameter custom_port("port", "Port du serveur", "8080", 6);

// Variables pour plus tard
String serverHost;
int serverPort;
String wifiName = "ESP_Config";
String wifiPassword = "admin123";
void backToMain();
int currentMenu = -1;

String menus[] = {"1","2","3","4","5","6","7","8"};

const int NUM_MENUS = sizeof(menus) / sizeof(menus[0]);

void setup() {
  Serial.begin(115200);
  // wm.resetSettings();
  // Configuration des boutons
  for (int i = 0; i < NUM_BUTTONS; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  // Initialisation de l'écran TFT
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("Connexion WiFi...");

  // Préparation des paramètres personnalisés
  wm.addParameter(&custom_host);
  wm.addParameter(&custom_port);

  wm.setConfigPortalTimeout(180);  // Timeout si le portail est lancé
  WiFi.mode(WIFI_STA);

  // Connexion auto ou AP si échec
  bool res = wm.autoConnect(wifiName.c_str(), wifiPassword.c_str());

  if (!res) {
    Serial.println("Connexion échouée ou portail expiré.");
    ESP.restart();
  }

  // Affichage générique d'information
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_GREEN);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("WiFi connecté !");

  tft.setTextSize(1);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(10, 40);
  tft.print("IP locale : ");
  tft.println(WiFi.localIP());

  tft.setCursor(10, 60);
  tft.println("Si vous devez vous connecter :");

  tft.setCursor(10, 75);
  tft.print("SSID : ");
  tft.println(wifiName);

  tft.setCursor(10, 90);
  tft.print("MDP  : ");
  tft.println(wifiPassword);

  tft.setCursor(10, 105);
  tft.print("Portail : http://");
  tft.println(WiFi.softAPIP());  // S'affiche 0.0.0.0 si non actif

  delay(4000);  // Laisse le temps de lire

  // Récupération des valeurs de formulaire WiFiManager
  serverHost = custom_host.getValue();
  serverPort = atoi(custom_port.getValue());

  Serial.print("Host = ");
  Serial.println(serverHost);
  Serial.print("Port = ");
  Serial.println(serverPort);

  delay(2000);
  showMainMenu();
}






void drawImageFromURL(const char* filename, int x, int y, int width, int height) {
  char url[128];
  sprintf(url, "http://%s:%d/%s", serverHost.c_str(), serverPort, filename);

  HTTPClient http;
  http.setTimeout(5000);
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    WiFiClient* stream = http.getStreamPtr();

    uint16_t* lineBuffer = (uint16_t*)malloc(width * 2);
    if (!lineBuffer) {
      Serial.println("Erreur allocation ligne !");
      return;
    }

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
    Serial.println("Image OK (buffer ligne)");
  } else {
    Serial.print("Erreur HTTP : ");
    Serial.println(httpCode);
    tft.drawRect(x, y, width, height, ILI9341_WHITE);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(2);
    tft.setCursor(x + 10, y + 10);
    tft.print("404");
  }

  http.end();
}





void playAnimation(const char* basePath, int frameCount, int x, int y, int delayMs) {
  for (int i = 0; i < frameCount; i++) {
    char path[64];
    sprintf(path, "%sframe_%03d.rgb", basePath, i);
    drawImageFromURL(path, x, y,320,240);
    delay(delayMs);
  }
  showMainMenu();
}


void showMainMenu() {
  tft.fillScreen(ILI9341_BLACK);
  for (int i = 0; i < NUM_MENUS && i < 9; i++) {
    int col = i % 3;
    int row = i / 3;
    int x = col * IMG_WIDTH;
    int y = row * IMG_HEIGHT;

    char filename[32];
    sprintf(filename, "images/menu/%s.rgb", menus[i]);
    drawImageFromURL(filename, x, y,IMG_WIDTH,IMG_HEIGHT);
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

    // Image du bouton
    char filename[64];
    sprintf(filename, "images/sous_menu/%s/%d.rgb", menus[index], i);
    drawImageFromURL(filename, x, y,IMG_WIDTH,IMG_HEIGHT);

    // Cadre blanc
    tft.drawRect(x, y, BUTTON_W, BUTTON_H, ILI9341_WHITE);
  }
}


void handleButtons() {
  unsigned long currentTime = millis();
  for (int i = 0; i < NUM_BUTTONS; i++) {
    if (digitalRead(buttonPins[i]) == LOW) {
      if (currentTime - lastDebounceTimes[i] > debounceDelay) {
        lastDebounceTimes[i] = currentTime;

          if (currentMenu == -1) {
            if (buttonPins[i] == 13) {
              playAnimation("/Utils/frames/", 74, 0, 0, 350);
              return;
            }
            if (i < NUM_MENUS && buttonPins[i] != 34) {
              currentMenu = i;
              showMenu(i);
            }
          }
          else if (currentMenu != -1) {
          if (buttonPins[i] == 13) {
            backToMain();
            break;
          }
          if(buttonPins[i]!=34){
            Serial.println(i);
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
