# Getting Started
Get SableUI runnning in under 5 minutes.

## Building from source
> NOTE: Building from source is required until v1.0 release
### Prerequisites
- C++20 compiler
- CMake 3.15+
- Git
- ~Vulkan SDK~

#### Platform-specific requirements:
- **Linux:** - Development libraries for OpenGL
- **macOS** Xcode Command Line Tools

## Installation
### Add to Existing CMake Project
> Git Submodules only works if `your-project` is initialised with git, if not, you can run `git init` before continuing or go with option 2.
Add SableUI as a submodule to your project:
```bash
cd your-project
git submodule add https://github.com/oliwilliams1/SableUI vendor/SableUI
git submodule update --init --recursive
```
Update your `CMakeLists.txt`:
```
# Add SableUI
add_subdirectory(vendor/SableUI)

# Link to your executable
add_executable(MyApp main.cpp)
target_link_libraries(MyApp PRIVATE SableUI)
```

Example `CMakeLists.txt`:
```
cmake_minimum_required(VERSION 3.15)

project(MyApp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add SableUI
add_subdirectory(vendor/SableUI)

add_executable(MyApp main.cpp)
target_link_libraries(MyApp PRIVATE SableUI)
```
---

Having Issues? Check [troubleshooting](troubleshooting-configuration.md)

Now you have configured SableUI, you can now create [your first application](your-first-application.md)!