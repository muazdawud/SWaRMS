# SWaRMS Firmware & Dashboard

Smart Ward-level Refuse Management Station — ESP32 firmware paired with a
web dashboard for monitoring bin fill level, fan state, and environment.

---

## Overview

| Layer | Technology |
|---|---|
| MCU | ESP32 |
| Firmware | Arduino (C++) |
| Filesystem | LittleFS |
| Web server | `WebServer` on port 80 |
| Realtime | `WebSocketsServer` on port 81 |
| Discovery | mDNS (`SWaRMS.local`) |
| Time | NTP over UDP |
| Sensing | HX711 load cell |
| Actuation | 3× 28BYJ-48 steppers, 1 fan, 3 status LEDs |

---

## Hardware Pinout

| Function | GPIO | Notes |
|---|---|---|
| Load cell SCK | 22 | HX711 |
| Load cell DATA | 23 | HX711 |
| Fan | 12 | Strapping pin — keep LOW at boot |
| Red LED | 25 | Lock / startup |
| Blue LED | 33 | Stepper activity |
| Yellow LED | 32 | Weight threshold |
| CP button | 35 | Input-only, needs external pull-up |
| EG button | 34 | Input-only, needs external pull-up |
| IG button | 21 | |

| Stepper | Pins |
|---|---|
| CP (Compaction Plate) | 15, 2, 4, 16 |
| EG (Extraction Gate) | 17, 5, 18, 19 |
| IG (Intake gate) | 13, 14, 27, 26 |

---

## Project Structure

```
data/
  index.html        # login page
  homepage.html     # dashboard
  notFound.html     # 404 page
  favicon.png       # site icon
src/
  SWaRMS_static.ino # firmware
README.md
License.txt
CHANGELOG.md
.gitignore
```

---

## Getting Started

### 1. Configure credentials

In `SWaRMS_static.ino`:

```cpp
const char *ssid     = "YOUR_SSID";
const char *password = "YOUR_PASSWORD";
```

### 2. Upload filesystem

- Arduino IDE: **ESP32 Sketch Data Upload**

### 3. Flash firmware

- Arduino IDE: **Upload**

### 4. Access the dashboard

```
http://<esp32-ip>/
http://SWaRMS/
```

Default login: `admin` / `0000`

---

## Configuration

| Macro | Default | Purpose |
|---|---|---|
| `stepSpeed` | 10 | Stepper RPM |
| `stepPerRevolution` | 2048 | Steps per full rotation |
| `CP_Distance` / `IG_Distance` / `EG_Distance` | 1 | Revolutions per action |
| `callibrationWeight` | 2280 | HX711 calibration factor |
| `W_Threshold` | 30 | Fill threshold (g) |
| `USERNAME` / `PASSWORD` | `admin` / `0000` | Web login |

---

## Runtime Behaviour

- **Startup** — LEDs blink, each stepper cycles one revolution, load cell tares.
- **Buttons** — CP / EG / IG trigger their stepper unless locked.
- **Auto-cycle** — When weight ≥ `W_Threshold` for 4 consecutive checks, the
  station closes and locks.
- **Fan** — Runs while weight ≥ 50 % of `W_Threshold`.
- **NTP** — Syncs periodically; time string is pushed to the dashboard.
- **WebSocket** — Dashboard sends `Refresh`, firmware replies with a
  comma-separated station snapshot.

---

## WebSocket Message Format

**Request**

```
Refresh
```

**Response**

```
Station Info: date,time,stationNumber,location,ambientTemp,ambientHum,
lock,threshold,weight,innerTemp,innerHum,fanFlag
```

---

## Known Limitations

- Stepper motion blocks the main loop for the duration of the move.
- Buttons on GPIO 34/35 require external pull-up resistors.
- GPIO 12 must be LOW at boot; add a pull-down if the fan driver floats it.

---

## Troubleshooting

| Symptom | Check |
|---|---|
| Device never boots | GPIO 12 held HIGH by fan driver |
| Buttons always read 0 | Missing pull-ups on GPIO 34/35 |
| Dashboard shows `--` | WebSocket blocked, or client not sending `Refresh` |
| Favicon 404 | `favicon.png` not uploaded to LittleFS |
| Time never syncs | `WiFi.hostByName` failed — check DNS |
| Weight drifts | Re-run `scale.tare()` with empty bin |

---

## License

Copyright © 2026 Dauda Muazu Sulaiman, Ecotronics Automation Concepts. All rights reserved.

Internal project — Bayero University Kano, Rimin Gata.