cmake_minimum_required(VERSION 3.26)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

project(steamvr_overlay_vulkan)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT DEFINED ENABLE_VULKAN_VALIDATION)
    set(ENABLE_VULKAN_VALIDATION OFF CACHE BOOL "Enable Vulkan validation layers" FORCE)
endif()

if(NOT DEFINED ENABLE_VULKAN_DYNAMIC_RENDERING)
    set(ENABLE_VULKAN_DYNAMIC_RENDERING OFF CACHE BOOL "Enable Vulkan dynamic rendering" FORCE)
endif()

if(NOT DEFINED IMGUI_OPENVR_PLATFORM_BACKEND)
    set(IMGUI_OPENVR_PLATFORM_BACKEND OFF CACHE BOOL "Headless OpenVR backend for ImGui" FORCE)
endif()

if(NOT DEFINED IMGUI_SDL_PLATFORM_BACKEND)
    set(IMGUI_SDL_PLATFORM_BACKEND OFF CACHE BOOL "SDL3 Window backend for ImGui" FORCE)
endif()

if(NOT DEFINED EXAMPLE_OVERLAY_TYPE_DASHBOARD)
    set(EXAMPLE_OVERLAY_TYPE_DASHBOARD OFF CACHE BOOL "Example Dashboard Overlay" FORCE)
endif()

if(NOT DEFINED EXAMPLE_OVERLAY_DEVICE_RELATIVE)
    set(EXAMPLE_OVERLAY_DEVICE_RELATIVE OFF CACHE BOOL "Example Device Relative World Overlay" FORCE)
endif()

if(NOT DEFINED EXAMPLE_OVERLAY_ORIGIN_RELATIVE)
    set(IMGUI_SDL_PLATFORM_BACKEND OFF CACHE BOOL "Example Origin Relative World Overlay" FORCE)
endif()

# Renderer configuration

# Vulkan validation layer adds extra reporting that may also catch validation layers orginating from external sources, ie. SteamVR
set(ENABLE_VULKAN_VALIDATION ON)

# ImGui backend configuration

# Custom OpenVR backend built for headless overlay rendering, this is the most performant option if you don't need physical Window as it does not present a swapchain
set(IMGUI_OPENVR_PLATFORM_BACKEND OFF)
# ImGui's official SDL3 backend for rendering cross-platform compatible Windows through usage of SDL3 which does require the usage of swapchain
set(IMGUI_SDL_PLATFORM_BACKEND ON)

# Example configuration

set(EXAMPLE_OVERLAY_TYPE_DASHBOARD ON)
# These need IMGUI_OPENVR_PLATFORM_BACKEND
set(EXAMPLE_OVERLAY_DEVICE_RELATIVE OFF)
set(EXAMPLE_OVERLAY_ORIGIN_RELATIVE OFF)

set(OpenVR_ROOT ${CMAKE_SOURCE_DIR}/3rdparty/OpenVR)
set(GLM_ROOT ${CMAKE_SOURCE_DIR}/3rdparty/glm)

find_package(OpenVR REQUIRED)

# Add GLM (avoiding an annoying target it adds by creating a dummy one with the same name).
add_custom_target(uninstall)
add_subdirectory(${GLM_ROOT})

# Include rest of the CMake bindings
add_subdirectory(3rdparty)

message(STATUS "ENABLE_VULKAN_VALIDATION = ${ENABLE_VULKAN_VALIDATION}")
message(STATUS "ENABLE_VULKAN_DYNAMIC_RENDERING = ${ENABLE_VULKAN_DYNAMIC_RENDERING}")
message(STATUS "IMGUI_OPENVR_PLATFORM_BACKEND = ${IMGUI_OPENVR_PLATFORM_BACKEND}")
message(STATUS "IMGUI_SDL_PLATFORM_BACKEND = ${IMGUI_SDL_PLATFORM_BACKEND}")

add_executable(steamvr_overlay_vulkan
    "src/Main.cpp"
    "src/VulkanRenderer.cpp"
    "src/ImGuiWindow.cpp"
    "src/ImGuiOverlayWindow.cpp"
)

target_compile_features(steamvr_overlay_vulkan PUBLIC cxx_std_23)

target_link_libraries(steamvr_overlay_vulkan
    PRIVATE
        OpenVR::API
        ImGui
        glm::glm
)

if (ENABLE_VULKAN_VALIDATION)
    add_definitions(-DENABLE_VULKAN_VALIDATION)
endif()

if (ENABLE_VULKAN_DYNAMIC_RENDERING)
    add_definitions(-DENABLE_VULKAN_DYNAMIC_RENDERING)
endif()

if (IMGUI_OPENVR_PLATFORM_BACKEND)
    add_definitions(-DIMGUI_OPENVR_PLATFORM_BACKEND)
endif()

if (IMGUI_SDL_PLATFORM_BACKEND)
    add_definitions(-DIMGUI_SDL_PLATFORM_BACKEND)
endif()

if (EXAMPLE_OVERLAY_TYPE_DASHBOARD)
    add_definitions(-DEXAMPLE_OVERLAY_TYPE_DASHBOARD)
endif()

if (EXAMPLE_OVERLAY_DEVICE_RELATIVE)
    add_definitions(-DEXAMPLE_OVERLAY_DEVICE_RELATIVE)
endif()

if (EXAMPLE_OVERLAY_ORIGIN_RELATIVE)
    add_definitions(-DEXAMPLE_OVERLAY_ORIGIN_RELATIVE)
endif()

add_custom_target(steamvr_overlay_vulkan_resources)

add_custom_command(
    TARGET steamvr_overlay_vulkan_resources POST_BUILD
    COMMENT "Copying Resources & Manifest"
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${CMAKE_SOURCE_DIR}/resources ${CMAKE_BINARY_DIR}
)
add_dependencies(steamvr_overlay_vulkan steamvr_overlay_vulkan_resources)

add_custom_command(
    TARGET steamvr_overlay_vulkan POST_BUILD
    COMMENT "Copying DLLs"
    COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:steamvr_overlay_vulkan> $<TARGET_RUNTIME_DLLS:steamvr_overlay_vulkan>
    COMMAND_EXPAND_LISTS
)