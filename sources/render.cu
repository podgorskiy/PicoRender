#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include "ray_payload.h"
#include "sampling.h"
#include "Camera.h"

/*! the 'builtin' launch index we need to render a frame */
rtDeclareVariable(uint2, pixelID,   rtLaunchIndex, );
rtDeclareVariable(uint2, launchDim, rtLaunchDim,   );

/*! the ray related state */
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(RayPayload, ray_payload, rtPayload, );

rtDeclareVariable(rtObject, root, , );

rtBuffer<float4, 2> pixelBuffer;

rtDeclareVariable(int,      numSamples, , );
rtDeclareVariable(int,      bounces, , );
rtDeclareVariable(float3,   camera_origin, , );
rtDeclareVariable(float3,   camera_lookat, , );
rtDeclareVariable(float3,   camera_up, , );
rtDeclareVariable(float,    camera_vfov, , );
rtDeclareVariable(float,    camera_aperture, , );
rtDeclareVariable(float,    camera_focusDist, , );



inline __device__ vec3 missColor(const optix::Ray &ray)
{
  const vec3 unit_direction = normalize(to_glm(ray.direction));
  const scal t = 0.5*(unit_direction.y + 1.0);
  const vec3 c = (scal(1.0) - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
  return c;
}


inline __device__ vec4 ComputeBounces(optix::Ray &ray, rnd::RandomState &rs)
{
    RayPayload ray_payload;

	vec3 light(0.0);
	vec3 color(1.0);
    int k = 0;
    for (; k < bounces; ++k)
    {
        ray_payload.rs = &rs;
        rtTrace(root, ray, ray_payload);
        if (ray_payload.scatterEvent == RayPayload::rayDidntHitAnything)
        {
            light = missColor(ray);
            break;
        }
        else if (length(color) < 0.01 || ray_payload.scatterEvent == RayPayload::rayGotCancelled)
        {
            break;
        }
        else
        {
            color *= ray_payload.attenuation;
            ray = optix::make_Ray(
                    /* origin   : */ to_cuda(ray_payload.origin),
                    /* direction: */ to_cuda(ray_payload.direction),
                    /* ray type : */ 0,
                    /* tmin     : */ 1e-3f,
                    /* tmax     : */ RT_DEFAULT_MAX);
        }
    }
    if (k != 0)
    {
        return vec4(light * color, 1.);
    }
    else
    {
        return vec4(0.);
    }
}


RT_PROGRAM void Render()
{
    uint32_t pixel_index = pixelID.y * launchDim.x + pixelID.x;

    vec4 col(0.f, 0.f, 0.f, 0.f);
    rnd::RandomState rs(pixel_index);

    float aspect = float(launchDim.x) / float(launchDim.y);

    Camera camera = Camera(
            to_glm(camera_origin),
            to_glm(camera_lookat),
            to_glm(camera_up),
            camera_vfov, aspect, camera_aperture, camera_focusDist);

    for (int s = 0; s < numSamples; s++)
    {
        int y_id = launchDim.y - pixelID.y - 1;
        float u = float(pixelID.x + rs.rand1()) / float(launchDim.x);
        float v = float(y_id + rs.rand1()) / float(launchDim.y);

        optix::Ray ray = camera.generateRay(vec2(u, v), rs);

        col += ComputeBounces(ray, rs);
        // col += missColor(ray);
    }
    col = col / scal(numSamples);

    pixelBuffer[pixelID] = to_cuda(vec4(pow(vec3(col), vec3(1.0 / 2.2)), col.a));
}

RT_PROGRAM void Miss()
{
    ray_payload.scatterEvent = RayPayload::rayDidntHitAnything;
}
