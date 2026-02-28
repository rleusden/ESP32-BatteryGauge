# ESP32 BatteryGauge (Legacy Tablet Edition)
![ESP32](https://img.shields.io/badge/ESP32-AP--Mode-blue)
![Android 4.x Compatible](https://img.shields.io/badge/Android-4.x%20Compatible-green)
![Legacy Hardware Friendly](https://img.shields.io/badge/Legacy%20Hardware-Yes-success)

Give discarded hardware a second life ⚡📟

This project turns an ESP32 into a standalone Wi-Fi access point that hosts a super-simple battery dashboard, compatible with **very old Android browsers** (tested on Android 4.1.1).  
The electronics can live in a waterproof box near the battery, while the display (an old tablet) stays safe and dry.

The web UI polls `/json` and shows:
- SOC with a bar (**green when charging, red when discharging**)
- Pack voltage, current, temperature
- Status (Charging / Discharging)
- Stale / No-data handling (after 60s -> blanks to `--`)

---

## ✨ Features

- **Android 4.1.1 friendly UI**
  - No `fetch()`, no CSS grid/vars, no modern JS tricks
  - Uses `XMLHttpRequest` + table layout (no scrolling)

- **AP mode dashboard**
  - ESP32 runs its own Wi-Fi network
  - Tablet connects directly (no router needed)

- **Auto Wi-Fi channel selection**
  - Optional scan at boot
  - Picks best channel from **1 / 6 / 11**
  - Logs SSID / channel / RSSI and selected channel to Serial

- **Demo vs Live mode**
  - Compile-time toggle:
    - **DEMO**: generates mock battery data
    - **LIVE**: reads real values via **UART** (`Serial2`)

- **Marine-grade behaviour**
  - Watchdog enabled
  - `NO DATA` handling and UI blanking after 60 seconds
  - Designed for waterproof enclosures + remote display

---

## Clean structure

- `ESP32_BatteryGauge.ino` – main sketch (setup/loop glue)
- `html.*` – the single-page dashboard (old Android compatible)
- `WifiScan.*` – scan + choose best channel (with Serial logging)
- `DataProcessing.*` – DEMO generator + UART parsing + CRC helpers
- `WebHandlers.*` – `/`, `/json`, `/ping`
- `State.h` – battery state struct

---

## Hardware idea

Typical setup:

- **Battery box (waterproof)**
  - Sensor electronics (e.g. Pro Mini + LTC2944)  
  - ESP32 (AP + web dashboard)
  - Optional external antenna for range

- **Display**
  - Old Android tablet (connects to ESP32 AP)
  - Browser opens `http://192.168.4.1`

---

## Quick Start

1. Flash the ESP32
2. Power the ESP32
3. Connect the tablet to Wi-Fi SSID: `BatteryGauge`
4. Open:

   `http://192.168.4.1`

You should see the dashboard updating in real-time.

---

🖥 Console Output Example

Below is an example of the ESP32 boot log with auto channel scan enabled.

Boot...

=== WIFI SCAN START ===

Networks found: 11

| SSID              | Channel | RSSI (dBm) | Weight |
| ----------------- | ------- | ---------- | ------ |
| Network_01        | 1       | -92        | 8      |
| Hidden_Network_01 | 8       | -92        | 8      |
| Hidden_Network_02 | 8       | -93        | 7      |
| Network_02        | 8       | -93        | 7      |
| Hidden_Network_03 | 8       | -93        | 7      |
| Hidden_Network_04 | 1       | -94        | 6      |
| Network_02        | 8       | -94        | 6      |
| Network_03        | 13      | -94        | 6      |
| Hidden_Network_05 | 13      | -95        | 5      |
| Network_04        | 11      | -96        | 4      |
| Hidden_Network_06 | 13      | -96        | 4      |

Channel scores (lower is better):

| Parameter        | Value        |
| ---------------- | ------------ |
| Selected Channel | 6            |
| AP SSID          | BatteryGauge |
| AP IP            | 192.168.4.1  |
| Mode             | DEMO_MODE    |

=== WIFI SCAN END ===

Explanation:

The ESP32 scans nearby 2.4 GHz networks at boot.
Only non-overlapping channels 1 / 6 / 11 are considered.
A weighted score is calculated based on RSSI.
(Weighted Score = Σ (100 + RSSI_dBm))
The channel with the lowest congestion score is selected.
The ESP32 then starts its own Access Point on that channel.

---

## LIVE mode (UART input)

In LIVE mode the ESP32 expects key/value CSV lines (newline-terminated), for example:

SOC=72,Vpack=11480,I=-1230,T=235,CHG=0,AL=0*5A

Units:
- `Vpack` = **mV**
- `I` = **mA** (positive = charging)
- `T` = **deci-°C** (235 => 23.5°C)
- `CHG` optional (if missing, charging is inferred from current)
- `*HH` optional CRC-8 Dallas/Maxim over payload before `*`

No CRC is also accepted:

SOC=72,Vpack=11480,I=-1230,T=235,AL=0


---

## ⚠️ Safety Notes

- If you connect this to real packs: use a proper **BMS** for protection.
- ESP32 is a UI + telemetry device, not a safety system.
- In sealed enclosures: watch out for **condensation** and ensure proper strain relief.

---

## What can you build?

- Battery gauge for a 3S Li-ion pack
- Power bank display (tablet UI)
- Remote “dry” display for a waterproof battery box
- SOC dashboard for LTC2944 / INA219 / shunt-based systems

---

## Contributing

Contributions are welcome:
- UART protocol variants
- Better channel scoring
- UI tweaks for even older browsers
- External antenna / enclosure notes

---

## Why this project?

Old tablets often get discarded because the Play Store/browser is too outdated.
This project turns them into a useful remote display again, reducing e-waste and keeping fragile screens out of harsh environments.

Happy hacking ✨
