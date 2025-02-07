#pragma once

#include "image.h"

#include "maths.h"

#include "entity.h"

#include "tasks/task_update_tlas.h"
#include "tasks/task_copy_buffer.h"
#include "tasks/task_clear_image.h"
#include "tasks/task_generate_initial_rays.h"
#include "tasks/task_reset_active_ray_count.h"
#include "tasks/task_clear_buffer.h"
#include "tasks/task_raytrace.h"
#include "tasks/task_material_sort_clear_total_materials_count.h"
#include "tasks/task_material_sort_count_materials_per_chunk.h"
#include "tasks/task_material_sort_compute_prefix_sums_up_sweep.h"
#include "tasks/task_material_sort_compute_prefix_sums_zero_out.h"
#include "tasks/task_material_sort_compute_prefix_sums_down_sweep.h"
#include "tasks/task_material_sort_compute_offsets_per_material.h"
#include "tasks/task_material_sort_sort_materials.h"
#include "tasks/task_master_shader.h"
#include "tasks/task_miss.h"
#include "tasks/task_accumulate.h"
#include "tasks/task_passthrough.h"
#include "tasks/task_advance_render_count.h"

#include "renderer_options.h"

#include <string>
#include <vector>

class FRender
{
public:
    FRender(uint32_t WidthIn = 1920, uint32_t HeightIn = 1080);
	~FRender();

    void RegisterExternalOutputs(std::vector<ImagePtr> OutputImagesIn, const std::vector<FSynchronizationPoint>& ExternalImageIsReadyIn);
    int Init();
    int Cleanup();
    int SetSize(uint32_t WidthIn, uint32_t HeightIn);
	void SetRenderTarget(EOutputType OutputType);

    ECS::FEntity CreateCamera();
    ECS::FEntity CreateFramebuffer(int WidthIn, int HeightIn, const std::string& DebugName = "");
	void DestroyFramebuffer(ECS::FEntity Framebuffer);
    ECS::FEntity CreateFramebufferFromExternalImage(ImagePtr ImageIn, const std::string& DebugName = "");
	ECS::FEntity CreateColorAttachment(int WidthIn, int HeightIn, const std::string& DebugName = "");
    void SetActiveCamera(ECS::FEntity Camera);
    void SaveFramebufferPng(ECS::FEntity Framebuffer, const std::string& Filename = "");
	void SaveFramebufferExr(ECS::FEntity Framebuffer, const std::string& Filename = "");
	void SaveOutputPng(EOutputType OutputType, const std::string& Filename = "");
	void SaveOutputExr(EOutputType OutputType, const std::string& Filename = "");
	void PrintScreenPng(const std::string& Filename = "");
	void PrintScreenExr(const std::string& Filename = "");

	void AddExternalTaskAfterRender(std::shared_ptr<FExecutableTask> Task);
	FSynchronizationPoint Render();
    FSynchronizationPoint Render(uint32_t OutputImageIndex);
    int Update();
	void WaitIdle();
	void Wait(FSynchronizationPoint& SynchronizationPoint);

	/// Materials creation
    ECS::FEntity CreateTexture(const std::string& FilePath);
	ECS::FEntity CreateEmptyMaterial();
    ECS::FEntity CreateMaterial();
	ECS::FEntity CreateDiffuseMaterial(const FVector3& BaseColor);
	ECS::FEntity CreateReflectiveMaterial(const FVector3& BaseColor);
	ECS::FEntity CreateRefractiveMaterial(const FVector3& BaseColor);
    ECS::FEntity ShapeSetMaterial(ECS::FEntity Shape, ECS::FEntity Material);

