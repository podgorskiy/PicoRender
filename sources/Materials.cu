#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include <optix_cuda.h>
#include "ray_phisics.h"
#include "ray_payload.h"
#include "sampling.h"

/*! the implicit state's ray we will intersect against */
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
/*! the per ray data we operate on */
rtDeclareVariable(RayPayload, ray_payload, rtPayload, );
rtDeclareVariable(rtObject, world, , );

/*! the attributes we use to communicate between intersection programs and hit program */
rtDeclareVariable(float3, hit_rec_normal, attribute hit_rec_normal, );
rtDeclareVariable(float3, hit_rec_p, attribute hit_rec_p, );

rtDeclareVariable(float3, shading_normal, attribute shading_normal, );
rtDeclareVariable(float3, geometric_normal, attribute geometric_normal, );
rtDeclareVariable(float3, hit_point, attribute hit_point, );
rtDeclareVariable(float3, texcoord, attribute texcoord, );

/*! and finally - that particular material's parameters */
rtDeclareVariable(float3, albedo, , );


RT_PROGRAM void lambertian_hit()
{
    const vec3 normal = faceforward( shading_normal, -ray.direction, geometric_normal );

    ray_payload.scatterEvent = RayPayload::rayGotBounced;
    ray_payload.direction = lambert_no_tangent(normal, ray_payload.rs);
    ray_payload.origin = hit_point;
    ray_payload.attenuation = albedo;
}
