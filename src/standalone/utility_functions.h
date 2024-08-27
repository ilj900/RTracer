#pragma once

#include "systems/camera_system.h"
#include "components/camera_component.h"

void SaveCamera(ECS::FEntity Camera, const std::string& Name);
void LoadCamera(ECS::FEntity Camera, const std::string& Name);