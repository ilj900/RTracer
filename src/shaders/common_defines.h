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

// Material fields
// Base
#define MATERIAL_BASE_WEIGHT                        0
#define MATERIAL_BASE_COLOR                         1
#define MATERIAL_BASE_METALNESS                     2
#define MATERIAL_BASE_DIFFUSE_ROUGHNESS             3
// Specular
#define MATERIAL_SPECULAR_WEIGHT                    4
#define MATERIAL_SPECULAR_COLOR                     5
#define MATERIAL_SPECULAR_ROUGHNESS                 6
#define MATERIAL_SPECULAR_ROUGHNESS_ANISOTROPY      7
#define MATERIAL_SPECULAR_IOR                       8
// Transmission
#define MATERIAL_TRANSMISSION_WEIGHT                9
#define MATERIAL_TRANSMISSION_COLOR                 10
#define MATERIAL_TRANSMISSION_DEPTH                 11
#define MATERIAL_TRANSMISSION_SCATTER               12
#define MATERIAL_TRANSMISSION_SCATTER_ANISOTROPY    13
#define MATERIAL_TRANSMISSION_DISPERSION_SCALE      14
#define MATERIAL_TRANSMISSION_DISPERSION_ABBE_NUMBER 15
// Subsurface
#define MATERIAL_SUBSURFACE_WEIGHT                  16
#define MATERIAL_SUBSURFACE_COLOR                   17
#define MATERIAL_SUBSURFACE_RADIUS                  18
#define MATERIAL_SUBSURFACE_RADIUS_SCALE            19
#define MATERIAL_SUBSURFACE_SCATTER_ANISOTROPY      20
// Coat
#define MATERIAL_COAT_WEIGHT                        21
#define MATERIAL_COAT_COLOR                         22
#define MATERIAL_COAT_ROUGHNESS                     23
#define MATERIAL_COAT_ROUGHNESS_ANISOTROPY          24
#define MATERIAL_COAT_IOR                           25
#define MATERIAL_COAT_DARKENING                     26
// Fuzz
#define MATERIAL_FUZZ_WEIGHT                        27
#define MATERIAL_FUZZ_COLOR                         28
#define MATERIAL_FUZZ_ROUGHNESS                     29
// Emission
#define MATERIAL_EMISSION_LUMINANCE                 30
#define MATERIAL_EMISSION_COLOR                     31
// Thin film
#define MATERIAL_THIN_FILM_WEIGHT                   32
#define MATERIAL_THIN_FILM_THICKNESS                33
#define MATERIAL_THIN_FILM_IOR                      34
// Geometry
#define MATERIAL_GEOMETRY_OPACITY                   35
#define MATERIAL_GEOMETRY_THIN_WALLED               36
#define MATERIAL_GEOMETRY_NORMAL                    37
#define MATERIAL_GEOMETRY_TANGENT                   38
#define MATERIAL_GEOMETRY_COAT_NORMAL               39
#define MATERIAL_GEOMETRY_COAT_TANGENT              40

/// Sampling strategies
#define SAMPLE_UNIFORM															0u
#define SAMPLE_IMPORTANCE														1u
#define SAMPLE_BXDF																2u

