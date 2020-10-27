#pragma once
#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_vector_types.h>
#include <inttypes.h>
#include <glm/glm.hpp>

#if defined(__CUDACC__)
#define CUDA_HOST __host__
#define CUDA_DEVICE __device__
#define CUDA __host__ __device__
#else
#define CUDA_HOST
#define CUDA_DEVICE
#define CUDA
#endif

#define PI_F 3.141592654f

#define PI_D 3.14159265358979323846264338327950288

typedef optix::float4 vec4;
typedef optix::float3 vec3;
typedef optix::float2 vec2;
typedef float scal;

using  optix::dot;
using  optix::length;
using  optix::cross;
using  optix::normalize;
using  optix::make_float2;
using  optix::make_float3;
using  optix::make_float4;
