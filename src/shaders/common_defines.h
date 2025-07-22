#ifndef COMMON_DEFINES_H
#define COMMON_DEFINES_H

#ifndef __cplusplus
#define UINT_MAX 4294967295
#endif

#ifndef M_PI
#define M_PI 3.141592653589
#endif
#ifndef M_2_PI
#define M_2_PI 6.28318530718
#endif
#ifndef M_PI_2
#define M_PI_2 1.57079632679
#endif
#ifndef M_PI_4
#define M_PI_4 0.78539816339
#endif
#ifndef M_INV_PI
#define M_INV_PI (1 / M_PI)
#endif

#ifndef UINT_MAX
	#define UINT_MAX 0xFFFFFFFF
#endif

#define MAX_TEXTURES 															512
#define TOTAL_MATERIALS 														128 /// 126 Materials + IBL + material for inactive ray
#define IBL_MATERIAL_INDEX														TOTAL_MATERIALS - 2
#define INACTIVE_MATERIAL_INDEX													TOTAL_MATERIALS - 1
#define BASIC_CHUNK_SIZE 														256

#define FLOAT_EPSILON															0.0001f

/// Ray flags
#define RAY_DATA_RAY_MISSED 													1u
#define DIFFUSE_LAYER 															1u << 1
#define SPECULAR_LAYER 															1u << 2
#define TRANSMISSION_LAYER 														1u << 3
#define SUBSURFACE_LAYER														1u << 4
#define SHEEN_LAYER 															1u << 5
#define COAT_LAYER 																1u << 6
#define EMISSION_LAYER															1u << 7
#define RAY_TYPE_MASK															0x0000FFFE


/// AOV Indices
#define AOV_COLOR																0u
#define AOV_SHADING_NORMAL														1u
#define AOV_GEOMETRIC_NORMAL													2u
#define AOV_UV_COORDS															3u
#define AOV_WORLD_SPACE_POSITION												4u
#define AOV_OPACITY																5u
#define AOV_DEPTH																6u
#define AOV_MATERIAL_ALBEDO_WEIGHT												7u
#define AOV_MATERIAL_ALBEDO														8u
#define AOV_MATERIAL_DIFFUSE_ROUGHNESS											9u
#define AOV_MATERIAL_METALNESS													10u
#define AOV_MATERIAL_NORMAL														11u
#define AOV_MATERIAL_SPECULAR_WEIGHT											12u
#define AOV_MATERIAL_SPECULAR_COLOR												13u
#define AOV_MATERIAL_SPECULAR_ROUGHNESS											14u
#define AOV_MATERIAL_SPECULAR_IOR												15u
#define AOV_MATERIAL_SPECULAR_ANISOTROPY										16u
#define AOV_MATERIAL_SPECULAR_ROTATION											17u
#define AOV_MATERIAL_TRANSMISSION_WEIGHT										18u
#define AOV_MATERIAL_TRANSMISSION_COLOR											19u
#define AOV_MATERIAL_TRANSMISSION_DEPTH											20u
#define AOV_MATERIAL_TRANSMISSION_SCATTER										21u
#define AOV_MATERIAL_TRANSMISSION_ANISOTROPY									22u
#define AOV_MATERIAL_TRANSMISSION_DISPERSION									23u
#define AOV_MATERIAL_TRANSMISSION_ROUGHNESS										24u
#define AOV_MATERIAL_SUBSURFACE_WEIGHT											25u
#define AOV_MATERIAL_SUBSURFACE_COLOR											26u
#define AOV_MATERIAL_SUBSURFACE_RADIUS											27u
#define AOV_MATERIAL_SUBSURFACE_SCALE											28u
#define AOV_MATERIAL_SUBSURFACE_ANISOTROPY										29u
#define AOV_MATERIAL_SHEEN_WEIGHT												30u
#define AOV_MATERIAL_SHEEN_COLOR												31u
#define AOV_MATERIAL_SHEEN_ROUGHNESS											32u
#define AOV_MATERIAL_COAT_WEIGHT												33u
#define AOV_MATERIAL_COAT_COLOR													34u
#define AOV_MATERIAL_COAT_ROUGHNESS												35u
#define AOV_MATERIAL_COAT_ANISOTROPY											36u
#define AOV_MATERIAL_COAT_ROTATION												37u
#define AOV_MATERIAL_COAT_IOR													38u
#define AOV_MATERIAL_COAT_NORMAL												39u
#define AOV_MATERIAL_COAT_AFFECT_COLOR											40u
#define AOV_MATERIAL_COAT_AFFECT_ROUGHNESS										41u
#define AOV_MATERIAL_THIN_FILM_THICKNESS										42u
#define AOV_MATERIAL_THIN_FILM_IOR												43u
#define AOV_MATERIAL_EMISSION_WEIGHT											44u
#define AOV_MATERIAL_COLOR														45u
#define AOV_MATERIAL_OPACITY													46u
#define AOV_MATERIAL_THIN_WALLED												47u
#define AOV_LUMINANCE															48u
#define AOV_RENDERABLE_INDEX													49u
#define AOV_PRIMITIVE_INDEX														50u
#define AOV_MATERIAL_INDEX														51u
#define AOV_DEBUG_LAYER_0														52u
#define AOV_DEBUG_LAYER_1														53u
#define AOV_DEBUG_LAYER_2														54u
#define AOV_DEBUG_LAYER_3														55u
#define AOV_DEBUG_LAYER_4														56u
#define AOV_DEBUG_LAYER_5														57u
#define AOV_DEBUG_LAYER_6														58u
#define AOV_DEBUG_LAYER_7														59u
#define AOV_MAX																	60u

