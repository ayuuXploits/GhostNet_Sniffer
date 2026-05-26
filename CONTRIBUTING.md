# Contributing to GhostNet Sniffer

Thank you for your interest in contributing to **GhostNet Sniffer** — an ESP32-based passive WiFi device tracker and radar visualizer. Whether you're fixing a bug, improving the radar display, or adding new detection features, your contribution is welcome.

---

## Table of contents

- [Code of conduct](#code-of-conduct)
- [Getting started](#getting-started)
- [How to contribute](#how-to-contribute)
  - [Reporting bugs](#reporting-bugs)
  - [Suggesting features](#suggesting-features)
  - [Submitting a pull request](#submitting-a-pull-request)
- [Development setup](#development-setup)
- [Coding standards](#coding-standards)
- [Commit message format](#commit-message-format)
- [Areas that need help](#areas-that-need-help)
- [License](#license)

---

## Code of conduct

This project is a learning-focused, open hardware/software effort. Be respectful, constructive, and patient — especially with beginners. Harassment or hostile behaviour of any kind will not be tolerated.

---

## Getting started

1. **Fork** the repository on GitHub.
2. **Clone** your fork locally:
   ```bash
   git clone https://github.com/ayuuXploits/ghostnet-sniffer.git
   cd ghostnet-sniffer
   ```
3. Create a new branch for your work:
   ```bash
   git checkout -b fix/radar-angle-mismatch
   ```
4. Make your changes, then push and open a Pull Request.

---


---

## How to contribute

### Reporting bugs

Open a [GitHub Issue](../../issues/new) and include:

- A clear title (e.g., `Bug: device angle offset by 90° on radar`)
- Steps to reproduce
- Expected vs. actual behaviour
- ESP32 firmware version and board variant
- Any relevant serial monitor output or screenshots

Use the labels `bug`, `radar`, `firmware`, or `dashboard` as appropriate.

### Suggesting features

Open an issue with the label `enhancement`. Describe:

- The problem you want to solve
- Your proposed solution or approach
- Any hardware or software constraints to be aware of

### Submitting a pull request

1. Make sure your branch is up to date with `main`:
   ```bash
   git fetch origin
   git rebase origin/main
   ```
2. Test your changes on real hardware if the PR touches firmware.
3. For dashboard changes, verify the radar renders correctly in both Chrome and Firefox.
4. Open a Pull Request with:
   - A descriptive title
   - A summary of what changed and why
   - Reference to any related issue (e.g., `Closes #12`)

PRs without a description or that cannot be tested will be held until clarified.

---

## Development setup

### Firmware (ESP32)

| Requirement | Version |
|---|---|
| Arduino IDE | 2.x or later |
| ESP32 board package | 2.x (by Espressif) |
| Arduino libraries | `WiFi.h` (built-in), `ArduinoJson` ≥ 6 |

1. Open `firmware/ghostnet_sniffer/ghostnet_sniffer.ino` in Arduino IDE.
2. Select your board: **ESP32 Dev Module** (or your specific variant).
3. Set baud rate to `115200` in Serial Monitor.
4. Upload and verify output in Serial Monitor before testing the dashboard.

### Dashboard

No build step required. Open `dashboard/index.html` directly in a browser, or serve it locally:

```bash
cd dashboard
python3 -m http.server 8080

```

Then open `http://localhost:8080` and connect via WebSocket or serial bridge.

---

## Coding standards

### Firmware (C/C++)

- Use `camelCase` for variables and functions, `UPPER_SNAKE_CASE` for constants and `#define` macros.
- Keep scan logic and angle calculation in separate files — do not mix rendering concerns into firmware.
- Always normalise angles before transmitting:
  ```cpp
  float normaliseAngle(float angle) {
    return fmod(fmod(angle, 360.0f) + 360.0f, 360.0f);
  }
  ```
- Avoid `delay()` in the main loop; use non-blocking timing with `millis()`.
- Comment any RSSI heuristic with the assumption it relies on (e.g., free-space path loss model).

### Dashboard (JavaScript)

- Use `const` and `let`; no `var`.
- Angles going into the canvas renderer must be in **radians**, converted from the normalised degree value received from firmware.
- The radar coordinate system: `0°` = 12 o'clock, clockwise. Convert with:
  ```js
  const toCanvasAngle = (deg) => (deg - 90) * (Math.PI / 180);
  ```
- Track devices by **MAC address**, not by scan-list index, to avoid dot-swapping between refreshes.
- No external dependencies unless discussed and agreed in an issue first.

---

## Commit message format

Use short, imperative commit messages:

```
fix: normalise radar angle before canvas render
feat: add MAC-based device tracking
docs: add calibration guide to docs/
refactor: split angle_calc into its own module
```

Prefix options: `fix`, `feat`, `docs`, `refactor`, `test`, `chore`.

---

## Areas that need help

If you're not sure where to start, these are open areas:

- **Angle accuracy** — improve RSSI-to-angle estimation; explore triangulation with multiple ESP32 nodes.
- **Radar UI** — smooth animation between device position updates, add signal strength ring indicators.
- **Calibration tool** — a guided single-device calibration mode to establish the 0° reference.
- **Multi-node support** — coordinate data from 2+ ESP32 scanners for better positioning.
- **Docs** — wiring diagrams, setup photos, and a beginner-friendly quickstart guide.

Check the [Issues](../../issues) tab for anything tagged `good first issue` or `help wanted`.

---

## License

By contributing, you agree that your contributions will be licensed under the same license as this project. See `LICENSE` for details.

---

*GhostNet Sniffer — built on curiosity, runs on packets.*
