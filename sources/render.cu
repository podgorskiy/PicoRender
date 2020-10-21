#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include "ray.h"
#include "sampling.h"

/*! the 'builtin' launch index we need to render a frame */
rtDeclareVariable(uint2, pixelID,   rtLaunchIndex, );
rtDeclareVariable(uint2, launchDim, rtLaunchDim,   );

/*! the ray related state */
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(PerRayData, prd, rtPayload, );

/*! the 2D, float3-type color frame buffer we'll write into */
rtBuffer<float3, 2> fb;

rtDeclareVariable(int, numSamples, , );

rtDeclareVariable(rtObject, world, , );

rtDeclareVariable(float3, camera_lower_left_corner, , );
rtDeclareVariable(float3, camera_horizontal, , );
rtDeclareVariable(float3, camera_vertical, , );
rtDeclareVariable(float3, camera_origin, , );
rtDeclareVariable(float3, camera_u, , );
rtDeclareVariable(float3, camera_v, , );
rtDeclareVariable(float, camera_lens_radius, , );


/*! the actual ray generation program - note this has no formal
  function parameters, but gets its paramters throught the 'pixelID'
  and 'pixelBuffer' variables/buffers declared above */
RT_PROGRAM void Render()
{
    uint32_t pixel_index = pixelID.y * launchDim.x + pixelID.x;
    vec3 col(0.f, 0.f, 0.f);
    rnd::RandomState rs(pixel_index);

    for (int s = 0; s < numSamples; s++)
    {
        float u = float(pixelID.x + rs.rand1()) / float(launchDim.x);
        float v = float(pixelID.y + rs.rand1()) / float(launchDim.y);
        // optix::Ray ray = Camera::generateRay(u, v, rnd);
        // col += color(ray, rnd);
        col += vec3(u, v, 0.);
    }
    col = col / scalar(numSamples);

    fb[pixelID] = to_cuda(col);
}
