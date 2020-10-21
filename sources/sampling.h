#pragma once
#include "types.h"
#include "rnd.h"


inline CUDA vec3 lambert_no_tangent(vec3 normal, rnd::RandomState* rs)
{
    vec2 uv = rs->rand2();
    scal theta = 2.0f * PI_D * uv.x;
    uv.y = 2.0f * uv.y - 1.0f;
    vec2 p = make_float2(cos(theta), sin(theta)) * scal(sqrtf(1.0f - uv.y * uv.y));
    vec3 spherePoint = make_float3(p.x, p.y, uv.y);
    spherePoint += normal;
    spherePoint = normalize(spherePoint);
    return spherePoint;
}

inline CUDA vec2 random_in_unit_disk(rnd::RandomState* rs)
{
    vec2 uv = rs->rand2();
    scal theta = scal(2.0) * PI_F * uv.x;
    scal r = sqrt(uv.y);
    vec2 p = make_float2(cos(theta), sin(theta)) * r;
    return p;
}

inline CUDA vec3 random_on_surface_of_unit_sphere(rnd::RandomState* rs)
{
    vec2 uv = rs->rand2();
    scal s =  sqrt(1.f - uv.y *uv.y);
    return make_float3(cos(uv.x) * s, sin(uv.x) * s, uv.y);
}

inline CUDA vec3 random_in_unit_sphere(rnd::RandomState* rs)
{
	vec3 p = random_on_surface_of_unit_sphere(rs);
    scal r = pow(rs->rand1(), 1.0f / 3.0f);
    return p * r;
}
