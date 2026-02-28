#pragma once
#include <Arduino.h>

struct State {
  float soc;        // %
  float vpack;      // V
  float current;    // A (+ = charging)
  float tempC;      // C
  bool charging;
  uint16_t alarms;
  uint32_t lastGoodMs; // millis() when last valid update arrived
};

// Utility
inline void stateSetDefaults(State& st) {
  // Sensible defaults so UI doesn't start completely empty
  st.soc = 82.0f;
  st.vpack = 11.7f;
  st.current = -1.1f;
  st.tempC = 23.5f;
  st.charging = false;
  st.alarms = 0;
  st.lastGoodMs = 0;
}