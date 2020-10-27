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
rtDeclareVariable(rtObject, root_uv, , );

rtBuffer<float4, 2> albedoBuffer;
rtBuffer<float4, 2> normalBuffer;
rtBuffer<float4, 2> bentNormalBuffer;
rtBuffer<float4, 2> giBuffer;
rtBuffer<float4, 2> finalBuffer;

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
    return make_float3(1.0);
}


inline __device__ vec4 Radiance(optix::Ray &ray, rnd::RandomState &rs, vec3& albedo_out, vec3& normal_out, vec3& bent_normal_out)
{
    RayPayload ray_payload;

	vec3 light = make_float3(0.0);
	vec3 color = make_float3(1.0);
	vec3 attenuation;

    int k = 0;
    for (; k < bounces; ++k)
    {
        ray_payload.rs = &rs;
        rtTrace(k == 0 ? root_uv : root, ray, ray_payload);
        if (ray_payload.scatterEvent == RayPayload::rayDidntHitAnything)
        {
            light = missColor(ray);
            break;
        }
        else if (ray_payload.scatterEvent == RayPayload::rayGotBounced)
        {
			attenuation = ray_payload.attenuation;
            //color *= ray_payload.attenuation;
            ray = optix::make_Ray(
                    /* origin   : */ ray_payload.origin,
                    /* direction: */ ray_payload.direction,
                    /* ray type : */ 0,
                    /* tmin     : */ 1e-3f,
                    /* tmax     : */ RT_DEFAULT_MAX);

			if (k == 0)
			{
				albedo_out = attenuation;
				normal_out = ray_payload.normal;
				attenuation = make_float3(1.0);
				bent_normal_out = ray_payload.direction;
			}
            color = color * attenuation;

            if (length(color) < 0.01)
            {
                break;
            }
        }
        else
        {
            color = make_float3(0.0);
            break;
        }
    }
    if (k != 0)
    {
        vec3 lightColor = light * color;
		bent_normal_out *= dot(lightColor, make_float3(0.3, 0.59, 0.11));
        return make_float4(lightColor, 1.);
    }
    else
    {
		albedo_out = make_float3(0.);
		normal_out = make_float3(0.);
		bent_normal_out = make_float3(0.);
        return make_float4(0.);
    }
}


__device__ vec3 pow(vec3 x, float e)
{
    return make_float3(powf(x.x, e), powf(x.y, e), powf(x.z, e));
}

__device__ vec4 pow(vec4 x, float e)
{
    return make_float4(powf(x.x, e), powf(x.y, e), powf(x.z, e), x.w);
}


RT_PROGRAM void RenderUV()
{
    uint32_t pixel_index = pixelID.y * launchDim.x + pixelID.x;

    vec4 col = make_float4(0.f, 0.f, 0.f, 0.f);
    rnd::RandomState rs(pixel_index, pixel_index*pixel_index);

    float aspect = float(launchDim.x) / float(launchDim.y);

    Camera camera = Camera(
            camera_origin,
            camera_lookat,
            camera_up,
            camera_vfov, aspect, camera_aperture, camera_focusDist);

    vec3 albedo = make_float3(0.);
    vec3 normal = make_float3(0.);
    vec3 bent_normal = make_float3(0.);

    for (int s = 0; s < numSamples; s++)
    {
        int y_id = launchDim.y - pixelID.y - 1;
        float u = float(pixelID.x + rs.rand1()) / float(launchDim.x);
        float v = float(y_id + rs.rand1()) / float(launchDim.y);

        optix::Ray ray = optix::make_Ray(
                /* origin   : */ make_float3(u, v, -1.0),
                /* direction: */ make_float3(0, 0, 1.),
                /* ray type : */ 0,
                /* tmin     : */ 1e-6f,
                /* tmax     : */ RT_DEFAULT_MAX);

        vec3 albedo_out;
        vec3 normal_out;
        vec3 bent_normal_out;

        col += Radiance(ray, rs, albedo_out, normal_out, bent_normal_out);
        albedo += albedo_out;
        normal += normal_out;
        bent_normal += bent_normal_out;
    }
    float c = col.w;

    vec4 final;
    if (col.w > 0.)
    {
        col /= c;
        albedo /= c;
        final = make_float4(make_float3(col.x, col.y, col.z) * albedo, 1.0);

        col = pow(col, 1.0/ 2.2);
        final = pow(final, 1.0/ 2.2);
        albedo = pow(albedo, 1.0/ 2.2);
        normal = normalize(normal);
        bent_normal = normalize(bent_normal);
    }
    else
    {
        col = make_float4(0.0);
        final = make_float4(0.0);
        albedo = make_float3(0.0);
        normal = make_float3(0.0);
        bent_normal = make_float3(0.0);
    }

    // pixelBuffer[pixelID] = col;
    albedoBuffer[pixelID] = make_float4(albedo, col.w);
    normalBuffer[pixelID] = make_float4(normal * 0.5 + make_float3(0.5), col.w);
    bentNormalBuffer[pixelID] = make_float4(bent_normal * 0.5 + make_float3(0.5), dot(make_float3(col), make_float3(0.3, 0.59, 0.11)));
    giBuffer[pixelID] = col;
    finalBuffer[pixelID] = final;
}

RT_PROGRAM void Miss()
{
    ray_payload.scatterEvent = RayPayload::rayDidntHitAnything;
}
