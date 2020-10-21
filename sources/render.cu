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

rtDeclareVariable(rtObject, world, , );


rtBuffer<float3, 2> fb;
rtDeclareVariable(int, numSamples, , );
rtDeclareVariable(float3, camera_origin, , );
rtDeclareVariable(float3, camera_lookat, , );
rtDeclareVariable(float3, camera_up, , );
rtDeclareVariable(float,  camera_vfov, , );
rtDeclareVariable(float,  camera_aperture, , );
rtDeclareVariable(float,  camera_focusDist, , );



inline __device__ vec3 missColor(const optix::Ray &ray)
{
  const vec3 unit_direction = normalize(to_glm(ray.direction));
  const scal t = 0.5*(unit_direction.y + 1.0);
  const vec3 c = (scal(1.0) - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
  return c;
}


inline __device__ vec3 ComputeBounces(optix::Ray &ray, rnd::RandomState &rs)
{
    RayPayload ray_payload;
    vec3 attenuation = vec3(1.);

    for (int k = 0; k < 5; ++k)
    {
        ray_payload.rs = &rs;
        rtTrace(world, ray, ray_payload);
        if (ray_payload.scatterEvent == RayPayload::rayDidntHitAnything)
        {
            return attenuation * missColor(ray);
        }
        else if (length(attenuation) < 0.01 || ray_payload.scatterEvent == RayPayload::rayGotCancelled)
        {
            return vec3(0.);
        }
        else { // ray is still alive, and got properly bounced
            attenuation *= ray_payload.attenuation;
            ray = optix::make_Ray(
                    /* origin   : */ to_cuda(ray_payload.origin),
                    /* direction: */ to_cuda(ray_payload.direction),
                    /* ray type : */ 0,
                    /* tmin     : */ 1e-3f,
                    /* tmax     : */ RT_DEFAULT_MAX);
        }
    }
    // recursion did not terminate - cancel it
    return vec3(0.);
}


RT_PROGRAM void Render()
{
    uint32_t pixel_index = pixelID.y * launchDim.x + pixelID.x;

    vec3 col(0.f, 0.f, 0.f);
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

    fb[pixelID] = to_cuda(pow(col, vec3(1.0 / 2.2)));
}

RT_PROGRAM void Miss()
{
    ray_payload.scatterEvent = RayPayload::rayDidntHitAnything;
}
