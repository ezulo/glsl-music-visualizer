# GLSL Music Visualizer

A real-time music visualizer using OpenGL Shader Language.

Currently building out in Linux, but the intention is cross-platform use.

## Build Dependencies

```bash

# For window management
pacman -S glfw mesa glad

# For audio streaming
pacman -S libpulse

# Fast Fourier Transform library for freq. analysis
pacman -S fftw

# General
A window system / compositor (no TTY usage)

```

## Building

```bash
make
```

## Running

```bash
make run
```

## Project Structure

```
.
├── main.c                  # Application entry point
├── shaders/
│   ├── vertex.glsl         # Vertex shader
│   └── fragment.glsl       # Fragment shader
└── Makefile
```
