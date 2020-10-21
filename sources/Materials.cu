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


/*! and finally - that particular material's parameters */
rtDeclareVariable(float3, albedo, , );


RT_PROGRAM void lambertian_hit()
{
    ray_payload.scatterEvent = RayPayload::rayGotBounced;
    ray_payload.direction = lambert_no_tangent(to_glm(hit_rec_normal), ray_payload.rs);
    ray_payload.origin = to_glm(hit_rec_p);
    ray_payload.attenuation = to_glm(albedo);
}
