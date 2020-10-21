#pragma once
#include "types.h"
#include "rnd.h"


inline CUDA vec3 __lambert_no_tangent(vec3 normal, rnd::RandomState* rs)
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

inline CUDA vec3 random_cosine_direction(rnd::RandomState* rs)
{
    vec2 uv = rs->rand2();
    auto z = sqrt(1-uv.y);

    auto phi = 2.0 * PI_F * uv.x;
    auto x = cos(phi)*sqrt(uv.y);
    auto y = sin(phi)*sqrt(uv.y);

    return make_float3(x, y, z);
}

inline CUDA vec3 lambert_no_tangent(vec3 normal, rnd::RandomState* rs)
{
    vec3 local = random_cosine_direction(rs);
    vec3 w = normal;
    vec3 a = (fabs(w.x) > 0.9) ? make_float3(0,1,0) : make_float3(1,0,0);
    vec3 v = normalize(cross(w, a));
    vec3 u = cross(w, v);
    return local.x * u + local.y * v + local.z * w;
}
