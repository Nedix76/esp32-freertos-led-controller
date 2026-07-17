
# ESP32 FreeRTOS LED Controller Pro

<img width="1708" height="470" alt="WhatsApp Image 2026-07-17 at 23 01 11" src="https://github.com/user-attachments/assets/ffba57a4-d89f-487a-9337-2561116569d2" />
<img width="516" height="745" alt="Zrzut ekranu 2026-07-17 225336" src="https://github.com/user-attachments/assets/a7bdb20b-e0f0-4e29-9073-2bfb652205dd" />


Zaawansowany sterownik adresowalnych taśm LED (WS2812B) oparty na mikrokontrolerze ESP32. Projekt wykorzystuje system operacyjny czasu rzeczywistego **FreeRTOS** do wielozadaniowości, zapewniając płynne generowanie animacji niezależnie od obciążenia serwera sieciowego.

## 🚀 Główne Funkcje

*   **Architektura wielowątkowa (FreeRTOS):** Wydzielone zadanie (`LedTask`) przypisane do dedykowanego rdzenia procesora odpowiada wyłącznie za renderowanie efektów LED. Zapobiega to jakimkolwiek szarpnięciom (lagom) animacji podczas obsługi żądań HTTP.
*   **Interfejs Web (Responsive UI):** Nowoczesny, ciemny panel kontrolny (HTML5/CSS3/JavaScript Async Fetch API) pozwalający na sterowanie parametrami w czasie rzeczywistym z telefonu lub komputera.
*   **Bezpieczna synchronizacja wątków:** Wykorzystanie semaforów binarnych (`Mutex`) do bezpiecznej wymiany danych konfiguracyjnych pomiędzy serwerem HTTP (wątek główny) a pętlą animacji.
*   **Aktualizacje Over-The-Air (OTA):** Integracja z biblioteką `ElegantOTA` umożliwia bezprzewodową aktualizację oprogramowania układowego bezpośrednio przez przeglądarkę, bez konieczności podłączania kabla USB.
*   **8 dynamicznych trybów:** W tym stały kolor, płynna tęcza, zaawansowany stroboskop trójkolorowy, efekt policyjny, płynne oddychanie (breathe), efekt Cylon (KITT), statyczny gradient oraz efekt "magicznych gwiazd" (Sparkle).

## 🛠️ Architektura i Technologie

*   **Hardware:** ESP32 (NodeMCU / DevKit) + Taśma LED WS2812B (150 diod)
*   **Framework:** Arduino IDE / PlatformIO
*   **Kluczowe Biblioteki:**
    *   `FastLED` (zaawansowane, wydajne sterowanie diodami)
    *   `ElegantOTA` (zarządzanie aktualizacjami bezprzewodowymi)
    *   `WebServer` (obsługa REST API do sterowania parametrami)
*   **Zarządzanie pamięcią i wątkami:** `xTaskCreatePinnedToCore`, `SemaphoreHandle_t` (`xSemaphoreTake` / `xSemaphoreGive`).

## 🔧 Jak uruchomić projekt

1. Sklonuj to repozytorium na swój dysk.
2. Otwórz plik `.ino` w Arduino IDE lub PlatformIO.
3. Zainstaluj wymagane biblioteki: `FastLED` oraz `ElegantOTA`.
4. W sekcji konfiguracji Wi-Fi wprowadź dane swojej sieci:
   ```cpp
   const char* ssid     = "TWOJA_NAZWA_SIECI";
   const char* password = "TWOJE_HASLO";
5. Wgraj program na płytkę ESP32.
6. Otwórz Monitor Szeregowy (baudrate `115200`), aby odczytać adres IP przypisany do urządzenia.
7. Wpisz adres IP w przeglądarce, aby otworzyć panel sterowania. Panel OTA dostępny jest pod adresem `http://<IP_ESP32>/update`.
