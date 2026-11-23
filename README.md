# Super Mario Bros.: The Lost Levels (Wii U Virtual Console): native build, work in progress

An attempt to run the Wii U Virtual Console release as a native PC program by static recompilation: the PowerPC
code of the Virtual Console program (which contains Nintendo's NES emulator) is translated to C++ ahead of time and
runs on a reimplementation of the Wii U's system libraries. No Wii U emulator.

> **No game files are included.** You need your own dump of the title (Europe, `WUP-FA9P.rpx`), unpacked to
> `code/`, `content/`, `meta/`. The translated C++ and the program built from it contain the title's code: do not
> share them. Not affiliated with or endorsed by Nintendo.

## Selbst bauen / Build it yourself

One command builds the native program from **your own dump** of Super Mario Bros.: The Lost Levels (Wii U Virtual Console). Nothing from the game is in this repository
or downloaded by the scripts: the game's code (WUP-FA9P.rpx) is translated to C++ and compiled on your computer.
*Ein Befehl baut das native Programm aus deinem eigenen Dump; nichts vom Spiel ist hier enthalten oder wird geladen.*

**You need / Du brauchst**
- your own dump of the game, **unpacked**: a folder with `code/`, `content/`, `meta/` (as Cemu or a Wii U dumper
  such as Dumpling gives it; `.wua`/`.wud` images must be unpacked first). Tested with Europe
  (000500001012F800); other versions are translated from your files the same way and may work.
- Windows 10/11 64-bit or Linux x86-64; a CPU with AVX2/FMA (Intel Haswell, AMD Zen or newer); a GPU with a Vulkan driver.
- internet for the build tools and the pinned sources (Smash4PC `f145e7f`, ZArchive, addrlib, Vulkan-Headers from GitHub).
- about **1 GB** free in this folder (plus about 3 GB for MSYS2 on Windows) and 8 GB of RAM or more.
- time: about **2-5 minutes** for the first build on an 8-core PC; a second run only redoes what changed.

**Windows** (PowerShell):

```powershell
git clone https://github.com/SirWinstonChurchillTheFirst/super-mario-bros-lost-levels-native
cd super-mario-bros-lost-levels-native
powershell -ExecutionPolicy Bypass -File .\setup-windows.ps1 -GameDir "D:\Games\Super Mario Bros.: The Lost Levels Wii U Virtual Console"
.\play\Start.bat
```

It installs [MSYS2](https://www.msys2.org) with winget if it is missing and the build packages into its CLANG64
environment with pacman (clang, lld, cmake, ninja, Python, git, SDL3, Vulkan loader + headers, shaderc, zlib, zstd,
xz, DejaVu fonts). Without `-GameDir` it asks for the folder. `-Jobs N` limits the parallel compile jobs; the build
runs at low priority so the PC stays usable.

**Linux** (Debian/Ubuntu, Arch, Fedora; others: install the same packages yourself):

```bash
git clone https://github.com/SirWinstonChurchillTheFirst/super-mario-bros-lost-levels-native
cd super-mario-bros-lost-levels-native
./setup-linux.sh "/path/to/Super Mario Bros.: The Lost Levels Wii U Virtual Console"
./play/start.sh
```

Missing packages are installed with sudo (apt: clang cmake ninja-build git python3 pkg-config zlib1g-dev libzstd-dev
libssl-dev libvulkan-dev libshaderc-dev fonts-dejavu-core, and libsdl3-dev where the distribution has it; without it
SDL 3.2.26 is built from source, e.g. on Ubuntu 24.04). `--jobs N` limits the compile jobs.

**What the setup does** (`tools/native-build.sh`, the same on both systems): checks your dump; clones
[Smash4PC](https://github.com/josiahg2502-glitch/Smash4PC) at `f145e7f` and the three pinned dependencies; applies
`port/upstream-local.patch` (all runtime/recompiler changes, made by the `port/patch_*.py` scripts in order); builds the
recompiler `cafe-recomp`; translates your `code/`; compiles runtime + translated game; and puts `cafegame`, a stand-in
font and the starter into `play/`. Everything it makes stays in `build/` and `play/` (both ignored by git): they hold
code translated from the game — **keep them for yourself, do not share them.**

The Wii U system font is not part of a game dump: DejaVu Sans stands in (`play/fonts/CafeStd.ttf`); put your console's
own `CafeStd.ttf` there to replace it. Saves go to `play/save/`. The system language follows your PC
(`CAFE_LANGUAGE=de|en|fr|...` overrides it).

**Getestet / tested (23 Nov 2026):** `setup-windows.ps1` in an empty folder on Windows 11 (i7-13700KF, 8 jobs): translated and compiled in under a minute once the MSYS2 packages were there; `Start.bat` starts the game (title and demo, about 60 frames/s, run with a PATH holding only Windows' folders, so no library is missing). `setup-linux.sh` on Ubuntu 26.04 (system SDL3) and on a fresh Ubuntu 24.04 (SDL3 built from source): both build, and the program runs (tested without a GPU in a VM: 59.9 frames/s headless).

## Status (22 November 2026): playable, native on a Batocera console at 60 frames/s

Tested on a PC (Windows 11, GeForce RTX 3070 Ti) and on a Batocera 43 mini PC (Ryzen 3 PRO 4350GE, Vega 6), with
scripted button presses.

| Step | State |
| --- | --- |
| Translate `WUP-FA9P.rpx` (4,722 functions, 0.29 million instructions) | works |
| Build with the runtime | works on Windows (clang, MSYS2) and Linux (clang, Ubuntu 24.04 build for the console) |
| Title screen, demo, starting a game with +, running and jumping in World 1-1 | works, picture right |
| Speed | 60 frames/s, steady: console 59.9/s, longest frame 18 ms over 2 minutes; CPU 59 C, GPU 51 C |
| Sound | works (music and effects, right pitch) since `port/patch_mix_devices.py` |
| Virtual Console save file (`WUP-FA9P.ves`) | written |
| Controller on the console, VC menu (restore points), manual | not tested by hand |

The "half-second hitch every few seconds" of the first tests was the test's own picture capture (each saved
1920x1080 PNG holds the renderer for ~0.5 s); without captures there is none.

Sound was silent because the title sets up its one voice (a looping PCM16 ring buffer the NES emulator fills) only
through the MIX library, and the runtime's `MIXInitDeviceControl` / `MIXSetInput` did nothing, so the voice had no
device mix. `port/patch_mix_devices.py` implements MIX's TV device mix (pan, span, fader, aux) as the console does
(`TTT2_MIX_DEVICES=0` = old behaviour).

## What is here

The same files as in the sibling repositories (for example
[super-mario-3d-world-native](https://github.com/SirWinstonChurchillTheFirst/super-mario-3d-world-native)), on top of
[Smash 4 PC](https://github.com/josiahg2502-glitch/Smash4PC) at `f145e7f` (MIT), which is built on
[Tekken Tag 2 Ultimate Edition](https://github.com/Petexy/Tekken-Tag-2-Ultimate-Edition)'s cafe-recomp and
Wii U runtime by Petexy (MIT). All credit for the recompiler and the runtime goes to them.

- `port/upstream-local.patch`: all changes to that runtime and recompiler; `port/patch_*.py` are the scripts that made
  them, each with the reason in its header.
- `port/os_extra.cpp`: system functions other titles need. What this title needed: block heaps
  (`MEMInitBlockHeap`, `MEMAllocFromBlockHeapEx`, `MEMFreeToBlockHeap`).
- `console/`: packaging for a Batocera console (`deploy.sh`, `launch.sh`, chooser entry).
- `port/game_main.cpp`: a game-neutral entry program. `port/iter.sh`: the build-and-run loop used while porting.
- `tools/win_build.sh NAME`, `tools/quick.sh`, `tools/run_gpu.sh`: Windows build, a run without a window that prints the
  next missing system function, and a run on the graphics card with scripted presses, sound recording and pictures.

## License

The files here are under the MIT license, as the projects they build on. Third-party code keeps its own license.