/// Task accumulate descriptor set layout defines
#define ACCUMULATE_PER_FRAME_LAYOUT_INDEX 										0u

#define INCOMING_IMAGE_TO_SAMPLE 												0u
#define ACCUMULATE_IMAGE_INDEX 													1u
#define ESTIMATED_IMAGE_INDEX 													2u

/// Task passthrough descriptor set layout defines
#define PASSTHROUGH_PER_FRAME_LAYOUT_INDEX 										0u

#define PASSTHROUGH_TEXTURE_SAMPLER_LAYOUT_INDEX 								0u

/// Task raytrace descriptor set layout defines
#define RAYTRACE_LAYOUT_INDEX 													0u

#define RAYTRACE_TLAS_LAYOUT_INDEX 												0u
#define RAYTRACE_RAYS_DATA_BUFFER 												1u
#define RAYTRACE_PIXEL_INDEX_BUFFER 											2u
#define RAYTRACE_RENDERABLE_BUFFER_INDEX 										3u
#define RAYTRACE_HIT_BUFFER 													4u
#define RAYTRACE_MATERIAL_INDEX_BUFFER 											5u
#define RAYTRACE_CAMERA_POSITION_BUFFER 										6u
#define RAYTRACE_RENDER_ITERATION_BUFFER 										7u
#define RAYTRACE_THROUGHPUT_BUFFER 												8u

/// Task reset active ray count descriptor set layout defines
#define RESET_ACTIVE_RAY_COUNT_LAYOUT_INDEX 									0u

#define ACTIVE_RAY_COUNT_BUFFER_INDEX											0u

/// Master shader task descriptor set layout defines
#define MASTER_SHADER_LAYOUT_STATIC_INDEX										0u

/// Descriptors that are constant
#define MASTER_SHADER_TLAS_INDEX 												0u
#define MASTER_SHADER_TEXTURE_SAMPLER 											1u
#define MASTER_SHADER_FLOAT_TEXTURE_ARRAY 										2u
#define MASTER_SHADER_UINT_TEXTURE_ARRAY 										3u
#define MASTER_SHADER_INT_TEXTURE_ARRAY 										4u
#define MASTER_SHADER_IBL_IMPORTANCE_BUFFER_INDEX								5u
#define MASTER_SHADER_IBL_IMAGE_SAMPLER_INDEX									6u
#define MASTER_SHADER_IBL_WEIGHTS_BUFFER_INDEX									7u
#define MASTER_SHADER_RENDER_ITERATION_BUFFER_INDEX 							8u
#define MASTER_SHADER_RAYS_BUFFER_INDEX 										9u
#define MASTER_SHADER_HITS_BUFFER_INDEX 										10u
#define MASTER_SHADER_MATERIALS_OFFSETS 										11u
#define MASTER_SHADER_PIXEL_INDEX_BUFFER 										12u
#define MASTER_SHADER_CUMULATIVE_MATERIAL_COLOR_BUFFER_INDEX					13u
#define MASTER_SHADER_NORMAL_BUFFER												14u
#define MASTER_SHADER_THROUGHPUT_BUFFER											15u
#define MASTER_SHADER_UTILITY_BUFFER_INDEX 										16u
#define MASTER_SHADER_COLOR_AOV_IMAGE_INDEX 									17u
#define MASTER_SHADER_AOV_RGBA32F_IMAGE_INDEX 									18u

/// Descriptors that might be updated out of order (by double/triple buffering for example)
#define MASTER_SHADER_LAYOUT_INDEX_PER_FRAME									1u

#define MASTER_SHADER_DIRECTIONAL_LIGHTS_BUFFER_INDEX 							0u
#define MASTER_SHADER_DIRECTIONAL_LIGHTS_IMPORTANCE_BUFFER_INDEX 				1u
#define MASTER_SHADER_SPOT_LIGHTS_BUFFER_INDEX 									2u
#define MASTER_SHADER_SPOT_LIGHTS_IMPORTANCE_BUFFER_INDEX						3u
#define MASTER_SHADER_POINT_LIGHTS_BUFFER_INDEX 								4u
#define MASTER_SHADER_POINT_LIGHTS_IMPORTANCE_BUFFER_INDEX						5u
#define MASTER_SHADER_AREA_LIGHTS_BUFFER_INDEX                                  6u
#define MASTER_SHADER_AREA_LIGHTS_IMPORTANCE_BUFFER_INDEX                       7u
#define MASTER_SHADER_RENDERABLES_BUFFER_INDEX 									8u
#define MASTER_SHADER_TRANSFORMS_BUFFER_INDEX 									9u


