import os
Import("env")

# LoRaMesher imports projenv while library discovery can still be running.
# Supply the base environment until PlatformIO exports its project environment.
env.Export(dict(projenv=env))

# include toolchain paths
env.Replace(COMPILATIONDB_INCLUDE_TOOLCHAIN=True)

# Keep the database at the project root so language servers discover it.
env.Replace(COMPILATIONDB_PATH=os.path.join("$PROJECT_DIR", "compile_commands.json"))
