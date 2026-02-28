/*
  ESP32 AP BatteryGauge (Android 4.1.1 friendly)

  Project layout (no extra files beyond the ones you already have):
   - html.*           : web UI (XHR polling, no modern CSS/JS)
   - WifiScan.*       : optional WiFi scan + channel pick with Serial logging
   - DataProcessing.* : DEMO mock (and later: UART parsing)
   - WebHandlers.*    : /, /json, /ping
   - State.h          : shared state struct

  Compile-time toggles live in DataProcessing.h so every translation unit
  sees the same settings.
*/

#include <WiFi.h>
#include <WebServer.h>
#include "esp_task_wdt.h"

#include "State.h"
#include "DataProcessing.h"   // contains the config macros (DEMO_MODE, AUTO_CHANNEL, etc.)
#include "WifiScan.h"         // Find less busy channels
#include "WebHandlers.h"

// ------------------- AP CONFIG -------------------
// Keep SSID/PASS here (only used in this .ino)
static const char* AP_SSID = "BatteryGauge";
static const char* AP_PASS = "recycle123";

constexpr uint32_t WDT_TIMEOUT_SEC = 10;

#define AUTO_CHANNEL 1   // 1 = scan and pick best channel, 0 = fixed
#define FIXED_CHANNEL 1  // used when AUTO_CHANNEL=0

WebServer server(80);
State st;

static void wdt_setup() {
  // Arduino-ESP32 core 3.x (IDF5): config-struct API
  esp_task_wdt_config_t cfg = {};
  cfg.timeout_ms = WDT_TIMEOUT_SEC * 1000;
  cfg.trigger_panic = true; // reset on timeout

  // Optional: watch idle tasks on both cores (works on your setup).
  cfg.idle_core_mask = (1U << portNUM_PROCESSORS) - 1U;

  esp_task_wdt_init(&cfg);
  esp_task_wdt_add(NULL); // add loopTask
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println();
  Serial.println("Boot...");

  // Defaults so UI shows something even before first update
  stateSetDefaults(st);

  // Optional: scan and pick best channel (logs to Serial)
  int apChannel = FIXED_CHANNEL;
#if AUTO_CHANNEL
  apChannel = pickBestChannel_1_6_11_withLogging();
#endif

  WiFi.mode(WIFI_AP);
  WiFi.setSleep(false);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  WiFi.softAP(AP_SSID, AP_PASS, apChannel, 0, 1);
  delay(100);

  Serial.print("AP SSID: "); Serial.println(AP_SSID);
  Serial.print("AP IP: ");   Serial.println(WiFi.softAPIP());
  Serial.print("AP CH : ");  Serial.println(apChannel);

  wdt_setup();

  // Start data source (DEMO or Serial)
  dataInit();

  // Web routes
  webRegisterRoutes(server, st);
  server.begin();

#if DEMO_MODE
  Serial.println("Mode: DEMO_MODE");
#else
  Serial.println("Mode: SERIAL_MODE");
#endif
}

void loop() {
  esp_task_wdt_reset();
  server.handleClient();

  // Update st from demo/mock or UART
  dataTick(st);
}
