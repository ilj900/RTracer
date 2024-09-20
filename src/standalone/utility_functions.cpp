#include "utility_functions.h"

#include "nlohmann_json/json.hpp"

#include <fstream>

using json = nlohmann::json;

void SaveCamera(ECS::FEntity Camera, const std::string& Name)
{
	auto& CameraComponent = ECS::GetCoordinator().GetComponent<ECS::COMPONENTS::FDeviceCameraComponent>(Camera);
	json Data;
	Data["Origin"] = { CameraComponent.Origin.X, CameraComponent.Origin.Y, CameraComponent.Origin.Z };
	Data["Direction"] = { CameraComponent.Direction.X, CameraComponent.Direction.Y, CameraComponent.Direction.Z };
	Data["Up"] = { CameraComponent.Up.X, CameraComponent.Up.Y, CameraComponent.Up.Z };
	Data["Right"] = { CameraComponent.Right.X, CameraComponent.Right.Y, CameraComponent.Right.Z };
	Data["SensorSizeX"] = CameraComponent.SensorSizeX;
	Data["SensorSizeY"] = CameraComponent.SensorSizeY;
	Data["FocalDistance"] = CameraComponent.FocalDistance;
	std::ofstream Result(Name + ".json");
	Result << Data.dump(4) << std::endl;
	Result.close();
}

void LoadCamera(ECS::FEntity Camera, const std::string& Name)
{
	auto& CameraComponent = ECS::GetCoordinator().GetComponent<ECS::COMPONENTS::FDeviceCameraComponent>(Camera);

	std::ifstream File(Name + ".json");
	json Data = json::parse(File);
	File.close();

	std::vector<float> Origin = Data["Origin"].get<std::vector<float>>();
	CameraComponent.Origin = {Origin[0], Origin[1], Origin[2]};

	std::vector<float> Direction = Data["Direction"].get<std::vector<float>>();
	CameraComponent.Direction = {Direction[0], Direction[1], Direction[2]};

	std::vector<float> Up = Data["Up"].get<std::vector<float>>();
	CameraComponent.Up = {Up[0], Up[1], Up[2]};

	std::vector<float> Right = Data["Right"].get<std::vector<float>>();
	CameraComponent.Right = {Right[0], Right[1], Right[2]};

	CameraComponent.SensorSizeX = Data["SensorSizeX"].get<float>();
	CameraComponent.SensorSizeY = Data["SensorSizeY"].get<float>();
	CameraComponent.FocalDistance = Data["FocalDistance"].get<float>();
	
	CAMERA_SYSTEM()->MarkDirty(Camera);
}