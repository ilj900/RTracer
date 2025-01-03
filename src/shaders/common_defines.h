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

#define MAX_TEXTURES 															512
#define TOTAL_MATERIALS 														128 /// 126 Materials + IBL + material for inactive ray
#define BASIC_CHUNK_SIZE 														256

#define RAY_DATA_RAY_MISSED 													1u
#define LAST_BOUNCE 															4

#define LIGHT_TYPE_NONE 														0u
#define LIGHT_TYPE_POINT_LIGHT 													1u
#define LIGHT_TYPE_DIRECTIONAL_LIGHT 											2u
#define LIGHT_TYPE_SPOT_LIGHT 													3u

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

/// Task reset active ray count descriptor set layout defines
#define RESET_ACTIVE_RAY_COUNT_LAYOUT_INDEX 									0u

#define ACTIVE_RAY_COUNT_BUFFER 												0u

/// Task compute shading data descriptor set layout defines
#define COMPUTE_SHADING_DATA_LAYOUT_INDEX 										0u

#define COMPUTE_SHADING_DATA_RENDERABLES_BUFFER_INDEX							0u
#define COMPUTE_SHADING_DATA_TRANSFORMS_BUFFER_INDEX							1u
#define COMPUTE_SHADING_DATA_HITS_BUFFER_INDEX 									2u
#define COMPUTE_SHADING_DATA_RAYS_BUFFER_INDEX 									3u
#define COMPUTE_SHADING_DATA_TRANSFORM_INDEX_BUFFER_INDEX						4u
#define COMPUTE_SHADING_DATA_RAY_INDEX_MAP_INDEX 								5u
#define COMPUTE_SHADING_DATA_NORMAL_AOV_DATA_INDEX 								6u
#define COMPUTE_SHADING_DATA_UV_AOV_DATA_INDEX 									7u
#define COMPUTE_SHADING_DATA_WORLD_SPACE_POSITION_AOV_DATA_INDEX 				8u

/// Task sample IBL set layout defines
#define RAYTRACE_SAMPLE_IBL_LAYOUT_INDEX 										0u

#define RAYTRACE_SAMPLE_IBL_TLAS_INDEX 											0u
#define RAYTRACE_SAMPLE_IBL_IMPORTANCE_BUFFER_INDEX 							1u
#define RAYTRACE_SAMPLE_IBL_IMAGE_SAMPLER_INDEX 								2u
#define RAYTRACE_SAMPLE_IBL_WEIGHTS_BUFFER_INDEX 								3u
#define RAYTRACE_SAMPLE_IBL_NORMAL_AOV_BUFFER_INDEX 							4u
#define RAYTRACE_SAMPLE_IBL_WORLD_SPACE_POSITION_AOV_BUFFER_INDEX 				5u
#define RAYTRACE_SAMPLE_IBL_TRANSFORM_INDEX_BUFFER_INDEX						6u
#define RAYTRACE_SAMPLE_IBL_DEVICE_TRANSFORM_BUFFER_INDEX						7u
#define RAYTRACE_SAMPLE_IBL_SAMPLED_IBL_BUFFER_INDEX 							8u
#define RAYTRACE_SAMPLE_IBL_PIXEL_INDEX_BUFFER 									9u
#define RAYTRACE_SAMPLE_IBL_RENDER_ITERATION_BUFFER_INDEX 						10u

/// Task shade descriptor set layout defines
#define COMPUTE_SHADE_LAYOUT_INDEX 												0u

#define COMPUTE_SHADE_OUTPUT_IMAGE_INDEX 										0u
#define COMPUTE_SHADE_RAYS_BUFFER_INDEX 										1u
#define COMPUTE_SHADE_TEXTURE_SAMPLER 											2u
#define COMPUTE_SHADE_TEXTURE_ARRAY 											3u
#define COMPUTE_SHADE_IBL_SAMPLING_RESULT_INDEX									4u
#define COMPUTE_SHADE_MATERIAL_INDEX_MAP 										5u
#define COMPUTE_SHADE_MATERIAL_INDEX_AOV_MAP 									6u
#define COMPUTE_SHADE_MATERIALS_OFFSETS 										7u
#define COMPUTE_SHADE_THROUGHPUT_BUFFER 										8u
#define COMPUTE_SHADE_RENDER_ITERATION_BUFFER 									9u
#define COMPUTE_SHADE_NORMAL_AOV_DATA_BUFFER 									10u
#define COMPUTE_SHADE_UV_AOV_DATA_BUFFER 										11u
#define COMPUTE_SHADE_WORLD_SPACE_POSITION_AOV_DATA_BUFFER 						12u

/// Task miss descriptor set layout defines
#define COMPUTE_MISS_LAYOUT_INDEX 												0u

#define COMPUTE_MISS_OUTPUT_IMAGE_INDEX 										0u
#define COMPUTE_MISS_RAYS_BUFFER_INDEX 											1u
#define COMPUTE_MISS_IBL_IMAGE_SAMPLER_LINEAR_INDEX 							2u
#define COMPUTE_MISS_IBL_IMAGE_SAMPLER_NEAREST_INDEX 							3u
#define COMPUTE_MISS_MATERIAL_INDEX_MAP 										4u
#define COMPUTE_MISS_MATERIAL_INDEX_AOV_MAP 									5u
#define COMPUTE_MISS_MATERIALS_OFFSETS 											6u
#define COMPUTE_MISS_THROUGHPUT_BUFFER 											7u
#define COMPUTE_MISS_NORMAL_AOV_IMAGE_INDEX 									8u
#define COMPUTE_MISS_UV_AOV_IMAGE_INDEX 										9u

/// Task aov pass descriptor set layout defines
#define AOV_PASS_LAYOUT_INDEX 													0u

#define AOV_PASS_NORMAL_BUFFER 													0u
#define AOV_PASS_NORMAL_AOV_IMAGE_INDEX 										1u
#define AOV_PASS_UV_BUFFER 														2u
#define AOV_PASS_UV_AOV_IMAGE_INDEX 											3u
#define AOV_PASS_WORLD_SPACE_POSITION_BUFFER 									4u
#define AOV_PASS_WORLD_SPACE_POSITION_AOV_IMAGE_INDEX							5u
#define AOV_PASS_DEBUG_LAYER_BUFFER 											6u
#define AOV_PASS_DEBUG_LAYER_IMAGE_INDEX										7u

/// Task advance render count descriptor set layout defines
#define ADVANCE_RENDER_COUNT_LAYOUT_INDEX 										0u

#define ADVANCE_RENDER_COUNT_RENDER_ITERATION_BUFFER 							0u

/// Task generate rays descriptor set layout defines
#define GENERATE_RAYS_LAYOUT_INDEX 												0u

#define CAMERA_RAYS_BUFFER 														0u
#define CAMERA_POSITION_BUFFER 													1u
#define GENERATE_RAYS_PIXEL_INDEX_BUFFER 										2u
#define GENERATE_RAYS_RENDER_ITERATION_BUFFER 									3u
#define GENERATE_RAYS_THROUGHPUT_BUFFER 										4u

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