# What is it?

This is a simple project for understanding lighting in computer graphics.

## How to use?

- navigate to cloned repo
- git clone https://github.com/microsoft/vcpkg.git

## Windows
- .\vcpkg\bootstrap-vcpkg.bat
- .\vcpkg\vcpkg install glm:x64-windows
- .\vcpkg\vcpkg install sdl2:x64-windows

## Linux
- ./vcpkg/bootstrap-vcpkg.bat
- ./vcpkg/vcpkg install glm:x64-linux
- ./vcpkg/vcpkg install sdl2:x64-linux

- mkdir build
- cd build
- cmake -DCMAKE_TOOLCHAIN_FILE="./vcpkg/scripts/buildsystems/vcpkg.cmake" .. 

## Gallery

| test1_gradient.ppm  |
| ------------- 	  |
| [[images/1.png]]    |
