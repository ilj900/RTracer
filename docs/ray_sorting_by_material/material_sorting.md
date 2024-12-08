Lets say we have a picture 20x20 with two shapes and each of them has a different material. Something like this:
![original-image](./a.jpg "original-image")

For simplification, the maximum number of material will be 8,
with the 8th one reserved for cases when ray misses the geometry (skybox material).

The first step of sorting materials is generating MaterialID map. It will look like this:
![material-id-map](./b.jpg "material-id-map")

Then we need to calculate entry of each material. But this will be done in chunks. Each chunk will cover 32 pixels,
So, for a 20x20 image we need 13 chunks, but, to simplify prefix sum computation, we allocate a power of 2 chunks - 16
Each of the chunk will have 8 uint counters (for each material).

Chunks partition will look something like this:
![chunk-partitioned-original-image](./c.jpg "chunk-partitioned-original-image")

After all the materials counted, we'll have a chank-to-material-counted buffer like this:
![chunk-to-material-counted](./d.jpg "chunk-to-material-counted")
> Coloring here is only to show where each shape contributed.

The last row (material 7) represents skybox materials, where rays didn't hit any geometry.

Then we calculate prefix sum for every material. [See chapter 39 in GPU Gems 3 ](https://developer.nvidia.com/gpugems/gpugems3/part-vi-gpu-computing/chapter-39-parallel-prefix-sum-scan-cuda)

After prefix sum has been calculated, the buffer will look like this
![chunk-to-material-counted](./e.jpg "chunk-to-material-counted")

Also, while calculating the prefix sum, we count how many entries of each material are there.
To make it possible to use this buffer as source for vkCmdDispatchIndirect each entry is actually a vec3 where x is materials count and y and z equals to 1.
![counted-materials-buffer](./f.jpg "counted-materials-buffer")

Now we create an additional buffer for prefix sum of counted materials - materials-offsets-buffer \
![materials-offsets](./g.jpg "materials-offsets")

Now we are ready to generate a texture that will map a sorted pixel index to the original one. \
To do so we run a compute shader for every pixel. In shader we:
1. Get pixel index: \
`uint PixelIndex = gl_GlobalInvocationID.x;`
2. Get the chunk index: \
`uint ChunkIndex = PixelIndex / BASIC_CHUNK_SIZE;` (BASIC_CHUNK_SIZE = 32 in out case)
3. Get the material index: \
`uint MaterialIndex = UnsortedMaterials[PixelIndex];`
4. Now we get the materia's offset from chunk-to-material-counted buffer and simultaneously increase it by 1 with atomicAdd: \
`uint NewRelativeIndex = atomicAdd(MaterialsOffsetsPerChunk[OriginalMaterialIndex * PushConstants.MaxGroupSize + ChunkIndex], 1);` \
this will be the number relative to the starting index of the material
5. Now we need to calculate new absolute pixel index. \
Just add the relative one to the starting one from  materials-offsets buffer: \
`uint NewRayIndex = MaterialsOffsetsPerMaterial[OriginalMaterialIndex] + NewRelativeIndex;`
6. Save the result: \
`SortedMaterialsIndexMap[NewRayIndex] = PixelIndex;`

After this you will have texture that looks something like this: \
(The order of indices inside the material might be different because of concurrency, \
but all the indices will refer to the indices of the appropriate material)
![new-to-old-pixel-map](./i.jpg "new-to-old-pixel-map")

Now everything is ready. When you decide to render all pixels that hit a particular material - lets say second material (the one that on a triangle shape)
You call vkCmdDispatchIndirect with chunk-to-material-counted buffer and offset apropriate to that material. In our case the offset is `MaterialIndex * 3 * sizeof(uint32_t)`, which will result in 1 * 3 * 4 = 12 bytes
It will point here:
![counted-materials-buffer](./h.jpg "counted-materials-buffer")
So, the compute shader will be executed 27 times in x, 1 in y and 1 in z, which is actually the amount of pixels in the triangle

In shader we have access to material index, gl_GlobalInvocationID.x and new-to-old-pixel-map. \
With this we can obtain the original pixel index. To do so:\
1. Generate the "new" pixel index. Fetch data from materials-offsets-buffer and add gl_GlobalInvocationID.x \
(gl_GlobalInvocationID.x runs from 0 to 26)
`uint PixelIndex = MaterialsOffsets[PushConstants.MaterialIndex] + gl_GlobalInvocationID.x;`
2. Fetch the "original" pixel index from new-to-old-pixel-map: \
`uint NewPixelIndex = MaterialsIndexMap[OriginalPixelIndex];`
3. 
And it's done. Now you have the pixel from original image with that material!