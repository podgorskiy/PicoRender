#pragma once
#include "types.h"
#include "sampling.h"
#include "ray.h"

class Camera
{
public:
	CUDA Camera(vec3 lookfrom, vec3 lookat, vec3 vup, float vfov, float aspect, float aperture, float focusDist)
	{
	    m_apertureRadius = aperture;
	    m_focusDist = focusDist;

	    scalar theta = vfov * PI_D / 180.0;
	    scalar halfHeight = tan(theta / 2.0);
	    scalar halfWidth = aspect * halfHeight;

	    m_origin = lookfrom;

	    vec3 z = normalize(lookfrom - lookat);
	    vec3 x = normalize(cross(vup, z));
	    vec3 y = normalize(cross(z, x));

	    m_cam = mat3(x, y, z);
	    m_k_inv = mat3(vec3(halfWidth, 0, 0), vec3(0, halfHeight, 0), vec3(0.0, 0.0, -1.));
	}

	PerRayData generateRay(vec2 uv)
	{
	    //vec3 rd = m_apertureRadius * random_in_unit_disk(uv, j);

	    PerRayData ray;
//	    vec2 pixel = 1.0/iResolution.xy;
//		uv += pixel * (hash22(uv) - 0.5);
//	    uv = 2.0 * uv - 1.0;
//
//	    ray.origin = m_origin + m_cam * rd;
//
//	    vec3 dir = m_k_inv * vec3(uv, 1.0);
//	    dir -= rd / m_focusDist;
//
//	    ray.direction = normalize(m_cam * dir);

	    return ray;
	}
private:
    float m_apertureRadius;
    vec3 m_origin;
    mat3 m_cam;
    mat3 m_k_inv;
    float m_focusDist;
};
