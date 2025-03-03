#include "utility_functions.h"
#include "render.h"

#include "nlohmann_json/json.hpp"

#include <fstream>

using json = nlohmann::json;

void SaveCamera(ECS::FEntity Camera, std::shared_ptr<FRender>& Render, const std::string& Name)
{
	FVector3 CameraPosition;
	FVector3 CameraDirection;
	FVector3 CameraUp;
	float SensorSizeX;
	float SensorSizeY;
	float FocalDistance;

	Render->GetCameraPosition(&CameraPosition, &CameraDirection, &CameraUp, Camera);
	Render->GetCameraSensorProperties(&SensorSizeX, &SensorSizeY, &FocalDistance, Camera);

	json Data;
	Data["Origin"] = { CameraPosition.X, CameraPosition.Y, CameraPosition.Z };
	Data["Direction"] = { CameraDirection.X, CameraDirection.Y, CameraDirection.Z };
	Data["Up"] = { CameraUp.X, CameraUp.Y, CameraUp.Z };
	Data["SensorSizeX"] = SensorSizeX;
	Data["SensorSizeY"] = SensorSizeY;
	Data["FocalDistance"] = FocalDistance;
	std::ofstream Result(Name + ".json");
	Result << Data.dump(4) << std::endl;
	Result.close();
}

void LoadCamera(ECS::FEntity Camera, std::shared_ptr<FRender>& Render, const std::string& Name)
{
	std::ifstream File(Name + ".json");
	json Data = json::parse(File);
	File.close();

	std::vector<float> Origin = Data["Origin"].get<std::vector<float>>();
	std::vector<float> Direction = Data["Direction"].get<std::vector<float>>();
	std::vector<float> Up = Data["Up"].get<std::vector<float>>();

	float SensorSizeX = Data["SensorSizeX"].get<float>();
	float SensorSizeY = Data["SensorSizeY"].get<float>();
	float FocalDistance = Data["FocalDistance"].get<float>();

	Render->SetCameraPosition(FVector3{Origin[0], Origin[1], Origin[2]},
		FVector3{Direction[0], Direction[1], Direction[2]},
		FVector3{Up[0], Up[1], Up[2]}, Camera);
	Render->SetCameraSensorProperties(SensorSizeX, SensorSizeY, FocalDistance, Camera);
}