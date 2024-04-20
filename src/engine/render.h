#pragma once

#include "image.h"

#include "maths.h"

#include "entity.h"

#include "tasks/task_generate_initial_rays.h"
#include "tasks/task_raytrace.h"
#include "tasks/task_material_sort_clear_materials_count_per_chunk.h"
#include "tasks/task_material_sort_clear_total_materials_count.h"
#include "tasks/task_material_sort_compute_offsets_per_material.h"
#include "tasks/task_material_sort_count_materials_per_chunk.h"
#include "tasks/task_material_sort_sort_materials.h"
#include "tasks/task_material_sort_compute_prefix_sums_down_sweep.h"
#include "tasks/task_material_sort_compute_prefix_sums_up_sweep.h"
#include "tasks/task_material_sort_compute_prefix_sums_zero_out.h"
#include "tasks/task_update_tlas.h"
#include "tasks/task_miss.h"
#include "tasks/task_shade.h"
#include "tasks/task_accumulate.h"
#include "tasks/task_passthrough.h"
#include "tasks/task_clear_image.h"

#include <string>
#include <vector>

enum class OutputType {Color0 = 0, Color1 = 1, Color2 = 2, Normal, UV};

class FRender
{
public:
    FRender(uint32_t WidthIn = 1920, uint32_t HeightIn = 1080);
	~FRender();

    void RegisterExternalOutputs(std::vector<ImagePtr> OutputImagesIn, const std::vector<FSynchronizationPoint>& ExternalImageIsReadyIn);
    int Init();
    int Cleanup();
    int SetSize(int WidthIn, int HeightIn);

    ECS::FEntity CreateCamera();
    ECS::FEntity CreateFramebuffer(int WidthIn, int HeightIn, const std::string& DebugName = "");
	void DestroyFramebuffer(ECS::FEntity Framebuffer);
    ECS::FEntity CreateFramebufferFromExternalImage(ImagePtr ImageIn, const std::string& DebugName = "");
	ECS::FEntity CreateColorAttachment(int WidthIn, int HeightIn, const std::string& DebugName = "");
    void SetActiveCamera(ECS::FEntity Camera);
    void SetOutput(OutputType OutputTypeIn, ECS::FEntity Framebuffer);
    ECS::FEntity GetOutput(OutputType OutputTypeIn);
    void SaveFramebuffer(ECS::FEntity Framebuffer, const std::string& Filename = "");
	void SaveOutput(OutputType OutputTypeIn, const std::string& Filename);
    void GetFramebufferData(ECS::FEntity Framebuffer);

	void AddExternalTaskAfterRender(std::shared_ptr<FExecutableTask> Task);
	FSynchronizationPoint Render();
    FSynchronizationPoint Render(uint32_t OutputImageIndex);
    int Update();
	void WaitIdle();

    ECS::FEntity CreateTexture(const std::string& FilePath);
    ECS::FEntity CreateMaterial(const FVector3& BaseColor);
    ECS::FEntity ShapeSetMaterial(ECS::FEntity Shape, ECS::FEntity Material);
    void MaterialSetBaseColor(ECS::FEntity Material, ECS::FEntity Image);
    void MaterialSetBaseColor(ECS::FEntity Material, const FVector3& Value);
    void MaterialSetDiffuseRoughness(ECS::FEntity Material, ECS::FEntity Image);
    void MaterialSetDiffuseRoughness(ECS::FEntity Material, float Value);
    void MaterialSetNormal(ECS::FEntity Material, const FVector3& Value);
    void MaterialSetNormal(ECS::FEntity Material, ECS::FEntity Image);
    ECS::FEntity CreatePlane(const FVector2& Size);
    ECS::FEntity CreatePyramid();
    ECS::FEntity CreateCube();
    ECS::FEntity CreateIcosahedronSphere(float Radius, int LevelOfComplexity, bool bJagged);
	ECS::FEntity CreateUVSphere(uint32_t LongitudeCount, uint32_t LatitudeCount);
    ECS::FEntity CreateModel(const std::string& Path);

    ECS::FEntity CreateInstance(ECS::FEntity BaseModel, const FVector3& Position, const FVector3& Direction = {0, 0, 1}, const FVector3& Up = {0, 1 ,0});
	void SetInstancePosition(ECS::FEntity Instance, const FVector3& Position);
	FVector3 GetInstancePosition(ECS::FEntity Instance);
	ECS::FEntity CreateLight(const FVector3& Position);
	void SetLightPosition(ECS::FEntity Light, const FVector3& Position);
	FVector3 GetLightPosition(ECS::FEntity Light);
    int SetIBL(const std::string& Path);

    bool bWasResized = false;

    uint32_t MaxFramesInFlight = 2;
    uint32_t RenderFrameIndex = 0;

    ECS::FEntity ActiveCamera;

	std::shared_ptr<FUpdateTLASTask> UpdateTLASTask = nullptr;
    std::shared_ptr<FGenerateInitialRays> GenerateRaysTask = nullptr;
    std::shared_ptr<FRaytraceTask> RayTraceTask = nullptr;
    std::shared_ptr<FClearMaterialsCountPerChunkTask> ClearMaterialsCountPerChunkTask = nullptr;
    std::shared_ptr<FClearTotalMaterialsCountTask> ClearTotalMaterialsCountTask = nullptr;
    std::shared_ptr<FComputeOffsetsPerMaterialTask> ComputeOffsetsPerMaterialTask = nullptr;
    std::shared_ptr<FCountMaterialsPerChunkTask> CountMaterialsPerChunkTask = nullptr;
    std::shared_ptr<FSortMaterialsTask> SortMaterialsTask = nullptr;
    std::shared_ptr<FComputePrefixSumsDownSweepTask> ComputePrefixSumsDownSweepTask = nullptr;
    std::shared_ptr<FComputePrefixSumsUpSweepTask> ComputePrefixSumsUpSweepTask = nullptr;
    std::shared_ptr<FComputePrefixSumsZeroOutTask> ComputePrefixSumsZeroOutTask = nullptr;
    std::shared_ptr<FShadeTask> ShadeTask = nullptr;
    std::shared_ptr<FMissTask> MissTask = nullptr;
    std::shared_ptr<FAccumulateTask> AccumulateTask = nullptr;
    std::shared_ptr<FPassthroughTask> PassthroughTask = nullptr;
    std::shared_ptr<FClearImageTask> ClearImageTask = nullptr;
	std::vector<std::shared_ptr<FExecutableTask>> ExternalTasks;

    std::vector<FSynchronizationPoint> ImageAvailable;
    std::vector<VkFence> ImagesInFlight;

	std::vector<FSynchronizationPoint> ExternalImageAvailable;
    std::unordered_map<OutputType, ECS::FEntity> OutputToFramebufferMap;

private:
    ECS::FEntity CreateEmptyModel();
    int Width;
    int Height;
};