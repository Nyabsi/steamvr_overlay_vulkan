/*
 * Copyright (C) 2025. Nyabsi <nyabsi@sovellus.cc>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "ImGuiOverlayWindow.h"

#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>

#include "backends/imgui_impl_openvr.h"

#include <math.h>

ImGuiOverlayWindow::ImGuiOverlayWindow()
{

}

auto ImGuiOverlayWindow::Initialize(VulkanRenderer*& renderer, VrOverlay*& overlay, int width, int height) -> void
{
    IMGUI_CHECKVERSION();

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;
    io.ConfigFlags  |= ImGuiConfigFlags_IsSRGB; // NOTE: ImGuiConfigFlags_IsSRGB is not used by ImGui, used to communicate state.

    io.IniFilename = nullptr;

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();

    style.ScaleAllSizes(1.0f);
    style.FontScaleDpi = 1.0f;

    if (io.ConfigFlags & ImGuiConfigFlags_IsSRGB) {
        // hack: ImGui doesn't handle sRGB colour spaces properly so convert from Linear -> sRGB
        // https://github.com/ocornut/imgui/issues/8271#issuecomment-2564954070
        // remove when these are merged:
        //  https://github.com/ocornut/imgui/pull/8110
        //  https://github.com/ocornut/imgui/pull/8111
        for (int i = 0; i < ImGuiCol_COUNT; i++) {
            ImVec4& col = style.Colors[i];
            col.x = col.x <= 0.04045f ? col.x / 12.92f : pow((col.x + 0.055f) / 1.055f, 2.4f);
            col.y = col.y <= 0.04045f ? col.y / 12.92f : pow((col.y + 0.055f) / 1.055f, 2.4f);
            col.z = col.z <= 0.04045f ? col.z / 12.92f : pow((col.z + 0.055f) / 1.055f, 2.4f);
        }
    }

    ImGui_ImplOpenVR_InitInfo openvr_init_info =
    {
        .handle = overlay->Handle(),
        .width = width,
        .height = height
    };

    ImGui_ImplOpenVR_Init(&openvr_init_info);

    VkSurfaceFormatKHR surface_format =
    {
        .format = VK_FORMAT_R8G8B8A8_SRGB,
        .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
    };

    VkPipelineRenderingCreateInfoKHR pipeline_rendering_create_info = 
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
        .viewMask = 0,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &surface_format.format,
        .depthAttachmentFormat = VK_FORMAT_UNDEFINED,
        .stencilAttachmentFormat = VK_FORMAT_UNDEFINED,
    };

    ImGui_ImplVulkan_InitInfo init_info = {
        .ApiVersion = VK_API_VERSION_1_3,
        .Instance = renderer->Instance(),
        .PhysicalDevice = renderer->PhysicalDevice(),
        .Device = renderer->Device(),
        .QueueFamily = renderer->QueueFamily(),
        .Queue = renderer->Queue(),
        .DescriptorPool = renderer->DescriptorPool(),
        .RenderPass = VK_NULL_HANDLE,
        .MinImageCount = 16,
        .ImageCount = 16,
        .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
        .PipelineCache = renderer->PipelineCache(),
        .Subpass = 0,
        .UseDynamicRendering = true,
        .PipelineRenderingCreateInfo = pipeline_rendering_create_info,
        .Allocator = renderer->Allocator(),
        .CheckVkResultFn = nullptr,
    };

    ImGui_ImplVulkan_Init(&init_info);
    renderer->SetupOverlay(width, height, surface_format);
}

auto ImGuiOverlayWindow::Draw() -> void
{
    static char buffer[128] = "Hello, world!";

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplOpenVR_NewFrame();
    ImGui::NewFrame();

    ImGuiIO& io = ImGui::GetIO();

    // == Menu Render Begin

    {
        static bool show_demo = true;
        ImGui::ShowDemoWindow(&show_demo);
    }

    {
        ImGui::Begin("Hello, world!");
        ImGui::Text("This is some useful text.");
        ImGui::InputText("Your input", buffer, IM_ARRAYSIZE(buffer));
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::End();
    }

    // == Menu Render End

    ImGui::Render();
}

auto ImGuiOverlayWindow::Destroy() -> void
{
    ImGui_ImplOpenVR_Shutdown();
}