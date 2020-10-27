#include "types.h"
#include <optixu/optixu_matrix_namespace.h>
#include <optixu/optixu_aabb_namespace.h>

rtBuffer<float3> vertex_buffer;
rtBuffer<float3> normal_buffer;
rtBuffer<float2> texcoord_buffer;
rtBuffer<int3>   index_buffer;
rtBuffer<int>    material_buffer;

rtDeclareVariable(float3, texcoord,         attribute texcoord, );
rtDeclareVariable(float3, geometric_normal, attribute geometric_normal, );
rtDeclareVariable(float3, shading_normal,   attribute shading_normal, );

rtDeclareVariable(float3, hit_point,        attribute hit_point, );
rtDeclareVariable(int,    uv_pass,          attribute uv_pass, );

rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );


RT_PROGRAM void mesh_intersect_uv(int primIdx) {
    const int3 v_0 = index_buffer[3 * primIdx + 0];
    const int3 v_1 = index_buffer[3 * primIdx + 1];
    const int3 v_2 = index_buffer[3 * primIdx + 2];

    const vec3 _p0 = vertex_buffer[v_0.x];
    const vec3 _p1 = vertex_buffer[v_1.x];
    const vec3 _p2 = vertex_buffer[v_2.x];
    const vec3 p0 = make_float3(texcoord_buffer[v_0.z], 0.);
    const vec3 p1 = make_float3(texcoord_buffer[v_1.z], 0.);
    const vec3 p2 = make_float3(texcoord_buffer[v_2.z], 0.);

    // Intersect ray with triangle
    vec3 n;
    float t, beta, gamma;
    if (intersect_triangle(ray, p0, p1, p2, n, t, beta, gamma)) {

        if (rtPotentialIntersection(t)) {
            geometric_normal = normalize(cross(_p1 - _p0, _p2 - _p0));
            if (normal_buffer.size() == 0) {
                shading_normal = geometric_normal;
            } else {
                vec3 n0 = normal_buffer[v_0.y];
                vec3 n1 = normal_buffer[v_1.y];
                vec3 n2 = normal_buffer[v_2.y];
                shading_normal = normalize(n1 * beta + n2 * gamma + n0 * (1.0f - beta - gamma));
            }
            hit_point = _p1 * beta + _p2 * gamma + _p0 * (1.0f - beta - gamma);

            if (texcoord_buffer.size() == 0)
            {
                texcoord = make_float3(0.0f, 0.0f, 0.0f);
            } else {
                vec2 t0 = texcoord_buffer[v_0.z];
                vec2 t1 = texcoord_buffer[v_1.z];
                vec2 t2 = texcoord_buffer[v_2.z];
                texcoord = make_float3(t1 * beta + t2 * gamma + t0 * (1.0f - beta - gamma));
            }
            uv_pass = 1;

            rtReportIntersection(material_buffer[primIdx]);
        }
    }
}


RT_PROGRAM void mesh_bounds_uv(int primIdx, float result[6]) {
    const int3 v_0 = index_buffer[3 * primIdx + 0];
    const int3 v_1 = index_buffer[3 * primIdx + 1];
    const int3 v_2 = index_buffer[3 * primIdx + 2];

    vec3 v0 = make_float3(texcoord_buffer[v_0.z], 0.);
    vec3 v1 = make_float3(texcoord_buffer[v_1.z], 0.);
    vec3 v2 = make_float3(texcoord_buffer[v_2.z], 0.);

    const float area = length(cross(v1 - v0, v2 - v0));

    optix::Aabb *aabb = (optix::Aabb *) result;

    if (area > 0.0f && !isinf(area)) {
        aabb->m_min = fminf(fminf(v0, v1), v2);
        aabb->m_max = fmaxf(fmaxf(v0, v1), v2);
    } else {
        aabb->invalidate();
    }
}
