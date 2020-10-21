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
