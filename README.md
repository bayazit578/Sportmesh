# Sportmesh

## ESP firmware

### Prerequisites

You need to have `ESP-IDF` installed. ![Instruction](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html).

## Build & run

In project root directory, with `esp-idf` venv activated, run:
```shell
idf.py build
```

## Components

| Component | Description |
| --------- | ----------- |
| ra01      | ra01 (sx 126x) driver |

## Tests

Tests for each component are in `components/<component_name>/test-apps`
In order to run a test, `cd` in test directory and run `idf.py build`, `idf.py flush`, ...
