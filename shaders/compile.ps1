glslc.exe triangle.vert -o triangle_vert.spv --target-env=vulkan1.2
glslc.exe triangle.frag -o triangle_frag.spv --target-env=vulkan1.2
glslc.exe raytrace.rchit -o rayclosesthit.spv --target-env=vulkan1.2
glslc.exe raytrace.rgen -o raygen.spv --target-env=vulkan1.2
glslc.exe raytrace.rmiss -o raymiss.spv --target-env=vulkan1.2