#include "DataProcessing.h"
#include <Arduino.h>
#include <HardwareSerial.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

// UART line buffer
static char lineBuf[192];
static size_t lineLen = 0;

// ---------------- CRC-8 Dallas/Maxim ----------------
static uint8_t crc8_dallas(const uint8_t* data, size_t len) {
  uint8_t crc = 0x00;
  while (len--) {
    uint8_t inbyte = *data++;
    for (uint8_t i = 0; i < 8; i++) {
      uint8_t mix = (crc ^ inbyte) & 0x01;
      crc >>= 1;
      if (mix) crc ^= 0x8C; // reflected poly 0x31
      inbyte >>= 1;
    }
  }
  return crc;
}

static int hexNibble(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
  if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
  return -1;
}

static bool parseHexByte(const char* p, uint8_t& out) {
  int hi = hexNibble(p[0]);
  int lo = hexNibble(p[1]);
  if (hi < 0 || lo < 0) return false;
  out = (uint8_t)((hi << 4) | lo);
  return true;
}

static void trimSpaces(char* s) {
  // leading
  char* p = s;
  while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
  if (p != s) {
    char* d = s;
    while (*p) *d++ = *p++;
    *d = 0;
  }
  // trailing
  int n = (int)strlen(s);
  while (n > 0 && (s[n-1] == ' ' || s[n-1] == '\t' || s[n-1] == '\r' || s[n-1] == '\n')) {
    s[n-1] = 0;
    n--;
  }
}

static bool parseKeyValueCSV(const char* payload, State& out) {
  int soc = -1;
  long vpack_mV = LONG_MIN;
  long i_mA = LONG_MIN;
  long t_deciC = LONG_MIN;
  long al = LONG_MIN;
  long chg = LONG_MIN;

  char tmp[192];
  strncpy(tmp, payload, sizeof(tmp) - 1);
  tmp[sizeof(tmp) - 1] = 0;

  char* saveptr = NULL;
  char* tok = strtok_r(tmp, ",", &saveptr);
  while (tok) {
    trimSpaces(tok);
    char* eq = strchr(tok, '=');
    if (eq) {
      *eq = 0;
      char* k = tok;
      char* v = eq + 1;
      trimSpaces(k);
      trimSpaces(v);

      if (strcmp(k, "SOC") == 0) soc = atoi(v);
      else if (strcmp(k, "Vpack") == 0) vpack_mV = atol(v);
      else if (strcmp(k, "I") == 0) i_mA = atol(v);
      else if (strcmp(k, "T") == 0) t_deciC = atol(v);
      else if (strcmp(k, "AL") == 0) al = atol(v);
      else if (strcmp(k, "CHG") == 0) chg = atol(v);
    }
    tok = strtok_r(NULL, ",", &saveptr);
  }

  if (soc < 0 || soc > 100) return false;
  if (vpack_mV == LONG_MIN) return false;
  if (i_mA == LONG_MIN) return false;

  out.soc = (float)soc;
  out.vpack = (float)vpack_mV / 1000.0f;
  out.current = (float)i_mA / 1000.0f;
  if (t_deciC != LONG_MIN) out.tempC = (float)t_deciC / 10.0f;
  if (al != LONG_MIN) out.alarms = (uint16_t)al;

  if (chg != LONG_MIN) out.charging = (chg != 0);
  else out.charging = (out.current > 0.02f); // deadband

  out.lastGoodMs = (uint32_t)millis();
  return true;
}

static bool acceptLineIfValid(const char* line, State& st) {
  const char* star = strchr(line, '*');

  char payload[192];
  payload[0] = 0;

  bool hasCrc = false;
  uint8_t gotCrc = 0;

  if (star) {
    size_t n = (size_t)(star - line);
    if (n >= sizeof(payload)) return false;
    memcpy(payload, line, n);
    payload[n] = 0;

    // Expect 2 hex chars after '*'
    if (strlen(star) >= 3) {
      hasCrc = parseHexByte(star + 1, gotCrc);
    }
  } else {
    strncpy(payload, line, sizeof(payload) - 1);
    payload[sizeof(payload) - 1] = 0;
  }

  trimSpaces(payload);
  if (payload[0] == 0) return false;

  if (hasCrc) {
    uint8_t calc = crc8_dallas((const uint8_t*)payload, strlen(payload));
    if (calc != gotCrc) return false;
  }

  State candidate = st; // start from current
  if (!parseKeyValueCSV(payload, candidate)) return false;

  st = candidate;
  return true;
}

// ---------------- DEMO MOCK ----------------
static unsigned long lastMockUpdateMs = 0;

static void updateMock(State& st) {
  unsigned long now = millis();
  if (now - lastMockUpdateMs < 200) return;
  lastMockUpdateMs = now;

  float t = now / 1000.0f;

  st.soc -= 0.02f;
  if (st.soc < 5.0f) st.soc = 100.0f;

  st.current = -1.2f + 0.2f * sinf(t * 0.7f);

  st.charging = (fmodf(t, 60.0f) > 45.0f);
  if (st.charging) st.current = 1.0f + 0.3f * sinf(t * 0.9f);

  float ocv = 9.6f + (st.soc / 100.0f) * (12.6f - 9.6f);
  float irDrop = 0.08f * (st.current);
  st.vpack = ocv + irDrop;

  st.tempC = 23.5f + 2.0f * sinf(t * 0.1f);

  st.alarms = 0;
  if (st.vpack < 10.0f) st.alarms |= 0x0001;
  if (st.vpack > 12.7f) st.alarms |= 0x0002;

  st.lastGoodMs = (uint32_t)millis();
}


// ---------------- UART polling ----------------
static void pollSerial(State& st) {
  while (Serial2.available() > 0) {
    int c = Serial2.read();
    if (c < 0) break;

    if (c == '\n') {
      lineBuf[lineLen] = 0;
      acceptLineIfValid(lineBuf, st);
      lineLen = 0;
    } else if (c != '\r') {
      if (lineLen < sizeof(lineBuf) - 1) lineBuf[lineLen++] = (char)c;
      else lineLen = 0; // overflow -> reset
    }
  }
}

void dataInit() {
#if DEMO_MODE == 0
  Serial2.begin(UART_BAUD, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
#endif
}

void dataTick(State& st) {
#if DEMO_MODE == 1
  updateMock(st);
#else
  pollSerial(st);
#endif
}
