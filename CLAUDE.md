# RealBloom Mac Port

## What This Is

RealBloom is a standalone C++ bloom simulation tool (originally Windows-only) that we are porting to macOS / Apple Silicon. It generates physically accurate convolution bloom kernel textures using diffraction and dispersion simulation. The output (EXR/PNG kernel images) gets imported into Unreal Engine 5 for the Radio Arctica project.

**Upstream:** https://github.com/bean-mhm/realbloom
**Our fork:** https://github.com/lateralsummer/realbloom
**Licence:** AGPL-3.0

## This Is Not a UE5 Project

No Unreal Engine, no Blueprints, no data assets, no VibeUE. This is plain C++ with CMake, GLFW, Dear ImGui, and standard libraries. Do not apply UE5 conventions here.

## Development Rules

### Research First, Always

Before modifying any file, read it. Before attempting a fix, understand why the original code works on Windows. Grep the codebase and read the results before guessing.

### Minimal Changes

The goal is to make RealBloom compile and run on macOS with the smallest possible diff. Do not refactor, do not improve, do not reorganise. If Windows code works and compiles on Mac, leave it alone. Only change what fails.

### GPU FFT &mdash; Do Not Port to Metal

The `dj_fft` dependency uses OpenGL 4.3 compute shaders. macOS only supports OpenGL up to 4.1 (no compute shaders). The correct fix is:

1. Disable the dj_fft / GPU FFT code path on Mac (`#ifdef __APPLE__`)
2. Fall back to PocketFFT (CPU-based, already in the project)
3. CPU FFT at 2048&times;2048 on M4 Max is under 1 second &mdash; fast enough

Do NOT attempt a Metal compute port. That is a separate project entirely.

### Platform Fixes Pattern

When replacing Windows-specific code:

```cpp
// GOOD: conditional compilation, minimal change
#ifdef _WIN32
    #include <windows.h>
    // Windows-specific implementation
#else
    #include <unistd.h>
    // POSIX equivalent
#endif

// BAD: rewriting the function for "cleanliness"
// BAD: adding abstraction layers that don't exist upstream
```

Use `std::filesystem` for path operations, `std::chrono` for timing, POSIX equivalents for anything else. Keep the diff small.

### Build System

The original uses MSBuild (.sln/.vcxproj). We are adding a CMakeLists.txt alongside it (not replacing it). The .sln stays for Windows users.

Dependencies on Mac come from Homebrew:
- `brew install cmake glfw glew openimageio opencolorio pugixml`

Vendored dependencies (Dear ImGui, NFD Extended, PocketFFT, Rapidcsv, dj_fft) should be compiled from the repo&rsquo;s own copies, not from Homebrew.

### Two-Strikes Rule

If two distinct approaches to the same compilation error both fail, stop. Document what you tried, why it failed, and move on to the next file. Do not guess a third time. Flag it for review.

### Checkpoint After Audit

After the initial codebase audit (Phase 1), report findings and wait for go-ahead before writing any code. The audit should answer:
1. How many files have Windows-specific code?
2. Which dependencies are vendored vs. system?
3. Is dj_fft isolatable (can we `#ifdef` it out cleanly)?
4. Estimated difficulty: easy / moderate / hard / not worth it

## Build Commands

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$(brew --prefix)"
make -j$(sysctl -n hw.ncpu)
```

## What Success Looks Like

1. `cmake` configures without errors
2. `make` compiles without errors (warnings are acceptable)
3. The binary launches and shows the Dear ImGui GUI
4. You can generate a diffraction kernel and export it as EXR or PNG
5. The exported kernel is a valid 2048&times;2048 image with bright centre, dark edges

## Do Not

- Refactor code beyond what&rsquo;s needed for the port
- Port dj_fft to Metal
- Add features or change the UI
- Remove Windows support (all changes must be `#ifdef` guarded)
- Force the build if the audit reveals unexpected complexity
- Push to the fork until the build works (work on a local `mac-port` branch)
