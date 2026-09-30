# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2026-09-30

### Added

- ESP32 firmware for the Smart Ward-level Refuse Management Station (SWaRMS).
- Control of three 28BYJ-48 steppers (CP, EG, IG) via the Arduino `Stepper` library.
- HX711 load cell integration for bin weight measurement.
- Automatic cycle trigger when weight reaches `W_Threshold` for four consecutive checks.
- Fan control based on weight threshold, with status LED indication.
- Manual push-button control for CP, EG, and IG stations.
- NTP time synchronization over UDP with periodic refresh.
- WiFi station mode with mDNS discovery (`SWaRMS.local`).
- HTTP server on port 80 serving LittleFS-hosted pages.
- WebSocket server on port 81 for live dashboard updates.
- Login page, dashboard (`homepage.html`), and 404 page.
- Favicon handler serving `favicon.png` from LittleFS.
- Custom `Stepper`-driven LED activity indication during motion.
- Hardware watchdog via a hardware timer for periodic task scheduling.

### Known Limitations

- Stepper motion blocks the main loop for the duration of the move.
- Buttons on GPIO 34/35 require external pull-up resistors.
- GPIO 12 (fan) must be LOW at boot; a pull-down is recommended.

[1.0.0]: https://github.com/muazdawud/SWaRMS