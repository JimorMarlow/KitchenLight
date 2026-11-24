# KitchenLight — agent instructions

Dimmer for the kitchen work-zone light on an ESP32-C3 Super Mini: one white 2 m LED strip
driven by PWM, a VL53L0X laser distance sensor for mid-air gestures, an EncButton hardware
button, settings persisted with `etl::settings::data`. Firmware is built with PlatformIO +
Arduino framework. Web UI and MQTT are still only designs in `docs/`, not implemented code.

## Canary

Owner of this project: the value of `git config user.name` (currently `JimorMarlow`). Take it
from git yourself, do not ask. Start **every** answer with the owner's name and a comma, then
continue the answer on the same line — so it is visible that this file was read and the rule is
followed:

`JimorMarlow, <the answer>`

Even a one-line answer, an error report or a question starts with it.

## No STL — extend ETL instead

- Do not use STL (`std::vector`, `std::string`, ...) in this project: only `etl::`, no exceptions,
  no RTTI, no unnecessary dynamic allocations. `etl::vector`, `etl::shared_ptr`, `etl::unique_ptr`,
  `etl::clamp` and `etl::led` are already in use — keep to that vocabulary.
- If ETL lacks a common tool that the task needs, do not substitute `std::` and do not leave an
  ad-hoc workaround in the project: implement it here in an `etl_*` file, debug it on the device,
  and when it is finished propose it to the ETL developer for inclusion in the ETL library.

## Language

- This file (`AGENTS.md`) is kept **in English**: every addition or edit here is written in English.
- Commit messages are **in English**, in the style of the recent history
  (`v0.2.1 upd to ETL 1.2.*`, `v0.1.9 upgrade to etl@0.9.20`). Older commits in the history are
  Russian; do not rewrite them.
- `README.md` and everything in `docs/` stay **in Russian**, including the version history entries in
  `README.md`: a new version is described there in Russian, while the commit message for the same
  version is written in English.
- Comments in code stay **in Russian**, as in the existing code. `Serial` debug lines are mostly
  English (`FADE END: brightness = ...`) — match the file you are touching instead of mass-renaming.

## Build and verification

`pio` is not in PATH — call it by full path. On this machine (macOS):

```bash
~/.platformio/penv/bin/pio run -e esp32c3
```

On Windows it is `& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" run -e esp32c3`.

- Before saying "done", build the project with this command and show the result (RAM/Flash, errors).
- Read compilation errors from the whole output: the first error is the cause, the rest are usually
  its consequences.
- The `pre:sync_version.py` extra script copies `src/version.h` into `package.json` on every build,
  so `package.json` changing after a build is expected.
- Uploading and monitoring need a connected board — **ask the user first**:

```bash
~/.platformio/penv/bin/pio run -e esp32c3 -t upload
~/.platformio/penv/bin/pio device monitor -b 115200
```

- Full clean: `... pio run -t clean`. Do not wipe `.pio` without a reason — rebuilds get slower.
- There are no unit tests (`test/` is an empty PlatformIO placeholder); the correctness criterion is a
  successful build plus reading the code.
- The build writes to the global `~/.platformio` store. Under a restricted file sandbox it fails with
  `PermissionError: ... ~/.platformio/platforms.lock` — that is an environment restriction, report it
  as such and do not try to "fix" the project for it.

## What must not break

- `src/pinout.h` — the actual pin assignment: `LED_PIN` 6 (GPIO6/MOSI, the C3 Super Mini does not
  break out the Arduino default), `BTN_PIN` 7, `LED_BUILTIN` forcibly redefined to 8 because the
  framework's built-in definition makes the board panic. Do not change it without explicit user
  confirmation.
- PWM setup: channel 1, 30 kHz, 10-bit resolution. 30 kHz keeps the strip from whining audibly; do not
  lower the frequency "for more resolution" without asking.
- `SERIAL_INIT_DELAY` (1000 ms in `setup()`) is a deliberate delay so the first debug lines appear on
  the C3 Super Mini over USB CDC — do not remove it.
- Gesture zones in `src/main.cpp`: closer than 100 mm turns the light off, 150–500 mm turns it on;
  `GUESTURE_DELAY` (2000 ms) debounces gestures **shared with the button**, so both paths check
  `millis() - guesture_time`. Any change must keep sensor and button from fighting each other.
- `platformio.ini` — dependencies are pinned (ETL, Gyver libraries, PubSubClient, ArduinoJson,
  VL53L0X). Add a new library only on request; do not bump versions "while at it".
  This is a single-purpose env (`[env:esp32c3]`); the `-D BOARD_ESP32_C3_SUPER_MINI` and USB CDC flags
  are required for the board, not leftovers.
- `src/version.h` — firmware version; `sync_version.py` copies it into `package.json` at build time.
  Bump PATCH for regular changes, MINOR for new features.
- Never commit or push to git yourself. When a development stage is finished and the user asked to
  bump the version, prepare the commit message text and show it — the user commits after review.

## Current state, code you may pick up

- Settings persistence is written but **switched off**: `USE_SETTINGS` is commented out in
  `src/light_control.h`, so brightness lives in a RAM variable. The `#ifdef` branches in
  `light_control.cpp` (and the unused `settings::kitchen_light_t`) must keep compiling in both modes —
  it is easy to break the other branch while editing one.
- `docs/todo_webui-mqt.md` and `docs/todo_ssl_web_update.md` are researched drafts (MQTT, Wi-Fi STA+AP,
  web UI, SSL certificate update). They are **not** the implemented architecture: the examples there
  use ESP8266 / `ESPAsyncWebServer` while this project is ETL-based. Do not copy them verbatim and do
  not treat them as a spec without asking.
- `lib/site_crt/` holds the wqtt.ru root certificate meant for the future TLS MQTT connection.

## Style

- Do not reformat other people's files wholesale: minimal diff, same style, same names.
- Comments in code — to the point and in Russian; do not comment the obvious.
- Do not touch `.pio/`, `docs/`, `README.md` without a reason.
- Commits are short, in the style of the history: `v0.2.1 upd to ETL 1.2.*`.

## Workflow

1. Read the affected files (and `README.md`) first, only then edit.
2. Change one logical thing at a time, and build after each change.
3. Use subagents for code search and for reviewing others' changes, but make the edits yourself.
4. If a build fails for an environment reason (no toolchain, no board, sandbox), say so directly; do
   not present it as a code bug.