	/// Materials manipulation
	/// Albedo
	static void MaterialSetBaseColorWeight(ECS::FEntity MaterialEntity, float Weight);
	static void MaterialSetBaseColorWeight(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);
	static void MaterialSetBaseColor(ECS::FEntity MaterialEntity, const FVector3& Color);
	static void MaterialSetBaseColor(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);
	static void MaterialSetDiffuseRoughness(ECS::FEntity MaterialEntity, float Roughness);
	static void MaterialSetDiffuseRoughness(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);
	static void MaterialSetMetalness(ECS::FEntity MaterialEntity, float Metalness);
	static void MaterialSetMetalness(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);
	static void MaterialSetAlbedoNormal(ECS::FEntity MaterialEntity, FVector3 Normal);
	static void MaterialSetAlbedoNormal(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);
	/// Specular
	static void MaterialSetSpecularWeight(ECS::FEntity MaterialEntity, float Weight);
	static void MaterialSetSpecularWeight(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);
	static void MaterialSetSpecularColor(ECS::FEntity MaterialEntity, const FVector3& Color);
	static void MaterialSetSpecularColor(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);
	static void MaterialSetSpecularRoughness(ECS::FEntity MaterialEntity, float Roughness);
	static void MaterialSetSpecularRoughness(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);
	static void MaterialSetSpecularIOR(ECS::FEntity MaterialEntity, float SpecularIOR);
	static void MaterialSetSpecularIOR(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);
	static void MaterialSetSpecularAnisotropy(ECS::FEntity MaterialEntity, float Anisotropy);
	static void MaterialSetSpecularAnisotropy(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);
	static void MaterialSetSpecularRotation(ECS::FEntity MaterialEntity, float Rotation);
	static void MaterialSetSpecularRotation(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);
	/// Transmission
	static void MaterialSetTransmissionWeight(ECS::FEntity MaterialEntity, float Weight);
	static void MaterialSetTransmissionWeight(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);
	static void MaterialSetTransmissionColor(ECS::FEntity MaterialEntity, const FVector3& Color);
	static void MaterialSetTransmissionColor(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);
	static void MaterialSetTransmissionDepth(ECS::FEntity MaterialEntity, float Depth);
	static void MaterialSetTransmissionDepth(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);
	static void MaterialSetTransmissionScatter(ECS::FEntity MaterialEntity, const FVector3& Color);
	static void MaterialSetTransmissionScatter(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);
	static void MaterialSetTransmissionAnisotropy(ECS::FEntity MaterialEntity, float Anisotropy);
	static void MaterialSetTransmissionAnisotropy(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);
	static void MaterialSetTransmissionDispersion(ECS::FEntity MaterialEntity, float Dispersion);
	static void MaterialSetTransmissionDispersion(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);
	static void MaterialSetTransmissionRoughness(ECS::FEntity MaterialEntity, float Roughness);
	static void MaterialSetTransmissionRoughness(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);
	/// Subsurface
	static void MaterialSetSubsurfaceWeight(ECS::FEntity MaterialEntity, float Weight);
	static void MaterialSetSubsurfaceWeight(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);
	static void MaterialSetSubsurfaceColor(ECS::FEntity MaterialEntity, const FVector3& Color);
	static void MaterialSetSubsurfaceColor(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);
	static void MaterialSetSubsurfaceRadius(ECS::FEntity MaterialEntity, const FVector3& Color);
	static void MaterialSetSubsurfaceRadius(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);
	static void MaterialSetSubsurfaceScale(ECS::FEntity MaterialEntity, float Scale);
	static void MaterialSetSubsurfaceScale(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);
	static void MaterialSetSubsurfaceAnisotropy(ECS::FEntity MaterialEntity, float Anisotropy);
	static void MaterialSetSubsurfaceAnisotropy(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);
	/// Sheen
	static void MaterialSetSheenWeight(ECS::FEntity MaterialEntity, float Weight);
	static void MaterialSetSheenWeight(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);
	static void MaterialSetSheenColor(ECS::FEntity MaterialEntity, const FVector3& Color);
	static void MaterialSetSheenColor(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);
	static void MaterialSetSheenRoughness(ECS::FEntity MaterialEntity, float Roughness);
	static void MaterialSetSheenRoughness(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);
	/// Coat
	static void MaterialSetCoatWeight(ECS::FEntity MaterialEntity, float Weight);
	static void MaterialSetCoatWeight(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);
	static void MaterialSetCoatColor(ECS::FEntity MaterialEntity, const FVector3& Color);
	static void MaterialSetCoatColor(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);
	static void MaterialSetCoatRoughness(ECS::FEntity MaterialEntity, float Roughness);
	static void MaterialSetCoatRoughness(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);
	static void MaterialSetCoatAnisotropy(ECS::FEntity MaterialEntity, float Anisotropy);
	static void MaterialSetCoatAnisotropy(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);
	static void MaterialSetCoatRotation(ECS::FEntity MaterialEntity, float Rotation);
	static void MaterialSetCoatRotation(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);
	static void MaterialSetCoatIOR(ECS::FEntity MaterialEntity, float CoatIOR);
	static void MaterialSetCoatIOR(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);
	static void MaterialSetCoatNormal(ECS::FEntity MaterialEntity, FVector3 Normal);
	static void MaterialSetCoatNormal(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);
	static void MaterialSetCoatAffectColor(ECS::FEntity MaterialEntity, float Color);
	static void MaterialSetCoatAffectColor(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);
	static void MaterialSetCoatAffectRoughness(ECS::FEntity MaterialEntity, float Roughness);
	static void MaterialSetCoatAffectRoughness(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);
	/// Thin film
	static void MaterialSetThinFilmThickness(ECS::FEntity MaterialEntity, float Thickness);
	static void MaterialSetThinFilmThickness(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);
	static void MaterialSetThinFilmIOR(ECS::FEntity MaterialEntity, float ThinFilmIOR);
	static void MaterialSetThinFilmIOR(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);
	/// Emission
	static void MaterialSetEmissionWeight(ECS::FEntity MaterialEntity, float Weight);
	static void MaterialSetEmissionWeight(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);
	static void MaterialSetEmissionColor(ECS::FEntity MaterialEntity, const FVector3& Color);
	static void MaterialSetEmissionColor(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);
	/// Opacity
	static void MaterialSetOpacity(ECS::FEntity MaterialEntity, const FVector3& Color);
	static void MaterialSetOpacity(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);
	/// Thin walled
	static void MaterialSetThinWalled(ECS::FEntity MaterialEntity, bool ThinWalled);
	static void MaterialSetThinWalled(ECS::FEntity MaterialEntity, ECS::FEntity TextureEntity);

