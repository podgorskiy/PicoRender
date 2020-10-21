cmake_minimum_required(VERSION 2.8)

cmake_policy(SET CMP0072 NEW)
cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0079 NEW)

project(PicoRender)

option (USE_MSVC_RUNTIME_LIBRARY_DLL OFF)

set ( CMAKE_CXX_STANDARD 11 )

# set to match your path
set(OptiX_INSTALL_DIR "/usr/local/nvidia/NVIDIA-OptiX-SDK-6.5.0-linux64" CACHE PATH "Path to OptiX installed location.")

set(CMAKE_MODULE_PATH
  "${OptiX_INSTALL_DIR}/SDK/CMake"
  ${CMAKE_MODULE_PATH}
  )

message(STATUS "OptiX_INSTALL_DIR: ${OptiX_INSTALL_DIR}")
message(STATUS "PROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR}")
message(STATUS "CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}")

include(CMake/configure_optix.cmake)

include_directories(${OptiX_INCLUDE})
message(STATUS "OptiX includes: ${OptiX_INCLUDE}")

add_definitions(-D__CUDA_INCLUDE_COMPILER_INTERNAL_HEADERS__=1)


if(MSVC)
    set(COMMON_OPTIONS /W3 /wd4996 /we4239 /we4002 /we4700 /we4305 /EHsc /MP)
    set(DEBUG_OPTIONS /MTd /Od /Zi)
    set(RELEASE_OPTIONS /MT /Ox /fp:fast /GL /GR-)
else()
	set(DEBUG_OPTIONS -g -Wall)
	set(RELEASE_OPTIONS -g -Ofast -fno-rtti -flto -fPIC -funsafe-math-optimizations -fno-strict-aliasing  -ffast-math -msse2 -msse3 -msse4)
endif()

add_compile_options(
        "$<$<CONFIG:RELEASE>:${RELEASE_OPTIONS}>"
        "$<$<CONFIG:DEBUG>:${DEBUG_OPTIONS}>")

add_subdirectory (libs/yaml-cpp)
add_subdirectory (libs/spdlog)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/stb)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/glm)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/yaml-cpp/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/backward-cpp)

configure_file(teapot.obj teapot.obj COPYONLY)
configure_file(wood.tga wood.tga COPYONLY)
configure_file(config.txt config.txt  COPYONLY)
configure_file(config_post.txt config_post.txt  COPYONLY)

file(GLOB_RECURSE SOURCES sources/*.cpp  sources/*.c sources/*.h)

cuda_compile_and_embed(
  render_program sources/render.cu
)
cuda_compile_and_embed(
  materials_program sources/Materials.cu
)
cuda_compile_and_embed(
  sphere_program sources/sphere.cu
)

add_executable(PicoRender ${SOURCES} ${render_program} ${materials_program} ${sphere_program})

target_link_libraries(PicoRender PRIVATE spdlog::spdlog yaml-cpp ${optix_LIBRARY})

if(MSVC)
else()
	target_link_libraries(PicoRender PRIVATE pthread  dl)
	target_link_options(PicoRender PRIVATE -static-libstdc++ -flto)
endif()
