#include <WiFi.h>
#include <WebServer.h>
#include <ElegantOTA.h>
#include <FastLED.h>

// --- Konfiguracja Wi-Fi (Uzupełnij własnymi danymi przed wgraniem) ---
const char* ssid     = "TWOJA_NAZWA_SIECI_WIFI";
const char* password = "TWOJE_HASLO_DO_WIFI";

WebServer server(80);

// --- Konfiguracja LED ---
#define NUM_LEDS    150
#define DATA_PIN    22
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

// --- Zmienne stanu (Sterowanie) ---
volatile bool ledPower = true;
volatile uint8_t ledBrightness = 64;
volatile uint8_t ledMode = 0;        
volatile uint16_t ledSpeed = 30; 

CRGB color1 = CRGB::Red;
CRGB color2 = CRGB::Green;
CRGB color3 = CRGB::Blue;

SemaphoreHandle_t dataMutex;

// --- Kod rozbudowanej strony HTML ---
const char HTML_INDEX[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 LED Controller Pro</title>
    <style>
        body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: #0f0f15; color: #e1e1e6; text-align: center; padding: 10px; margin: 0; }
        .container { max-width: 450px; margin: 20px auto; background: #161622; padding: 25px; border-radius: 20px; box-shadow: 0 8px 24px rgba(0,0,0,0.6); }
        h1 { color: #00adb5; font-size: 26px; margin-bottom: 25px; letter-spacing: 1px; }
        .section { margin: 18px 0; padding: 15px; background: #1f1f30; border-radius: 12px; text-align: left; }
        label { display: block; margin-bottom: 10px; font-weight: 600; font-size: 14px; color: #9effff; }
        .flex-row { display: flex; justify-content: space-between; align-items: center; gap: 10px; }
        input[type="range"] { flex-grow: 1; height: 6px; border-radius: 5px; accent-color: #00adb5; background: #2d2d44; -webkit-appearance: none; }
        .color-pickers { display: flex; justify-content: space-around; gap: 15px; }
        .color-box { text-align: center; }
        .color-box span { display: block; font-size: 12px; margin-bottom: 5px; color: #aaa; }
        input[type="color"] { width: 65px; height: 45px; border: 2px solid #2d2d44; border-radius: 8px; cursor: pointer; background: none; }
        select { width: 100%; padding: 12px; background: #2d2d44; color: white; border: 1px solid #3d3d5c; border-radius: 8px; font-size: 15px; outline: none; }
        .btn { background: #00adb5; color: white; border: none; padding: 14px; font-size: 16px; border-radius: 8px; cursor: pointer; width: 100%; font-weight: bold; transition: 0.2s; box-shadow: 0 4px 12px rgba(0,173,181,0.3); }
        .btn-off { background: #ff2e63; box-shadow: 0 4px 12px rgba(255,46,99,0.3); }
        .footer-link { display: inline-block; margin-top: 20px; color: #555577; text-decoration: none; font-size: 13px; transition: 0.2s; }
        .footer-link:hover { color: #00adb5; }
    </style>
</head>
<body>
    <div class="container">
        <h1>LED Controller Pro</h1>
        
        <div class="section">
            <button id="powerBtn" class="btn" onclick="togglePower()">Wyłącz System</button>
        </div>

        <div class="section">
            <label>Główna Jasność</label>
            <input type="range" id="brightness" min="0" max="255" value="64" oninput="sendUpdate()">
        </div>

        <div class="section">
            <label>Prędkość Efektów (Mniejsze opóźnienie = szybciej)</label>
            <input type="range" id="speed" min="5" max="150" value="30" oninput="sendUpdate()">
        </div>

        <div class="section">
            <label>Paleta Kolorów (Wybierz do 3 kolorów)</label>
            <div class="color-pickers">
                <div class="color-box"><span>Kolor 1</span><input type="color" id="c1" value="#ff0000" onchange="sendUpdate()"></div>
                <div class="color-box"><span>Kolor 2</span><input type="color" id="c2" value="#00ff00" onchange="sendUpdate()"></div>
                <div class="color-box"><span>Kolor 3</span><input type="color" id="c3" value="#0000ff" onchange="sendUpdate()"></div>
            </div>
        </div>

        <div class="section">
            <label>Wybór Efektu / Animacji</label>
            <select id="mode" onchange="sendUpdate()">
                <option value="0">Stały Kolor (Tylko Kolor 1)</option>
                <option value="1">Płynna Tęcza (Rainbow)</option>
                <option value="2">Stroboskop (Mikstura 3 kolorów)</option>
                <option value="3">Wzór Policyjny (Kolor 1 + Kolor 3)</option>
                <option value="4">Oddychanie (Breathe - Kolor 1)</option>
                <option value="5">Wędrujący Punkt (Cylon - Kolor 1)</option>
                <option value="6">Gradient Statyczny (Przejście C1 -> C2 -> C3)</option>
                <option value="7">Magiczne Gwiazdy (Sparkle - Mix Kolorów)</option>
            </select>
        </div>
        
        <a class="footer-link" href="/update" target="_blank">Panel Aktualizacji OTA</a>
    </div>

    <script>
        let powerState = true;

        function togglePower() {
            powerState = !powerState;
            const btn = document.getElementById('powerBtn');
            if(powerState) {
                btn.innerText = "Wyłącz System";
                btn.classList.remove('btn-off');
            } else {
                btn.innerText = "Włącz System";
                btn.classList.add('btn-off');
            }
            sendUpdate();
        }

        function sendUpdate() {
            const brightness = document.getElementById('brightness').value;
            const speed = document.getElementById('speed').value;
            const mode = document.getElementById('mode').value;
            const power = powerState ? 1 : 0;
            
            const c1 = document.getElementById('c1').value.replace('#', '');
            const c2 = document.getElementById('c2').value.replace('#', '');
            const c3 = document.getElementById('c3').value.replace('#', '');

            const url = `/set?p=${power}&b=${brightness}&s=${speed}&m=${mode}&c1=${c1}&c2=${c2}&c3=${c3}`;
            fetch(url).then(response => console.log("Zsynchronizowano"));
        }
    </script>
</body>
</html>
)rawliteral";

CRGB hexToCRGB(String hexStr) {
  long hexColor = strtol(hexStr.c_str(), NULL, 16);
  CRGB color;
  color.r = (hexColor >> 16) & 0xFF;
  color.g = (hexColor >> 8) & 0xFF;
  color.b = hexColor & 0xFF;
  return color;
}

void handleSet() {
  xSemaphoreTake(dataMutex, portMAX_DELAY);
  
  if (server.hasArg("p"))  ledPower = server.arg("p").toInt();
  if (server.hasArg("b")) {
    ledBrightness = server.arg("b").toInt();
    FastLED.setBrightness(ledBrightness);
  }
  if (server.hasArg("s"))  ledSpeed = server.arg("s").toInt();
  if (server.hasArg("m"))  ledMode = server.arg("m").toInt();
  
  if (server.hasArg("c1")) color1 = hexToCRGB(server.arg("c1"));
  if (server.hasArg("c2")) color2 = hexToCRGB(server.arg("c2"));
  if (server.hasArg("c3")) color3 = hexToCRGB(server.arg("c3"));
  
  xSemaphoreGive(dataMutex);
  server.send(200, "text/plain", "OK");
}

void LedTask(void * pvParameters) {
  CRGB localC1, localC2, localC3;
  uint8_t localMode;
  uint16_t localSpeed;

  uint8_t breatheIndex = 0;
  bool breatheUp = true;
  int cylonPos = 0;
  bool cylonDir = true;

  for(;;) {
    if (!ledPower) {
      FastLED.clear();
      FastLED.show();
      vTaskDelay(pdMS_TO_TICKS(50));
      continue;
    }

    xSemaphoreTake(dataMutex, portMAX_DELAY);
    localC1 = color1; localC2 = color2; localC3 = color3;
    localMode = ledMode; localSpeed = ledSpeed;
    xSemaphoreGive(dataMutex);

    switch (localMode) {
      case 0: // Stały kolor (C1)
        fill_solid(leds, NUM_LEDS, localC1);
        FastLED.show();
        vTaskDelay(pdMS_TO_TICKS(40));
        break;

      case 1: // Tęcza
        static uint8_t gHue = 0;
        gHue++;
        fill_rainbow(leds, NUM_LEDS, gHue, 7);
        FastLED.show();
        vTaskDelay(pdMS_TO_TICKS(localSpeed / 2 + 5));
        break;

      case 2: // Stroboskop z 3 kolorów
        static uint8_t strobeStep = 0;
        if (strobeStep == 0) fill_solid(leds, NUM_LEDS, localC1);
        else if (strobeStep == 1) fill_solid(leds, NUM_LEDS, localC2);
        else if (strobeStep == 2) fill_solid(leds, NUM_LEDS, localC3);
        else FastLED.clear();
        
        FastLED.show();
        strobeStep = (strobeStep + 1) % 4;
        vTaskDelay(pdMS_TO_TICKS(localSpeed + 10));
        break;

      case 3: // Policja (C1 i C3)
        static bool flip = false;
        flip = !flip;
        for (int i = 0; i < NUM_LEDS; i++) {
          if (i < NUM_LEDS / 2) leds[i] = flip ? localC1 : CRGB::Black;
          else leds[i] = flip ? CRGB::Black : localC3;
        }
        FastLED.show();
        vTaskDelay(pdMS_TO_TICKS(localSpeed * 3 + 10));
        break;

      case 4: // Oddychanie (Breathe)
        if (breatheUp) {
          breatheIndex++;
          if (breatheIndex >= 255) breatheUp = false;
        } else {
          breatheIndex--;
          if (breatheIndex <= 5) breatheUp = true;
        }
        fill_solid(leds, NUM_LEDS, localC1);
        FastLED.setBrightness(map(breatheIndex, 0, 255, 0, ledBrightness));
        FastLED.show();
        vTaskDelay(pdMS_TO_TICKS(localSpeed / 4 + 2));
        break;

      case 5: // Wędrujący Punkt (Cylon / KITT)
        fadeToBlackBy(leds, NUM_LEDS, 40);
        leds[cylonPos] = localC1;
        
        if (cylonDir) {
          cylonPos++;
          if (cylonPos >= NUM_LEDS - 1) cylonDir = false;
        } else {
          cylonPos--;
          if (cylonPos <= 0) cylonDir = true;
        }
        FastLED.show();
        vTaskDelay(pdMS_TO_TICKS(localSpeed / 3 + 5));
        break;

      case 6: // Poprawiony Gradient (C1 -> C2 -> C3)
        fill_gradient_RGB(leds, 0, localC1, (NUM_LEDS / 2), localC2);
        fill_gradient_RGB(leds, (NUM_LEDS / 2), localC2, (NUM_LEDS - 1), localC3);
        FastLED.show();
        vTaskDelay(pdMS_TO_TICKS(100));
        break;

      case 7: // Magiczne Gwiazdy (Sparkle)
        fadeToBlackBy(leds, NUM_LEDS, 20);
        if (random8() < 60) {
          int p = random16(NUM_LEDS);
          uint8_t r = random8(3);
          leds[p] = (r == 0) ? localC1 : ((r == 1) ? localC2 : localC3);
        }
        FastLED.show();
        vTaskDelay(pdMS_TO_TICKS(localSpeed / 2 + 5));
        break;
    }
    
    if (localMode != 4) {
      FastLED.setBrightness(ledBrightness);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dataMutex = xSemaphoreCreateMutex();

  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(ledBrightness);
  FastLED.clear();
  FastLED.show();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); }

  server.on("/", []() { server.send(200, "text/html", HTML_INDEX); });
  server.on("/set", handleSet);

  ElegantOTA.begin(&server);
  ElegantOTA.onStart([]() {
    vTaskDelete(NULL); 
    FastLED.clear();
    FastLED.show();
  });

  server.begin();

  xTaskCreatePinnedToCore(LedTask, "LedTask", 4000, NULL, 1, NULL, 0);
}

void loop() {
  server.handleClient();
  ElegantOTA.loop();
  vTaskDelay(pdMS_TO_TICKS(2));
}
