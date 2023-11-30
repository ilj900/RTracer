#ifndef COMMON_DEFINES_H
#define COMMON_DEFINES_H

#define MAX_TEXTURES 512

#define LIGHT_TYPE_NONE 0u
#define LIGHT_TYPE_POINT_LIGHT 1u
#define LIGHT_TYPE_DIRECTIONAL_LIGHT 2u
#define LIGHT_TYPE_SPOT_LIGHT 3u

/// Task accumulate descriptor set layout defines
#define ACCUMULATE_PER_FRAME_LAYOUT_INDEX 0u

#define INCOMING_IMAGE_TO_SAMPLE 0u
#define ACCUMULATE_IMAGE_INDEX 1u
#define ESTIMATED_IMAGE_INDEX 2u

/// Task passthrough descriptor set layout defines
#define PASSTHROUGH_PER_FRAME_LAYOUT_INDEX 0u

#define PASSTHROUGH_TEXTURE_SAMPLER_LAYOUT_INDEX 0u

/// Task raytrace descriptor set layout defines
#define RAYTRACE_LAYOUT_INDEX 0u

#define RAYTRACE_TLAS_LAYOUT_INDEX 0u
#define RAYTRACE_FINAL_IMAGE_INDEX 1u
#define RAYTRACE_RAYS_DATA_BUFFER 2u
#define RAYTRACE_RENDERABLE_BUFFER_INDEX 3u
#define RAYTRACE_MATERIAL_BUFFER_INDEX 4u
#define RAYTRACE_LIGHT_BUFFER_INDEX 5u
#define RAYTRACE_IBL_IMAGE_INDEX 6u
#define RAYTRACE_TEXTURE_SAMPLER 7u
#define RAYTRACE_TEXTURE_ARRAY 8u

/// Task clear image descriptor set layout defines
#define CLEAR_IMAGE_LAYOUT_INDEX 0u

#define IMAGE_TO_CLEAR 0u

/// Task generate rays descriptor set layout defines
#define GENERATE_RAYS_LAYOUT_INDEX 0

#define CAMERA_RAYS_BUFFER 0
#define CAMERA_POSITION_BUFFER 1

#endif // COMMON_DEFINES_H