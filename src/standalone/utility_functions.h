#pragma once

#include "systems/camera_system.h"
#include "components/device_camera_component.h"

class FRender;

void SaveCamera(ECS::FEntity Camera,  std::shared_ptr<FRender>& Render, const std::string& Name);
void LoadCamera(ECS::FEntity Camera, std::shared_ptr<FRender>& Render, const std::string& Name);