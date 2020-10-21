
#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>
#include <optixu/optixu_aabb_namespace.h>

using namespace optix;

rtBuffer<float3> vertex_buffer;
rtBuffer<float3> normal_buffer;
rtBuffer<float2> texcoord_buffer;
rtBuffer<int3>   index_buffer;
rtBuffer<int>    material_buffer;

rtDeclareVariable(float3, texcoord,         attribute texcoord, );
rtDeclareVariable(float3, geometric_normal, attribute geometric_normal, );
rtDeclareVariable(float3, shading_normal,   attribute shading_normal, );

rtDeclareVariable(float3, hit_point,        attribute hit_point, );

rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );


RT_PROGRAM void mesh_intersect(int primIdx) {
    const int3 v_0 = index_buffer[3 * primIdx + 0];
    const int3 v_1 = index_buffer[3 * primIdx + 1];
    const int3 v_2 = index_buffer[3 * primIdx + 2];

    const float3 p0 = vertex_buffer[v_0.x];
    const float3 p1 = vertex_buffer[v_1.x];
    const float3 p2 = vertex_buffer[v_2.x];

    // Intersect ray with triangle
    float3 n;
    float t, beta, gamma;
    if (intersect_triangle(ray, p0, p1, p2, n, t, beta, gamma)) {

        if (rtPotentialIntersection(t)) {

            geometric_normal = normalize(n);
            if (normal_buffer.size() == 0) {
                shading_normal = geometric_normal;
            } else {
                float3 n0 = normal_buffer[v_0.y];
                float3 n1 = normal_buffer[v_1.y];
                float3 n2 = normal_buffer[v_2.y];
                shading_normal = normalize(n1 * beta + n2 * gamma + n0 * (1.0f - beta - gamma));
            }

            if (texcoord_buffer.size() == 0)
            {
                texcoord = make_float3(0.0f, 0.0f, 0.0f);
            } else {
                float2 t0 = texcoord_buffer[v_0.z];
                float2 t1 = texcoord_buffer[v_1.z];
                float2 t2 = texcoord_buffer[v_2.z];
                texcoord = make_float3(t1 * beta + t2 * gamma + t0 * (1.0f - beta - gamma));
            }
            float3 original_hit_point = ray.origin + t * ray.direction;

            float  refined_t = -(optix::dot(geometric_normal, original_hit_point - p0)) / optix::dot(geometric_normal, ray.direction);
            float3 refined_hit_point = original_hit_point + refined_t * ray.direction;

            hit_point = refined_hit_point;

            rtReportIntersection(material_buffer[primIdx]);
        }
    }
}


RT_PROGRAM void mesh_bounds(int primIdx, float result[6]) {
    const int3 v_0 = index_buffer[3 * primIdx + 0];
    const int3 v_1 = index_buffer[3 * primIdx + 1];
    const int3 v_2 = index_buffer[3 * primIdx + 2];

    const float3 v0 = vertex_buffer[v_0.x];
    const float3 v1 = vertex_buffer[v_1.x];
    const float3 v2 = vertex_buffer[v_2.x];
    const float area = length(cross(v1 - v0, v2 - v0));

    optix::Aabb *aabb = (optix::Aabb *) result;

    if (area > 0.0f && !isinf(area)) {
        aabb->m_min = fminf(fminf(v0, v1), v2);
        aabb->m_max = fmaxf(fmaxf(v0, v1), v2);
    } else {
        aabb->invalidate();
    }
}
