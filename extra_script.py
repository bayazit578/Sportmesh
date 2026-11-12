import os
from pathlib import Path
Import("env")

# FreeRTOS copies the queue's configured item size into the caller's buffer.
# LoRaMesher v1.0.0 rounds this size up to four bytes, overflowing buffers for
# small items such as its uint8_t superframe notifications. Patch the installed
# dependency before compilation, including after dependency reinstalls.
rtos_header = (
    Path(env.subst("$PROJECT_LIBDEPS_DIR"))
    / env.subst("$PIOENV")
    / "LoRaMesher" / "src" / "os" / "rtos_freertos.hpp"
)
if rtos_header.is_file():
    original = rtos_header.read_bytes()
    patched = original.replace(
        b"itemSize = ((itemSize + 3) & ~3);  // Align to 4 bytes",
        b"// Keep the exact item size to avoid overwriting receive buffers.",
    ).replace(
        b"// Ensure minimum item size and alignment",
        b"// FreeRTOS manages queue storage alignment internally.",
    )
    if patched != original:
        rtos_header.write_bytes(patched)

# LoRaMesher imports projenv while library discovery can still be running.
# Supply the base environment until PlatformIO exports its project environment.
env.Export(dict(projenv=env))

# include toolchain paths
env.Replace(COMPILATIONDB_INCLUDE_TOOLCHAIN=True)

# Keep the database at the project root so language servers discover it.
env.Replace(COMPILATIONDB_PATH=os.path.join("$PROJECT_DIR", "compile_commands.json"))
