# WISP_Payload_2026_27
WISP Payload Teams 2026/2027 Codebase

## Building and running

Requires a C++17 compiler (e.g. `g++`) and CMake 3.14+:

```
sudo apt install build-essential cmake
```

Quick version, from the repo root:

```
make build   # configure and compile into build/
make run     # build, then run the main control loop (Ctrl+C to stop)
make clean   # delete build/
```

Or call CMake directly. Configure and build (from the repo root):

```
cmake -S . -B build
cmake --build build
```

Run the main control loop (stop it with Ctrl+C):

```
./build/main_control
```

To rebuild after changing code, just re-run `cmake --build build`.
