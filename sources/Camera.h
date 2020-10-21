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
	    halfHeight = tan(theta / 2.0);
	    halfWidth = aspect * halfHeight;

	    m_origin = lookfrom;

	    m_cam_z = normalize(lookfrom - lookat);
	    m_cam_x = normalize(cross(vup, m_cam_z));
	    m_cam_y = normalize(cross(m_cam_z, m_cam_x));
	}

	CUDA optix::Ray generateRay(vec2 uv, rnd::RandomState& rs)
	{
	    vec3 rd = m_apertureRadius * make_float3(random_in_unit_disk(&rs), 0.0);

	    uv = scal(2.0) * uv - scal(1.0);

	    vec3 origin = m_origin + m_cam_x * rd.x + m_cam_y * rd.y + m_cam_z * rd.z;

	    vec3 direction = make_float3(uv, -1.0);
	    direction.x *= halfWidth;
	    direction.y *= halfHeight;

	    direction -= rd / m_focusDist;

	    direction = normalize(m_cam_x * direction.x + m_cam_y * direction.y + m_cam_z * direction.z);

        return optix::make_Ray(
                /* origin   : */ origin,
                /* direction: */ direction,
                /* ray type : */ 0,
                /* tmin     : */ 1e-6f,
                /* tmax     : */ RT_DEFAULT_MAX);
    }
private:
    scal m_apertureRadius;
    vec3 m_origin;
    vec3 m_cam_z;
    vec3 m_cam_x;
    vec3 m_cam_y;
    scal halfHeight;
    scal halfWidth;
    scal m_focusDist;
};
