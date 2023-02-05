glslc.exe ..\shaders\triangle.vert -o ..\shaders\triangle_vert.spv
glslc.exe ..\shaders\triangle.frag -o ..\shaders\triangle_frag.spv

glslc.exe ..\shaders\passthrough.vert -o ..\shaders\passthrough_vert.spv
glslc.exe ..\shaders\passthrough.frag -o ..\shaders\passthrough_frag.spv

glslc.exe --target-spv=spv1.6 ..\shaders\raytrace.rchit -o ..\shaders\ray_closest_hit.spv
glslc.exe --target-spv=spv1.6 ..\shaders\raytrace.rmiss -o ..\shaders\ray_miss.spv
glslc.exe --target-spv=spv1.6 ..\shaders\raytrace.rgen -o ..\shaders\ray_gen.spv