/// Task miss descriptor set layout defines
#define COMPUTE_MISS_LAYOUT_INDEX 												0u

#define COMPUTE_MISS_RAYS_BUFFER_INDEX 											0u
#define COMPUTE_MISS_IBL_IMAGE_SAMPLER_LINEAR_INDEX 							1u
#define COMPUTE_MISS_MATERIAL_INDEX_MAP 										2u
#define COMPUTE_MISS_MATERIAL_INDEX_AOV_MAP 									3u
#define COMPUTE_MISS_MATERIALS_OFFSETS 											4u
#define COMPUTE_MISS_PREVIOUS_BOUNCE_NORMAL_AOV_DATA_BUFFER						5u
#define COMPUTE_MISS_CUMULATIVE_MATERIAL_COLOR_BUFFER_INDEX						6u
#define COMPUTE_MISS_THROUGHPUT_BUFFER 											7u
#define COMPUTE_MISS_OUTPUT_IMAGE_INDEX 										8u
#define COMPUTE_MISS_AOV_RGBA32F_IMAGE_INDEX 									9u
#define COMPUTE_MISS_UTILITY_BUFFER_INDEX										10u

/// Task advance render count descriptor set layout defines
#define ADVANCE_RENDER_COUNT_LAYOUT_INDEX 										0u

#define ADVANCE_RENDER_COUNT_RENDER_ITERATION_BUFFER 							0u

/// Task material sort count materials set layout defines
#define MATERIAL_SORT_COUNT_MATERIALS_PER_CHUNK_INDEX 							0u

#define MATERIAL_SORT_COUNT_MATERIALS_MATERIAL_INDICES_AOV_BUFFER 				0u
#define MATERIAL_SORT_COUNT_MATERIALS_MATERIAL_COUNT_BUFFER 					1u

/// Task material sort clear total materials count set layout defines
#define MATERIAL_SORT_CLEAR_TOTAL_MATERIALS_COUNT_LAYOUT_INDEX 					0u

#define MATERIAL_SORT_CLEAR_TOTAL_MATERIALS_COUNT_BUFFER 						0u

/// Task material sort compute offsets per material set layout defines
#define MATERIAL_SORT_COMPUTE_OFFSETS_PER_MATERIAL_LAYOUT_INDEX 				0u

#define MATERIAL_SORT_COMPUTE_OFFSETS_PER_MATERIAL_TOTAL_MATERIAL_COUNT_BUFFER 	0u
#define MATERIAL_SORT_COMPUTE_OFFSETS_PER_MATERIAL_MATERIAL_OFFSETS_BUFFER 		1u
#define MATERIAL_SORT_ACTIVE_RAY_COUNT_BUFFER 									2u

/// Task material sort sort materials set layout defines
#define MATERIAL_SORT_SORT_MATERIALS_LAYOUT_INDEX 								0u

#define MATERIAL_SORT_SORT_MATERIALS_MATERIALS_OFFSETS_PER_CHUNK_BUFFER 		0u
#define MATERIAL_SORT_SORT_MATERIALS_MATERIAL_OFFSETS_PER_MATERIAL_BUFFER 		1u
#define MATERIAL_SORT_SORT_MATERIALS_UNSORTED_MATERIALS_BUFFER 					2u
#define MATERIAL_SORT_SORT_MATERIALS_SORTED_MATERIALS_INDEX_MAP_BUFFER 			3u

/// Task material sort compute prefix sums us sweep set layout defines
#define MATERIAL_SORT_COMPUTE_PREFIX_SUMS_UP_SWEEP_LAYOUT_INDEX 				0u

#define MATERIAL_SORT_COMPUTE_PREFIX_SUMS_UP_SWEEP_BUFFER_A 					0u

/// Task material sort compute prefix sums zero out set layout defines
#define MATERIAL_SORT_COMPUTE_PREFIX_SUMS_ZERO_OUT_LAYOUT_INDEX 				0u

#define MATERIAL_SORT_COMPUTE_PREFIX_SUMS_ZERO_OUT_BUFFER_A 					0u
#define MATERIAL_SORT_TOTAL_MATERIAL_OFFSETS_BUFFER 							1u

/// Task material sort compute prefix sums down sweep set layout defines
#define MATERIAL_SORT_COMPUTE_PREFIX_SUMS_DOWN_SWEEP_LAYOUT_INDEX 				0u

#define MATERIAL_SORT_COMPUTE_PREFIX_SUMS_DOWN_SWEEP_BUFFER_A 					0u

#endif // COMMON_DEFINES_H