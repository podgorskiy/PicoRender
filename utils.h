#pragma once
#include "types.h"
#include "rnd.h"



inline vec3 lambertNoTangent(vec3 normal)
{
    vec2 uv(rnd::rand(), rnd::rand());
    float theta = 2.0 * PI * uv.x;
    uv.y = 2.0 * uv.y - 1.0;
    vec2 p = vec2(cos(theta), sin(theta)) * sqrt(1.0 - uv.y * uv.y);
    vec3 spherePoint = vec3(p.x, p.y, uv.y);
    spherePoint += normal;
    spherePoint = glm::normalize(spherePoint);
    return spherePoint;
}
