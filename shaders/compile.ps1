glslc.exe F:\Work\Projects\_C++_Home_Projects\RTracer\shaders\triangle.vert -o F:\Work\Projects\_C++_Home_Projects\RTracer\shaders\triangle_vert.spv
glslc.exe F:\Work\Projects\_C++_Home_Projects\RTracer\shaders\triangle.frag -o F:\Work\Projects\_C++_Home_Projects\RTracer\shaders\triangle_frag.spv

glslc.exe F:\Work\Projects\_C++_Home_Projects\RTracer\shaders\passthrough.vert -o F:\Work\Projects\_C++_Home_Projects\RTracer\shaders\passthrough_vert.spv
glslc.exe F:\Work\Projects\_C++_Home_Projects\RTracer\shaders\passthrough.frag -o F:\Work\Projects\_C++_Home_Projects\RTracer\shaders\passthrough_frag.spv

glslc.exe --target-spv=spv1.6 F:\Work\Projects\_C++_Home_Projects\RTracer\shaders\raytrace.rchit -o F:\Work\Projects\_C++_Home_Projects\RTracer\shaders\ray_closest_hit.spv
glslc.exe --target-spv=spv1.6 F:\Work\Projects\_C++_Home_Projects\RTracer\shaders\raytrace.rgen -o F:\Work\Projects\_C++_Home_Projects\RTracer\shaders\ray_gen.spv
glslc.exe --target-spv=spv1.6 F:\Work\Projects\_C++_Home_Projects\RTracer\shaders\raytrace.rmiss -o F:\Work\Projects\_C++_Home_Projects\RTracer\shaders\ray_miss.spv