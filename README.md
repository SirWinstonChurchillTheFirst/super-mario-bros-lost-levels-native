# Super Mario Bros.: The Lost Levels (Wii U Virtual Console): native build, work in progress

An attempt to run the Wii U Virtual Console release as a native PC program by static recompilation: the PowerPC
code of the Virtual Console program (which contains Nintendo's NES emulator) is translated to C++ ahead of time and
runs on a reimplementation of the Wii U's system libraries. No Wii U emulator.

> **No game files are included.** You need your own dump of the title (Europe, `WUP-FA9P.rpx`), unpacked to
> `code/`, `content/`, `meta/`. The translated C++ and the program built from it contain the title's code: do not
> share them. Not affiliated with or endorsed by Nintendo.

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
- `port/game_main.cpp`: a game-neutral entry program. `port/setup.sh`, `port/iter.sh`: the Linux build steps and loop.
- `tools/win_build.sh NAME`, `tools/quick.sh`, `tools/run_gpu.sh`: Windows build, a run without a window that prints the
  next missing system function, and a run on the graphics card with scripted presses, sound recording and pictures.

## License

The files here are under the MIT license, as the projects they build on. Third-party code keeps its own license.