/// Light types
#define POINT_LIGHT																0u
#define DIRECTIONAL_LIGHT														1u
#define SPOT_LIGHT																3u
#define AREA_LIGHT																3u

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
#define AOV_COLOR                                       						0u
#define AOV_SHADING_NORMAL                              						1u
#define AOV_GEOMETRIC_NORMAL                            						2u
#define AOV_UV_COORDS                                   						3u
#define AOV_WORLD_SPACE_POSITION                        						4u
#define AOV_OPACITY                                     						5u
#define AOV_DEPTH                                       						6u
// Base
#define AOV_MATERIAL_BASE_WEIGHT                        						7u
#define AOV_MATERIAL_BASE_COLOR                         						8u
#define AOV_MATERIAL_BASE_METALNESS                     						9u
#define AOV_MATERIAL_BASE_DIFFUSE_ROUGHNESS             						10u
// Specular
#define AOV_MATERIAL_SPECULAR_WEIGHT                    						11u
#define AOV_MATERIAL_SPECULAR_COLOR                     						12u
#define AOV_MATERIAL_SPECULAR_ROUGHNESS                 						13u
#define AOV_MATERIAL_SPECULAR_ROUGHNESS_ANISOTROPY      						14u
#define AOV_MATERIAL_SPECULAR_IOR                       						15u
// Transmission
#define AOV_MATERIAL_TRANSMISSION_WEIGHT                						16u
#define AOV_MATERIAL_TRANSMISSION_COLOR                 						17u
#define AOV_MATERIAL_TRANSMISSION_DEPTH                 						18u
#define AOV_MATERIAL_TRANSMISSION_SCATTER               						19u
#define AOV_MATERIAL_TRANSMISSION_SCATTER_ANISOTROPY    						20u
#define AOV_MATERIAL_TRANSMISSION_DISPERSION_SCALE      						21u
#define AOV_MATERIAL_TRANSMISSION_DISPERSION_ABBE_NUMBER						22u
// Subsurface
#define AOV_MATERIAL_SUBSURFACE_WEIGHT                      					23u
#define AOV_MATERIAL_SUBSURFACE_COLOR                       					24u
#define AOV_MATERIAL_SUBSURFACE_RADIUS                      					25u
#define AOV_MATERIAL_SUBSURFACE_RADIUS_SCALE                					26u
#define AOV_MATERIAL_SUBSURFACE_SCATTER_ANISOTROPY          					27u
// Coat
#define AOV_MATERIAL_COAT_WEIGHT                            					28u
#define AOV_MATERIAL_COAT_COLOR                             					29u
#define AOV_MATERIAL_COAT_ROUGHNESS                         					30u
#define AOV_MATERIAL_COAT_ROUGHNESS_ANISOTROPY              					31u
#define AOV_MATERIAL_COAT_IOR                               					32u
#define AOV_MATERIAL_COAT_DARKENING                         					33u
// Fuzz
#define AOV_MATERIAL_FUZZ_WEIGHT                            					34u
#define AOV_MATERIAL_FUZZ_COLOR                             					35u
#define AOV_MATERIAL_FUZZ_ROUGHNESS                         					36u
// Emission
#define AOV_MATERIAL_EMISSION_LUMINANCE                     					37u
#define AOV_MATERIAL_EMISSION_COLOR                         					38u
// Thin film
#define AOV_MATERIAL_THIN_FILM_WEIGHT                       					39u
#define AOV_MATERIAL_THIN_FILM_THICKNESS                    					40u
#define AOV_MATERIAL_THIN_FILM_IOR                          					41u
// Geometry
#define AOV_MATERIAL_GEOMETRY_OPACITY                       					42u
#define AOV_MATERIAL_GEOMETRY_THIN_WALLED                   					43u
#define AOV_MATERIAL_GEOMETRY_NORMAL                        					44u
#define AOV_MATERIAL_GEOMETRY_TANGENT                       					45u
#define AOV_MATERIAL_GEOMETRY_COAT_NORMAL                   					46u
#define AOV_MATERIAL_GEOMETRY_COAT_TANGENT                  					47u
// Misc
#define AOV_LUMINANCE                                       					48u
#define AOV_BOUNCE_HEATMAP                                  					49u
#define AOV_RENDERABLE_INDEX                                					50u
#define AOV_PRIMITIVE_INDEX                                 					51u
#define AOV_MATERIAL_INDEX                                  					52u
#define AOV_DEBUG_LAYER_0                                   					53u
#define AOV_DEBUG_LAYER_1                                   					54u
#define AOV_DEBUG_LAYER_2                                   					55u
#define AOV_DEBUG_LAYER_3                                   					56u
#define AOV_DEBUG_LAYER_4                                   					57u
#define AOV_DEBUG_LAYER_5                                   					58u
#define AOV_DEBUG_LAYER_6                                   					59u
#define AOV_DEBUG_LAYER_7                                   					60u
#define AOV_MAX                                             					61u

/// Task accumulate descriptor set layout defines
#define ACCUMULATE_PER_FRAME_LAYOUT_INDEX 										0u

#define INCOMING_IMAGE_TO_SAMPLE 												0u
#define ACCUMULATE_IMAGE_INDEX 													1u
#define ESTIMATED_IMAGE_INDEX 													2u
#define UTILITY_BUFFER_INDEX													3u

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
#define MASTER_SHADER_THROUGHPUT_BUFFER											14u
#define MASTER_SHADER_UTILITY_BUFFER_INDEX 										15u
#define MASTER_SHADER_COLOR_AOV_IMAGE_INDEX 									16u
#define MASTER_SHADER_AOV_RGBA32F_IMAGE_INDEX 									17u
#define MASTER_SHADER_MATERIAL_SYSTEM_DATA_BUFFER_INDEX							18u

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
#define COMPUTE_MISS_CUMULATIVE_MATERIAL_COLOR_BUFFER_INDEX						5u
#define COMPUTE_MISS_THROUGHPUT_BUFFER 											6u
#define COMPUTE_MISS_OUTPUT_IMAGE_INDEX 										7u
#define COMPUTE_MISS_AOV_RGBA32F_IMAGE_INDEX 									8u
#define COMPUTE_MISS_UTILITY_BUFFER_INDEX										9u

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
