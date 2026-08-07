# Builds shaderc (and its glslang / SPIRV-Tools / SPIRV-Headers dependencies)                                                                                                                                                                                                                                    
# from source via FetchContent, so we don't depend on the prebuilt
# shaderc_combined archive bundled in the LunarG Vulkan SDK. Provides the
# `shaderc` target for linking.

include(FetchContent)

set(SHADERC_SKIP_TESTS ON)
set(SHADERC_SKIP_EXAMPLES ON)
set(SHADERC_SKIP_COPYRIGHT_CHECK ON)
set(SHADERC_SKIP_INSTALL ON)
set(SHADERC_SKIP_EXECUTABLES ON)        # we only need the library, not glslc
set(SHADERC_ENABLE_WERROR_COMPILE OFF)  # don't let upstream -Werror break your build
set(SHADERC_ENABLE_SHARED_CRT ON)       # match RTracer's default /MD runtime on MSVC

set(SPIRV_SKIP_TESTS ON)
set(SPIRV_SKIP_EXECUTABLES ON)
set(SPIRV_WERROR OFF)

set(SPIRV_HEADERS_SKIP_EXAMPLES ON)

set(ENABLE_GLSLANG_BINARIES OFF)
set(ENABLE_CTEST OFF)
set(BUILD_TESTING OFF)

# Commit pins taken from shaderc's own DEPS file at tag v2026.3:
# https://github.com/google/shaderc/blob/v2026.3/DEPS
# Re-check that file (and update these) whenever SHADERC_GIT_TAG is bumped.
set(SHADERC_GIT_TAG       "v2026.3")
set(SPIRV_HEADERS_GIT_TAG "29981f65241605e08b0ede4cfeb999fe3b723c6a")
set(SPIRV_TOOLS_GIT_TAG   "b707790a898e44038547df54580022fc1cf89c3d")
set(GLSLANG_GIT_TAG       "168d452a4f460d24b588fed08477a81c44ee27a1")

FetchContent_Declare(spirv-headers
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Headers.git
        GIT_TAG        ${SPIRV_HEADERS_GIT_TAG}
        GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(spirv-headers)
# SPIRV-Tools looks for this exact mixed-case name, not FetchContent's
# lower-cased spirv-headers_SOURCE_DIR — bridge it manually.
set(SPIRV-Headers_SOURCE_DIR ${spirv-headers_SOURCE_DIR})

FetchContent_Declare(spirv-tools
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Tools.git
        GIT_TAG        ${SPIRV_TOOLS_GIT_TAG}
        GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(spirv-tools)

FetchContent_Declare(glslang
        GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
        GIT_TAG        ${GLSLANG_GIT_TAG}
        GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(glslang)

FetchContent_Declare(shaderc
        GIT_REPOSITORY https://github.com/google/shaderc.git
        GIT_TAG        ${SHADERC_GIT_TAG}
        GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(shaderc)