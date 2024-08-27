#include "utility_functions.h"

#include "nlohmann_json/json.hpp"

#include <fstream>

using json = nlohmann::json;

void SaveCamera(ECS::FEntity Camera, const std::string& Name)
{
	auto& CameraComponent = ECS::GetCoordinator().GetComponent<ECS::COMPONENTS::FCameraComponent>(Camera);
	json Data;
	Data["Position"] = {CameraComponent.Position.X, CameraComponent.Position.Y, CameraComponent.Position.Z };
	Data["Direction"] = {CameraComponent.Direction.X, CameraComponent.Direction.Y, CameraComponent.Direction.Z };
	Data["Up"] = {CameraComponent.Up.X, CameraComponent.Up.Y, CameraComponent.Up.Z };
	Data["ZNear"] = CameraComponent.ZNear;
	Data["ZFar"] = CameraComponent.ZFar;
	Data["FOV"] = CameraComponent.FOV;
	Data["Ratio"] = CameraComponent.Ratio;
	std::ofstream Result(Name + ".json");
	Result << Data.dump(4) << std::endl;
	Result.close();
}

void LoadCamera(ECS::FEntity Camera, const std::string& Name)
{
	auto& CameraComponent = ECS::GetCoordinator().GetComponent<ECS::COMPONENTS::FCameraComponent>(Camera);

	std::ifstream File(Name + ".json");
	json Data = json::parse(File);
	File.close();

	std::vector<float> Position = Data["Position"].get<std::vector<float>>();
	CameraComponent.Position = {Position[0], Position[1], Position[2]};

	std::vector<float> Direction = Data["Direction"].get<std::vector<float>>();
	CameraComponent.Direction = {Direction[0], Direction[1], Direction[2]};

	std::vector<float> Up = Data["Up"].get<std::vector<float>>();
	CameraComponent.Up = {Up[0], Up[1], Up[2]};

	CameraComponent.ZNear = Data["ZNear"].get<float>();
	CameraComponent.ZFar = Data["ZFar"].get<float>();
	CameraComponent.FOV = Data["FOV"].get<float>();
	CameraComponent.Ratio = Data["Ratio"].get<float>();
	
	CAMERA_SYSTEM()->MarkDirty(Camera);
}