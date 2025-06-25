/*
 * Copyright (C) 2025. Nyabsi <nyabsi@sovellus.cc>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <stdio.h>
#include <stdlib.h>

#include <sstream>
#include <fstream>
#include <vector>

#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_vulkan.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_vulkan.h>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <openvr.h>

#include "VulkanRenderer.h"
#include "VulkanUtils.h"

#include "ImGuiWindow.h"
#include "ImGuiOverlayWindow.h"

#include "VrOverlay.h"
#include "VrUtils.h"

#include "backends/imgui_impl_openvr.h"

#ifdef _WIN32
extern "C" __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
extern "C" __declspec(dllexport) unsigned long AmdPowerXpressRequestHighPerformance = 0x00000001;
#endif

static VulkanRenderer* g_vulkanRenderer = new VulkanRenderer();
static ImGuiWindow* g_imGuiWindow = new ImGuiWindow();
static ImGuiOverlayWindow* g_ImGuiOverlayWindow = new ImGuiOverlayWindow();
static VrOverlay* g_overlay = new VrOverlay();

static uint64_t g_last_frame_time = SDL_GetTicksNS();
static float g_hmd_refresh_rate = 24.0f;
static bool g_ticking = true;

#define APP_KEY     "github.VulkanOverlayExample"
#define APP_NAME    "Vulkan Overlay Example"

#define WIN_WIDTH   1280
#define WIN_HEIGHT  720

static auto UpdateApplicationRefreshRate() -> void
{
    try {
        auto hmd_properties = VrTrackedDeviceProperties::FromDeviceIndex(vr::k_unTrackedDeviceIndex_Hmd);
        hmd_properties.CheckConnection();
        g_hmd_refresh_rate = hmd_properties.GetFloat(vr::Prop_DisplayFrequency_Float);
    }
    catch (std::exception& ex) {
        printf("%s\n\n", ex.what());
        if (g_hmd_refresh_rate == 24.0f)
            std::exit(EXIT_FAILURE);
    }
}

int main(
    [[maybe_unused]] int argc, 
    [[maybe_unused]] char** argv
) {
    std::srand(std::time(nullptr));

    // Initialize the overlay as "VRApplication_Background" instead of "VRApplication_Overlay"
    // This makes sure that the overlay *cannot* run while SteamVR is not running.
    try {
        OpenVRInit(vr::VRApplication_Background);
    }
    catch (std::exception& ex) {
        printf("%s\n\n", ex.what());
        return EXIT_FAILURE;
    }

    UpdateApplicationRefreshRate();

    try {
        if (!OpenVRManifestInstalled(APP_KEY)) OpenVRManifestInstall();
    }
    catch (std::exception& ex) {
        printf("%s\n\n", ex.what());
        return EXIT_FAILURE;
    }
    
    try {
        char overlay_key[100];
        snprintf(overlay_key, 100, "%s-%d", APP_KEY, std::rand() % 1024); // chances of overlap? Slim.

#ifdef EXAMPLE_OVERLAY_TYPE_DASHBOARD
        g_overlay->Create(vr::VROverlayType_Dashboard, overlay_key, APP_NAME);

        // when overlay is VROverlayType_Dashboard we should set a thumbnail for the dashboard
        std::string thumbnail_path = {};
        thumbnail_path += SDL_GetCurrentDirectory();
        thumbnail_path += "icon.png";
        g_overlay->SetThumbnail(thumbnail_path);

        g_overlay->SetInputMethod(vr::VROverlayInputMethod_Mouse);
        g_overlay->SetWidth(2.5f);

        g_overlay->EnableFlag(vr::VROverlayFlags_SendVRDiscreteScrollEvents);
        g_overlay->EnableFlag(vr::VROverlayFlags_EnableClickStabilization);
#endif

#ifdef EXAMPLE_OVERLAY_DEVICE_RELATIVE
        g_overlay->Create(vr::VROverlayType_World, overlay_key, APP_NAME);

        g_overlay->SetInputMethod(vr::VROverlayInputMethod_Mouse);
        g_overlay->SetWidth(0.15f);

        g_overlay->EnableFlag(vr::VROverlayFlags_SendVRDiscreteScrollEvents);
        g_overlay->EnableFlag(vr::VROverlayFlags_EnableClickStabilization);
        g_overlay->EnableFlag(vr::VROverlayFlags_MakeOverlaysInteractiveIfVisible);

        // Device relative offset
        glm::vec3 position = { -0.10, 0, 0.10 };
        glm::quat rotation = glm::angleAxis(glm::half_pi<float>(), glm::vec3(0, 1, 0)) * glm::angleAxis(-glm::half_pi<float>(), glm::vec3(1, 0, 0));
        rotation *= glm::angleAxis(glm::radians(10.0f), glm::vec3(0, 1, 0));
        rotation = glm::normalize(rotation);

        g_overlay->SetTransformDeviceRelative(vr::TrackedControllerRole_LeftHand, position, rotation);
        g_overlay->Show();
#endif

#ifdef EXAMPLE_OVERLAY_ORIGIN_RELATIVE
        g_overlay->Create(vr::VROverlayType_World, overlay_key, APP_NAME);

        g_overlay->SetInputMethod(vr::VROverlayInputMethod_Mouse);
        g_overlay->SetWidth(1.0f);

        g_overlay->EnableFlag(vr::VROverlayFlags_SendVRDiscreteScrollEvents);
        g_overlay->EnableFlag(vr::VROverlayFlags_EnableClickStabilization);
        g_overlay->EnableFlag(vr::VROverlayFlags_MakeOverlaysInteractiveIfVisible);

        // Origin relative offset
        glm::vec3 position = { 0.0f, 1.5f, -1.0f };
        glm::quat rotation = glm::quat_identity<float, glm::defaultp>();

        g_overlay->SetTransformWorldRelative(vr::TrackingUniverseStanding, position, rotation);
        g_overlay->Show();
#endif
    }
    catch (std::exception& ex) {
        printf("%s\n\n", ex.what());
        return EXIT_FAILURE;
    }

#ifdef IMGUI_SDL_PLATFORM_BACKEND
    auto sdl_init_flags = SDL_INIT_VIDEO | SDL_INIT_AUDIO;
    if (!SDL_Init(sdl_init_flags)) {
        printf("SDL_Init\n%s\n\n", SDL_GetError());
        return EXIT_FAILURE;
    }
#endif

    g_vulkanRenderer->Initialize();

#ifdef IMGUI_OPENVR_PLATFORM_BACKEND
    g_ImGuiOverlayWindow->Initialize(g_vulkanRenderer, g_overlay, WIN_WIDTH, WIN_HEIGHT);
#else
    float dpiScale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    g_imGuiWindow->Initialize(g_vulkanRenderer, APP_NAME, WIN_WIDTH, WIN_HEIGHT, dpiScale);
    g_vulkanRenderer->SetupOverlay(WIN_WIDTH, WIN_HEIGHT, g_imGuiWindow->WindowData()->surface_format);
#endif

    SDL_Event event = {};
    vr::VREvent_t vr_event = {};

    while (g_ticking)
    {
#ifdef IMGUI_SDL_PLATFORM_BACKEND
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);

            if (event.type == SDL_EVENT_WINDOW_MINIMIZED && event.window.windowID == SDL_GetWindowID(g_imGuiWindow->Window()))
                g_imGuiWindow->SetMinimizedFromEvent(true);
            if (event.type == SDL_EVENT_WINDOW_RESTORED && event.window.windowID == SDL_GetWindowID(g_imGuiWindow->Window()))
                g_imGuiWindow->SetMinimizedFromEvent(false);
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(g_imGuiWindow->Window()))
                g_ticking = false;
        }
#endif
        while (vr::VROverlay()->PollNextOverlayEvent(g_overlay->Handle(), &vr_event, sizeof(vr_event))) 
        {
            ImGui_ImplOpenVR_ProcessOverlayEvent(vr_event);

            switch (vr_event.eventType) 
            {
                case vr::VREvent_PropertyChanged:
                {
                    // Some drivers such as lighthouse or vrlink are capable of changing
                    // vr::Prop_DisplayFrequency_Float without restarting SteamVR
                    if (vr_event.data.property.prop == vr::Prop_DisplayFrequency_Float) {
                        UpdateApplicationRefreshRate();
                    }
                    break;
                }
#ifdef IMGUI_SDL_PLATFORM_BACKEND
                case vr::VREvent_OverlayShown:
                {
                    if (g_overlay->IsVisible() && g_imGuiWindow->Shown()) {
                        g_imGuiWindow->Hide();
                    }
                    break;
                }
                case vr::VREvent_OverlayHidden:
                {
                    if (!g_overlay->IsVisible() && g_imGuiWindow->Shown()) {
                        g_imGuiWindow->Show();
                    }
                    break;
                }
#endif
                case vr::VREvent_Quit:
                {
                    g_ticking = false;
                    return false;
                }
            }
        }

#ifdef IMGUI_OPENVR_PLATFORM_BACKEND
        g_ImGuiOverlayWindow->Draw();
#endif

#ifdef IMGUI_SDL_PLATFORM_BACKEND
        {
            ImGuiIO& io = ImGui::GetIO();

            if (!io.WantTextInput) {
                g_imGuiWindow->SetKeyboardActiveState(false);
            }

            if (g_overlay->IsVisible() && !g_imGuiWindow->KeyboardActive() && io.WantTextInput) {
                g_overlay->ShowKeyboard(vr::k_EGamepadTextInputModeNormal);
                g_imGuiWindow->SetKeyboardActiveState(true);
            }
        }

        int fb_width = {};
        int fb_height = {};
        SDL_GetWindowSize(g_imGuiWindow->Window(), &fb_width, &fb_height);

        fb_width *= static_cast<int>(dpiScale);
        fb_height *= static_cast<int>(dpiScale);

        if ((fb_width != 0 && fb_height != 0) && (g_vulkanRenderer->ShouldRebuildSwapchain() || g_imGuiWindow->WindowData()->width != fb_width || g_imGuiWindow->WindowData()->height != fb_height))
        {
            ImGui_ImplVulkan_SetMinImageCount(g_vulkanRenderer->MinimumConcurrentImageCount());

            g_imGuiWindow->WindowData()->width = fb_width;
            g_imGuiWindow->WindowData()->width = fb_height;

            g_vulkanRenderer->SetupSwapchain(g_imGuiWindow->WindowData(), fb_width, fb_height);
            g_imGuiWindow->WindowData()->frame_index = 0;
        }

        g_overlay->SetMouseScale(fb_width, fb_height);
        g_imGuiWindow->Draw();
#endif

        ImDrawData* draw_data = ImGui::GetDrawData();

#ifdef IMGUI_OPENVR_PLATFORM_BACKEND
        g_vulkanRenderer->RenderOverlay(draw_data, g_overlay);
#endif

#ifdef IMGUI_SDL_PLATFORM_BACKEND
        const ImVec4 background_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

        g_imGuiWindow->WindowData()->clear_value.color.float32[0] = background_color.x * background_color.w;
        g_imGuiWindow->WindowData()->clear_value.color.float32[1] = background_color.y * background_color.w;
        g_imGuiWindow->WindowData()->clear_value.color.float32[2] = background_color.z * background_color.w;
        g_imGuiWindow->WindowData()->clear_value.color.float32[3] = background_color.w;

        const bool is_minimized = g_imGuiWindow->Shown() && g_imGuiWindow->Minimized();
        g_imGuiWindow->WindowData()->is_minimized = is_minimized;

        if (!is_minimized) {
            g_vulkanRenderer->RenderWindow(draw_data, g_imGuiWindow->WindowData());
            g_vulkanRenderer->Present(g_imGuiWindow->WindowData());
        }
        
        g_vulkanRenderer->RenderOverlay(draw_data, g_overlay);
#endif
        uint64_t target_time = static_cast<uint64_t>((static_cast<float>(1000000000) / g_hmd_refresh_rate));
        const uint64_t frame_duration = (SDL_GetTicksNS() - g_last_frame_time);

        if (frame_duration < target_time) {
            SDL_DelayPrecise(target_time - frame_duration);
        }

        g_last_frame_time = SDL_GetTicksNS();
    }

    VkResult vk_result = vkDeviceWaitIdle(g_vulkanRenderer->Device());
    VK_VALIDATE_RESULT(vk_result);

    g_ImGuiOverlayWindow->Destroy();
    g_vulkanRenderer->DestroyWindow(g_imGuiWindow->WindowData());
    g_imGuiWindow->Destroy(g_vulkanRenderer);
    g_vulkanRenderer->Destroy();

    ImGui::DestroyContext();

    SDL_Quit();

    return 0;
}
