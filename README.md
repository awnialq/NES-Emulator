# NES-Emulator
Welcome to my NES Emulator. For a blog and status updates, please go to my website page listed here: [Blog](https://awnialq.github.io/portfolio/#/nesemu)
# How to Use
Compile the emulator with `make` from the project root.

Run it with a NES ROM path, for example `./bin/nes-emulator path/to/rom.nes`.

## Web build (Emscripten)

Requires the [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html) (`emcc` on your PATH).

```bash
make emscripten
cd web && python3 -m http.server
```

Open `http://localhost:8000`, upload a `.nes` ROM, and play in the browser. Supported mappers: 0 (NROM) and 3 (CNROM).

# Documentation
For a detailed walkthrough of the emulator architecture, CPU, PPU, SDL2 frontend, cartridge loading, mappers, build system, and current limitations, see [docs/IMPLEMENTATION.md](docs/IMPLEMENTATION.md).

# Tasks Left
- Implement Audio
- Implement more Mappers to support more games