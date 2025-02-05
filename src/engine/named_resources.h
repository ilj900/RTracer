#pragma once
/// All named buffers
constexpr const char* THROUGHPUT_BUFFER = "ThroughputBuffer";
constexpr const char* INITIAL_RAYS_BUFFER = "InitialRaysBuffer";
constexpr const char* HITS_BUFFER = "HitsBuffer";
constexpr const char* PIXEL_INDEX_BUFFER = "PixelIndexBuffer";
constexpr const char* NORMAL_AOV_BUFFER = "NormalAOVBuffer";
constexpr const char* HISTORY_NORMAL_AOV_BUFFER = "PreviousBounceNormalAOVBuffer";
constexpr const char* UV_AOV_BUFFER = "UVAOVBuffer";
constexpr const char* WORLD_SPACE_POSITION_AOV_BUFFER = "WorldSpacePositionAOVBuffer";
constexpr const char* SAMPLED_IBL_BUFFER = "SampledIBLBuffer";
constexpr const char* SAMPLED_POINT_LIGHT_BUFFER = "SampledPointLightBuffer";
constexpr const char* SAMPLED_DIRECTIONAL_LIGHT_BUFFER = "SampledDirectionalLightBuffer";
constexpr const char* SAMPLED_SPOT_LIGHT_BUFFER = "SampledSpotLightBuffer";
constexpr const char* TRANSFORM_INDEX_BUFFER = "TransformIndexBuffer";
constexpr const char* ACTIVE_RAY_COUNT_BUFFER = "ActiveRayCountBuffer";
constexpr const char* CUMULATIVE_MATERIAL_COLOR_BUFFER = "CumulativeMaterialColorBuffer";
constexpr const char* DEBUG_LAYER_BUFFER = "DebugLayerBuffer";
constexpr const char* RENDER_ITERATION_BUFFER = "RenderIterationBuffer";
constexpr const char* COUNTED_MATERIALS_PER_CHUNK_BUFFER = "CountedMaterialsPerChunkBuffer";
constexpr const char* TOTAL_COUNTED_MATERIALS_BUFFER = "TotalCountedMaterialsBuffer";
constexpr const char* MATERIALS_OFFSETS_PER_MATERIAL_BUFFER = "MaterialsOffsetsPerMaterialBuffer";
constexpr const char* UTILITY_INFO_POINT_LIGHT_BUFFER = "UtilityInfoPointLight";
constexpr const char* UTILITY_INFO_DIRECTIONAL_LIGHT_BUFFER = "UtilityInfoDirectionalLight";
constexpr const char* UTILITY_INFO_SPOT_LIGHT_BUFFER = "UtilityInfoSpotLight";
