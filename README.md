<a name="readme-top"></a>

<br />
<div align="center">

<h3 align="center">cstrl</h3>

  <p align="center">
    A simple game framework.
    <br />
    <br />
    <a href="https://github.com/smeinscher/cstrl">View Demo (coming soon...)</a>
  </p>
</div>

## About The Project

This is a simple game framework made for educational purposes. The goal is to create my own games with this.

Motivation:
* I enjoy programming, not so much learning the UI or functionality of a traditional game engine.
* The extra time spent working on this will allow me to polish my programming skills while making games.
* I want to learn how to make projects publicly accessible, and easy to get working on different machines.

## External Tools and Libraries

Graphics APIs:
* [OpenGL](https://www.opengl.org/)
* [Vulkan (still learning this)](https://www.vulkan.org/)

Libraries:
* [glad](https://github.com/Dav1dde/glad)
* [stb](https://github.com/nothings/stb)
* [log.c](https://github.com/rxi/log.c)

Tools
* [CMake](https://cmake.org/)
* [Ninja](https://ninja-build.org/)

Working Compilers:
* [MinGW](https://www.mingw-w64.org/)
* [Clang](https://clang.llvm.org/)
* [GCC](https://gcc.gnu.org/)

## Getting Started

Should work on Windows and Linux. There is currently no Mac support. Using the library for Android should work as well (although it has not been tested in a while).

### Prerequisites

[CMake](https://cmake.org/) and [Ninja](https://ninja-build.org/) are recommended for building. For Visual Studio users: Note that this code will not work with MSVC, as the language standard used in this project is C11. See [here](https://stackoverflow.com/questions/48981823/is-there-any-option-to-switch-between-c99-and-c11-c-standards-in-visual-studio)
a stackoverflow post describing the issue with MSVC. If you want to use Visual Studio with this project, there are ways to use Clang and MinGW compilers instead of MSVC.

### Building

1. Clone the repo
   ```sh
    git clone https://github.com/smeinscher/STRL.git
    ```
2. This step assumes you're using bash. You can build the project with the provided script:
   ```sh
   ./cmake_build.sh
   ```
   Running without arguments defaults to Debug OPENGL.
   Possible arguments are:
   <br />
     $1: Debug/Release
   <br />
     $2: OPENGL/VULKAN
   <br />
   Note that Vulkan is not supported yet so things will break if you build with that option.
   <br />
   You can build and run the tests with the script provided:
   ```sh
   ./run_tests.sh
   ```
   OR if you want to build and run the sandbox code:
   ```sh
   ./run_sandbox.sh
   ```

   Instructions for alternative building methods will come soon.
