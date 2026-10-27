# Repository Guidelines

## Project Structure & Module Organization

This repository contains ESP32 Arduino firmware demonstrating LoRaMesher networking on the `nodemcu-32s` board.

- `src/main.cpp`: radio pin assignments, mesh initialization, receive callbacks, routing diagnostics, and periodic peer messages.
- `include/`: shared project headers; currently contains only a placeholder README.
- `lib/`: private libraries, organized as `lib/<library>/src/`; currently empty apart from its README.
- `test/`: reserved for PlatformIO tests; no tests are implemented.
- `platformio.ini`: board, framework, serial settings, and the LoRaMesher dependency on its `new_loramesher` branch.
- `extra_script.py`: compilation database configuration. Treat `.pio/` and compilation databases as generated output.

## Build, Test, and Development Commands

Install PlatformIO Core and run commands from the repository root:

- `pio run -e nodemcu-32s`: resolve dependencies and compile firmware.
- `pio run -e nodemcu-32s -t upload`: flash a connected board.
- `pio device monitor -b 115200`: inspect serial diagnostics.
- `pio run -e nodemcu-32s -t clean`: remove build output.
- `pio run -e nodemcu-32s -t compiledb`: generate a compilation database using the configured build-directory path.
- `pio test -e nodemcu-32s`: run tests once suites are added under `test/`.

## Coding Style & Naming Conventions

Follow existing C++ style: four-space indentation, opening braces on the declaration line, uppercase `LORA_*` configuration macros, and descriptive names. Existing functions mix camelCase helpers and PascalCase callbacks; match the surrounding code. Keep receive callbacks short and move substantial processing into a separate task. No formatter or linter is configured; avoid unrelated formatting changes.

## Testing Guidelines

No test framework or coverage threshold is explicitly configured. Add future suites under `test/test_<feature>/` and document their hardware requirements. For firmware changes, build first, then validate discovery, routing updates, and message exchange with compatible nodes. Record board models, radio settings, and relevant serial output.

## Commit & Pull Request Guidelines

Git history is unavailable in this checkout, so no existing commit convention can be verified. Use short, imperative commit subjects such as `Fix peer message scheduling`. Pull requests should explain the behavior change, link relevant issues, and report build results and hardware checks. Highlight changes to pin mappings, radio parameters, or dependency branches.
