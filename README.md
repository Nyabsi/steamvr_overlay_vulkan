# SteamVR Overlay Example

OpenVR overlay template project written in C++23 to create OpenVR overlays with ImGui utilizing modern and cross-platform graphic libraries

## Purpose

The purpose and goal of this project is to provide high quality, fully feature complete template for writing cross-platform and performant OpenVR overlays utilizing the best practises.

This project is intended for developers, not users. If your goal is not to create an OpenVR overlay then this project is not for you.

> [!WARNING]
> This project is made with Vulkan 1.3+ in mind, this code will not run on your GTX 7xx even if it is capable of running SteamVR

## Features

- Vulkan Rendering
	- Uses sRGB colour profile
 	- Dynamic Rendering
  	- HMD Refresh Rate synchronization
- ImGui multiple platform backends
	- Custom OpenVR backend built for OpenVR exclusively
 	- Optional SDL3 backend if your application requires an representable Window
- First class OpenVR support
	- Text Input
	- Mouse Emulation
		- Left, Right and Middle
		- Scroll Wheel
    	- Virtual Keyboard
    - Overlays
		- Dashboard
		- World
			- You can create overlays relative to Origin (World Space) or Tracked Devices (ie. Head, etc...)
    - Useful high-level OpenVR API wrappers
- Written cross-platform in mind
	- Supports both Linux and Windows
	- Full support for AMD, NVIDIA and Intel GPUs

## Building

> [!IMPORTANT]
> You need [Vulkan SDK](https://vulkan.lunarg.com/) to build this project, make sure you have it downloaded before proceeding

Make sure you have the required libraries fetched through `git submodule`

```sh
git submodule init && git submodule update
```

Build it through Visual Studio on Windows or if you're using an Unix-like system

```sh
cmake -B build .
cmake --build build
```

## Running

> [!IMPORTANT]
> This project uses CMake to copy all the required resources from `resources` to the build directory, if you're using custom build environment you need to copy them manually

Run `steamvr_overlay_vulkan.exe` (*or* `steamvr_overlay_vulkan` on Unix-like systems) from the build directory

## License

This project is licensed under `Mozilla Public License 2.0` which can be found from the root of this project named `LICENSE`

it comes with the following legal obligations:

- credit the project author (Nyabsi)
- disclose any changes done to files in scope of this project
- ship the license visibly in your project, such as `licenses` folder

### Exceptions

You may additionally license files that start with `imgui_` under `src/backends/` with `MIT` license while preserving copyright and credit

**NOTE: This is not legal advice, talk to your laywer or other legal representatives if you're uncertain**
