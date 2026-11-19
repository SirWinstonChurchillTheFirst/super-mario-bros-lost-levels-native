# Super Mario Bros.: The Lost Levels (Wii U Virtual Console): native build, work in progress

An attempt to run the Wii U Virtual Console release as a native PC program by static recompilation: the PowerPC
code of the Virtual Console program (which contains Nintendo's NES emulator) is translated to C++ ahead of time and
runs on a reimplementation of the Wii U's system libraries. No Wii U emulator.

> **No game files are included.** You need your own dump of the title (Europe, `WUP-FA9P.rpx`), unpacked to
> `code/`, `content/`, `meta/`. The translated C++ and the program built from it contain the title's code: do not
> share them. Not affiliated with or endorsed by Nintendo.

## Status (19 November 2026): title screen and demo run, the game was not started

Tested on one PC (Windows 11, GeForce RTX 3070 Ti, 1280x720), with scripted button presses only.

| Step | State |
| --- | --- |
| Translate `WUP-FA9P.rpx` (4,722 functions, 0.29 million instructions) | works |
| Build with the runtime | works on Windows (clang, MSYS2) |
| Title screen of the game, its demo play | shown, look right |
| Starting a game | **not reached**: the scripted presses of A and + did not start it; why is not known (the buttons may not arrive, or the script pressed at the wrong moment) |
| Sound | **not verified**: the recording of the output is silent, and the title screen and demo of this game are silent on the console too |
| Speed | 51 to 54 frames/s in the test, with single pictures about half a second late. Not looked into (the test's picture captures cost time, but less than that) |
| Controller, Virtual Console menu, save states, manual | **not tested** |

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
- `port/game_main.cpp`: a game-neutral entry program. `port/setup.sh`, `port/iter.sh`: the Linux build steps and loop.
- `tools/win_build.sh NAME`, `tools/quick.sh`, `tools/run_gpu.sh`: Windows build, a run without a window that prints the
  next missing system function, and a run on the graphics card with scripted presses, sound recording and pictures.

## License

The files here are under the MIT license, as the projects they build on. Third-party code keeps its own license.
