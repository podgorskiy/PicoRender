#pragma once
#include <optix.h>
#include <internal/optix_datatypes.h>
#include "types.h"
#include "sampling.h"
#include "ray_payload.h"

class Camera
{
public:
	CUDA Camera()
    {}

	CUDA Camera(vec3 lookfrom, vec3 lookat, vec3 vup, float vfov, float aspect, float aperture, float focusDist)
	{
	    m_apertureRadius = aperture;
	    m_focusDist = focusDist;

	    scal theta = vfov * PI_D / 180.0;
	    scal halfHeight = tan(theta / 2.0);
	    scal halfWidth = aspect * halfHeight;

	    m_origin = lookfrom;

	    vec3 z = normalize(lookfrom - lookat);
	    vec3 x = normalize(cross(vup, z));
	    vec3 y = normalize(cross(z, x));

	    m_cam = mat3(x, y, z);
	    m_k_inv = mat3(vec3(halfWidth, 0, 0), vec3(0, halfHeight, 0), vec3(0.0, 0.0, -1.));
	}

	CUDA optix::Ray generateRay(vec2 uv, rnd::RandomState& rs)
	{
	    vec3 rd = m_apertureRadius * vec3(random_in_unit_disk(&rs), 0.0);

	    uv = scal(2.0) * uv - scal(1.0);

	    vec3 origin = m_origin + m_cam * rd;

	    vec3 direction = m_k_inv * vec3(uv, 1.0);
	    direction -= rd / m_focusDist;

	    direction = normalize(m_cam * direction);

        return optix::make_Ray(
                /* origin   : */ to_cuda(origin),
                /* direction: */ to_cuda(direction),
                /* ray type : */ 0,
                /* tmin     : */ 1e-6f,
                /* tmax     : */ RT_DEFAULT_MAX);
    }
private:
    scal m_apertureRadius;
    vec3 m_origin;
    mat3 m_cam;
    mat3 m_k_inv;
    scal m_focusDist;
};
