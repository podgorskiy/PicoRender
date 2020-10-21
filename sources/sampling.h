#pragma once
#include "types.h"
#include "rnd.h"


inline CUDA vec3 lambert_no_tangent(vec3 normal, rnd::RandomState* rs)
{
    vec2 uv = rs->rand2();
    scalar theta = 2.0 * PI_D * uv.x;
    uv.y = 2.0 * uv.y - 1.0;
    vec2 p = vec2(cos(theta), sin(theta)) * sqrt(1.0 - uv.y * uv.y);
    vec3 spherePoint = vec3(p.x, p.y, uv.y);
    spherePoint += normal;
    spherePoint = glm::normalize(spherePoint);
    return spherePoint;
}

inline CUDA vec2 random_in_unit_disk(rnd::RandomState* rs)
{
    vec2 uv = rs->rand2();
    scalar theta = 2.0 * PI_F * uv.x;
    scalar r = sqrt(uv.y);
    vec2 p = vec2(cos(theta), sin(theta)) * r;
    return p;
}

inline CUDA vec3 random_on_surface_of_unit_sphere(rnd::RandomState* rs)
{
    vec2 uv = rs->rand2();
    scalar s =  sqrt(1 - uv.y *uv.y);
    return vec3(cos(uv.x) * s, sin(uv.x) * s, uv.y);
}

inline CUDA vec3 random_in_unit_sphere(rnd::RandomState* rs)
{
	vec3 p = random_on_surface_of_unit_sphere(rs);
    scalar r = pow(rs->rand1(), 1.0 / 3.0);
    return p * r;
}
