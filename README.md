# What is it?

This is a simple project for understanding lighting in computer graphics.

## How to use?

## Step 1
- navigate to cloned repo
- git clone https://github.com/microsoft/vcpkg.git

## Step 2 - Windows
- .\vcpkg\bootstrap-vcpkg.bat
- .\vcpkg\vcpkg install glm:x64-windows
- .\vcpkg\vcpkg install sdl2:x64-windows

## Step 2 - Linux
- ./vcpkg/bootstrap-vcpkg.bat
- ./vcpkg/vcpkg install glm:x64-linux
- ./vcpkg/vcpkg install sdl2:x64-linux

## Step 3
- mkdir build
- cd build
- cmake -DCMAKE_TOOLCHAIN_FILE="./vcpkg/scripts/buildsystems/vcpkg.cmake" .. 

## Gallery

| test1_gradient.ppm  |
| ------------- 	  |
| ![](images/1.png)    |

| test2_background.ppm  |
| ------------- 	  |
| ![](images/2.png)    |

| test3_sphere.ppm  |
| ------------- 	  |
| ![](images/3.png)    |

| test4_world_sphere.ppm  |
| ------------- 	  |
| ![](images/4.png)    |
