#!/bin/bash

mkdir -p components

git clone --branch "v1.0.0" https://github.com/LoRaMesher/LoRaMesher.git components/loramesher

cat > "components/loramesher/CMakeLists.txt" <<'CMAKE'
file(GLOB_RECURSE loramesher_sources
    "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
)

idf_component_register(
    SRCS ${loramesher_sources}
    INCLUDE_DIRS "src" "include"
    REQUIRES
        RadioLib
        espressif__arduino-esp32
        esp_timer
)
target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_20)
CMAKE

rm components/loramesher/idf_component.yml
