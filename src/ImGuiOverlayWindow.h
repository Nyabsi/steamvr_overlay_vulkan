/*
 * Copyright (C) 2025. Nyabsi <nyabsi@sovellus.cc>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <imgui.h>

#include "VulkanRenderer.h"
#include "VrOverlay.h"

class ImGuiOverlayWindow
{
public:
    explicit ImGuiOverlayWindow();
    auto Initialize(VulkanRenderer*& renderer, VrOverlay*& overlay, int width, int height) -> void;

    [[nodiscard]] auto OverlayData() -> Vulkan_Overlay* { return reinterpret_cast<Vulkan_Overlay*>(&overlay_data_); };

    auto Draw() -> void;
    auto Destroy() -> void;

private:

    Vulkan_Overlay overlay_data_;
};