	/// Shapes
    ECS::FEntity CreatePlane(const FVector2& Size);
    ECS::FEntity CreatePyramid();
    ECS::FEntity CreateCube();
    ECS::FEntity CreateIcosahedronSphere(float Radius, int LevelOfComplexity, bool bJagged);
	ECS::FEntity CreateUVSphere(uint32_t LongitudeCount, uint32_t LatitudeCount, float Radius);
    ECS::FEntity CreateModel(const std::string& Path);

    ECS::FEntity CreateInstance(ECS::FEntity BaseModel, const FVector3& Position, const FVector3& Direction = {0, 0, 1}, const FVector3& Up = {0, 1 ,0}, const FVector3& Scale = {1, 1 ,1});
	void SetInstancePosition(ECS::FEntity Instance, const FVector3& Position);
	FVector3 GetInstancePosition(ECS::FEntity Instance);

	/// Lights
	ECS::FEntity CreateDirectionalLight(const FVector3& Direction, const FVector3& Color, float Intensity);
	ECS::FEntity CreatePointLight(const FVector3& Position, const FVector3& Color, float Intensity);
	ECS::FEntity CreateSpotLight(const FVector3& Position, const FVector3& Direction, const FVector3& Color, float Intensity, float OuterAngle, float InnerAngle);
	void SetLightPosition(ECS::FEntity Light, const FVector3& Position);
	FVector3 GetLightPosition(ECS::FEntity Light);
    int SetIBL(const std::string& Path);

	void GetAllTimings(std::vector<std::string>& Names, std::vector<std::vector<float>>& Timings, float& FrameTime, uint32_t FrameIndex);

