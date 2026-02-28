#include "WifiScan.h"
#include <WiFi.h>

// ------------------- Find less busy channels -------------------
static int clampChannel(int ch) {
  if (ch < 1) return 1;
  if (ch > 13) return 13;
  return ch;
}

int pickBestChannel_1_6_11_withLogging() {
  Serial.println();
  Serial.println("=== WIFI SCAN START ===");

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(200);

  int n = WiFi.scanNetworks(false, true);
  if (n < 0) {
    Serial.println("Scan failed, fallback to channel 1");
    return 1;
  }

  Serial.print("Networks found: ");
  Serial.println(n);
  Serial.println("----------------------------------------");
  Serial.println("SSID                     CH   RSSI");

  long score[14];
  for (int i=0;i<14;i++) score[i]=0;

  for (int i = 0; i < n; i++) {
    String ssid = WiFi.SSID(i);
    int ch = clampChannel(WiFi.channel(i));
    int rssi = WiFi.RSSI(i);

    int w = 100 + rssi; // rssi negative
    if (w < 1) w = 1;
    if (w > 100) w = 100;

    score[ch] += w;

    Serial.printf("%-24s %2d   %4d dBm  (w=%d)\n", ssid.c_str(), ch, rssi, w);
  }

  Serial.println("----------------------------------------");
  Serial.println("Channel scores (lower is better):");
  int candidates[3] = {1, 6, 11};

  int best = candidates[0];
  long bestScore = score[best];

  for (int k=0;k<3;k++) {
    int ch = candidates[k];
    Serial.printf("  CH %2d  -> score %ld\n", ch, score[ch]);
    if (score[ch] < bestScore) { bestScore = score[ch]; best = ch; }
  }

  Serial.println("----------------------------------------");
  Serial.print("Selected channel: ");
  Serial.println(best);
  Serial.println("=== WIFI SCAN END ===");
  Serial.println();

  WiFi.scanDelete();
  return best;
}