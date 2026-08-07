# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Working Mode: Read-Only Mentor

Claude Code acts as a mentor and analyst on this project, not an implementer. In this repo:

- **Do not edit or create files.** No `Edit`, `Write`, or other file-modifying actions. All code changes are made by the user.
- **Do not run the code, tests, or build.** No `cmake --build`, no running `bin/RTracer.exe` or `bin/Test.exe`, no other execution — not even for verification purposes. Read-only shell commands for inspection (e.g. `git status`, `git log`, `git diff`) are fine; anything that builds, runs, or mutates state is not.
- **Instead:** read the relevant code, explain what you find, and propose a concrete solution (code snippets, diffs-as-text, or step-by-step instructions) for the user to apply themselves.
- If a task seems to require an edit or a run to make progress, stop and describe what you would do and why, rather than doing it.

## Project Overview

RTracer is a Vulkan-based GPU ray tracer written in C++17. It uses Vulkan Ray Tracing extensions (KHR) for hardware-accelerated path tracing with physically-based materials, multiple light types, and image-based lighting.

## Build

Requires Vulkan SDK 1.2+ (with shaderc) and CMake 3.24+.

```bash
cmake -B build -S .
cmake --build build
```

Outputs: `bin/RTracer.exe` (main app) and `bin/Test.exe` (Catch2 tests).

To run tests: `./bin/Test.exe`

CMake option `ENABLE_GPU_TIMINGS` (default ON) adds per-task GPU timing instrumentation.

## Architecture

### Rendering Pipeline

The renderer uses a **wavefront-style GPU pipeline** where work is spread across multiple GPU tasks rather than a single monolithic shader:

1. **Ray generation** (`task_raytrace`) — casts primary rays from camera
2. **Intersection** — handled by Vulkan acceleration structures (BLAS/TLAS)
3. **Material sorting** (`task_material_sort_*`) — 7 compute shaders perform a parallel prefix-sum sort to group hits by material type, improving GPU cache coherence
4. **Material evaluation** (`task_master_shader`) — evaluates BXDF, samples next direction, spawns secondary rays
5. **Miss shading** (`task_miss`) — samples IBL/sky for escaped rays
6. **Accumulation** (`task_accumulate`) — blends current frame with history for progressive refinement
7. **Display** (`task_passthrough`) — blit to swapchain

`src/engine/render.cpp` (`FRender`) is the central orchestrator: it owns all GPU resources, creates ECS entities, and submits tasks.

### ECS

A custom ECS (`src/ECS/`) is used to manage scene objects. Components are plain data; systems contain logic. The split between "host components" (CPU-side) and "device components" (GPU buffer–backed) is intentional — systems like `FMeshSystem` and `FMaterialSystem` sync CPU state to GPU buffers each frame.

### Vulkan Abstraction

`src/vulkan_helpers/` provides thin wrappers:
- `vk_context` — device, queues, command pools (global singleton pattern)
- `vk_shader_compiler` — runtime GLSL→SPIR-V compilation via shaderc
- `resource_allocation` — VMA-backed buffer/image allocation
- `descriptors` — descriptor set layout and pool management
- `texture_manager` — handles image loading (stb/tinyexr) and sampler creation

### Shader System

All shaders live in `src/shaders/`. They are compiled at runtime from GLSL 460 source using shaderc. Key files:

| File | Role |
|---|---|
| `master_shader.rgen` | Ray generation entry point, manages bounce loop state |
| `master_shader.rchit` | Closest-hit: reads geometry, dispatches to material logic |
| `master_shader.rmiss` | Miss: environment/sky lookup |
| `bxdf.h` | All BRDF/BTDF implementations (diffuse, GGX specular, transmission, SSS, sheen, coat, thin-film) |
| `process_material_interaction.h` | Top-level material scattering: selects lobe, computes weight, updates ray |
| `lighting.h` | Light sampling (point, directional, spot, area, IBL) |
| `common_structures.h` | Shared GPU structs (lights, camera, material parameters) |
| `random.h` | PCG-based RNG |

The material model is inspired by MaterialX Standard Surface: albedo, specular weight/roughness/IOR/anisotropy, transmission, subsurface, sheen, coat (clearcoat), thin-film, and emission are all supported.

### Task System

GPU tasks inherit from `FExecutableTask` (`src/engine/tasks/executable_task.h`). Each task owns its pipeline, descriptor sets, and knows how to record itself into a command buffer. `FRender` sequences them each frame.

### Scene Loading

`src/scene_loader/scene_loader.cpp` contains hardcoded scene definitions (Cornell box, sphere scenes, etc.). Scenes are selected at startup; there is no runtime scene file parsing. Camera presets are stored as JSON in `data/cameras/`.

## Code Conventions

- `.clang-format` is present (LLVM-based style) — run clang-format before committing.
- GPU structs shared between C++ and GLSL are defined in `src/shaders/common_structures.h` and mirrored manually in C++ component headers — keep these in sync when modifying material or light data layouts.
- The `F` prefix is used for classes (e.g., `FRender`, `FMeshSystem`), following Unreal Engine naming conventions.