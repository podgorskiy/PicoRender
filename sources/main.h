#pragma once
#include "types.h"


template<typename T>
uint8_t *TOUINT8(const T* data, int size) {
    uint8_t *out = new uint8_t[size];
    for (int i = 0; i < size; ++i) {
        T x = data[i];
        if (x < 0.0f) x = 0.0f;
        if (x > 1.0f) x = 1.0f;
        out[i] = (uint8_t) (x * 255.0f);
    }
    return out;
}

inline vec2 make_float2(glm::vec2 v)
{
    vec2 t; t.x = v.x; t.y = v.y; return t;
}

inline vec3 make_float3(glm::vec3 v)
{
    vec3 t; t.x = v.x; t.y = v.y; t.z = v.z; return t;
}

inline vec4 make_float4(glm::vec4 v)
{
    vec4 t; t.x = v.x; t.y = v.y; t.z = v.z; t.w = v.w; return t;
}