    bool bWasResized = false;

    uint32_t MaxFramesInFlight = 2;
    uint32_t RenderFrameIndex = 0;
	uint32_t Counter = 0;

    ECS::FEntity ActiveCamera;

	std::shared_ptr<FUpdateTLASTask> UpdateTLASTask = nullptr;
	std::shared_ptr<FClearBufferTask> ResetRenderIterations = nullptr;
    std::shared_ptr<FClearImageTask> ClearImageTask = nullptr;
	std::shared_ptr<FClearBufferTask> ClearCumulativeMaterialColorBuffer = nullptr;
    std::shared_ptr<FGenerateInitialRays> GenerateRaysTask = nullptr;
	std::shared_ptr<FResetActiveRayCountTask> ResetActiveRayCountTask = nullptr;
	std::shared_ptr<FCopyBufferTask> CopyNormalAOVBuffer = nullptr;
	std::shared_ptr<FClearBufferTask> ClearBuffersEachFrameTask = nullptr;
	std::shared_ptr<FClearBufferTask> ClearBuffersEachBounceTask = nullptr;
    std::shared_ptr<FRaytraceTask> RayTraceTask = nullptr;
    std::shared_ptr<FClearTotalMaterialsCountTask> ClearTotalMaterialsCountTask = nullptr;
    std::shared_ptr<FCountMaterialsPerChunkTask> CountMaterialsPerChunkTask = nullptr;
    std::shared_ptr<FComputePrefixSumsUpSweepTask> ComputePrefixSumsUpSweepTask = nullptr;
    std::shared_ptr<FComputePrefixSumsZeroOutTask> ComputePrefixSumsZeroOutTask = nullptr;
    std::shared_ptr<FComputePrefixSumsDownSweepTask> ComputePrefixSumsDownSweepTask = nullptr;
    std::shared_ptr<FComputeOffsetsPerMaterialTask> ComputeOffsetsPerMaterialTask = nullptr;
    std::shared_ptr<FSortMaterialsTask> SortMaterialsTask = nullptr;
	std::shared_ptr<FMasterShader> MasterShader = nullptr;
    std::shared_ptr<FMissTask> MissTask = nullptr;
    std::shared_ptr<FAccumulateTask> AccumulateTask = nullptr;
    std::shared_ptr<FPassthroughTask> PassthroughTask = nullptr;
	std::shared_ptr<FAdvanceRenderCount> AdvanceRenderCountTask = nullptr;
	std::vector<std::shared_ptr<FExecutableTask>> ExternalTasks;

    std::vector<FSynchronizationPoint> ImageAvailable;
    std::vector<VkFence> ImagesInFlight;

	std::vector<FSynchronizationPoint> ExternalImageAvailable;
    std::vector<ECS::FEntity> OutputFramebuffers;

	std::unordered_map<EOutputType, std::string> OutputToFramebufferNameMap;

private:
    ECS::FEntity CreateEmptyModel();
	void AllocateDependentResources();
	void AllocateIndependentResources();
	void FreeDependentResources();
	void FreeIndependentResources();

	struct FBufferDescription
	{
		std::string Name;
		VkDeviceSize Size;
		VkBufferUsageFlags Flags;
	};

	struct FImageDescription
	{
		std::string Name;
		uint32_t Width;
		uint32_t Height;
		uint32_t Format;
	};

	void CreateAndRegisterBufferShortcut(const std::vector<FBufferDescription>& BufferDescriptions);
	void CreateRegisterAndTransitionImageShortcut(const std::vector<FImageDescription>& ImageDescriptions);

    uint32_t Width;
    uint32_t Height;
	uint32_t RecursionDepth = 7;
	uint32_t DiffuseRecursionDepth = 4;
	uint32_t ReflectionRecursionDepth = 4;
	uint32_t RefractionRecursionDepth = 7;
	bool bAnyUpdate = false;
	std::chrono::time_point<std::chrono::steady_clock> Time;
	std::chrono::time_point<std::chrono::steady_clock> PreviousTime;
};