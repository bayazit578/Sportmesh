import os
from pathlib import Path
Import("env")

# LoRaMesher assumes compiler flags are strings, but SCons also accepts
# nested lists. Patch each environment's dependency before library discovery.
library_script = (
  Path(env.subst("$PROJECT_LIBDEPS_DIR"))
  / env.subst("$PIOENV")
  / "LoRaMesher" / "scripts" / "extra_script.py"
)
if library_script.is_file():
  original_script = library_script.read_bytes()
  patched_script = original_script.replace(
    b'for f in environment.get("CXXFLAGS", [])',
    b'for f in environment.Flatten(environment.get("CXXFLAGS", []))',
  )
  if patched_script != original_script:
    library_script.write_bytes(patched_script)

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
