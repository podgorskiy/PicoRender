#pragma once

#include "types.h"


inline CUDA float schlick(float cosine, float ref_idx)
{
    float r0 = (1.0f - ref_idx) / (1.0f + ref_idx);
    r0 = r0 * r0;
    return r0 + (1.0f - r0) * powf((1.0f - cosine), 5.0f);
}

inline CUDA bool refract(const vec3 &v, const vec3 &n, float ni_over_nt, vec3 &refracted)
{
    vec3 uv = normalize(v);
    float dt = dot(uv, n);
    float discriminant = 1.0f - ni_over_nt * ni_over_nt * (1 - dt * dt);
    if (discriminant > 0)
    {
        refracted = ni_over_nt * (uv - n * dt) - n * scal(sqrtf(discriminant));
        return true;
    }
    else
        return false;
}

inline CUDA vec3 reflect(const vec3 &v, const vec3 &n)
{
    return v - scal(2.0) * dot(v, n) * n;
}
