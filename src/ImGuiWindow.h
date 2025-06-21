/*
 * Copyright (C) 2025. Nyabsi <nyabsi@sovellus.cc>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>

#include <SDL3/SDL.h>

#include "VulkanRenderer.h"

class ImGuiWindow
{
public:
    explicit ImGuiWindow();
    auto Initialize(VulkanRenderer*& renderer, const char* name, int width, int height, float dpiScale, bool show = true) -> void;

    [[nodiscard]] auto Window() const -> SDL_Window* { return window_; };
    [[nodiscard]] auto WindowData() -> Vulkan_Window* { return reinterpret_cast<Vulkan_Window*>(&window_data_); };
    [[nodiscard]] auto Shown() const -> bool { return window_shown_; };
    [[nodiscard]] auto Minimized() const -> bool { return window_minimized_; };
    [[nodiscard]] auto KeyboardUnfocusPending() const -> bool { return keyboard_unfocus_requested_; };

    auto Hide() -> void;
    auto Show() -> void;
    auto SetMinimizedFromEvent(bool state) -> void;
    auto RequestKeyboardUnfocus() -> void;
    auto Draw() -> void;

    auto Destroy(VulkanRenderer*& renderer) -> void;

private:

    SDL_Window* window_;
    Vulkan_Window window_data_;
    bool window_shown_;
    bool window_minimized_;
    bool keyboard_unfocus_requested_;
};