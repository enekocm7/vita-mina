# Vita Minesweeper

A native Minesweeper game for PlayStation Vita, written in C++17 and rendered with SDL2. It has no runtime asset or font dependencies.

## Features

- Easy: 9 × 9, 10 mines
- Medium: 16 × 16, 40 mines
- Hard: 24 × 16, 75 mines
- First reveal is always safe and normally opens a mine-free area
- Flood reveal and number chording
- Controller and front-touchscreen input
- Mine counter, timer, win/loss state, and instant restart

## Controls

| Vita input | Action |
| --- | --- |
| D-pad | Move cursor |
| Cross | Reveal cell / chord a revealed number |
| Square | Place or remove flag |
| L / R | Change difficulty and start a new game |
| Triangle or Start | Start a new game |
| Select | Exit |
| Touch | Tap to reveal; hold for 0.5 seconds to flag |

Desktop keyboard controls are also available for testing: arrow keys, `Enter`/`Space`, `F`, `Q`/`E`, `R`, and `Escape`.

## Build for PS Vita

Install [VitaSDK](https://vitasdk.org/) and its SDL2 package, then ensure `VITASDK` is set. Build the installable package with:

```sh
make
```

The build creates `build/vita_minesweeper.vpk`. Transfer that VPK to a homebrew-enabled Vita and install it with VitaShell.

Other useful targets:

```sh
make self     # Build only the Vita SELF executable
make test     # Build and run the board logic tests on the host
make rebuild  # Clean and rebuild the VPK
make clean    # Remove generated build files
make help     # List targets and configurable variables
```

You can customize builds with Make variables, for example:

```sh
make BUILD_TYPE=Debug JOBS=8
```

The underlying CMake build remains available directly if needed:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```
