#pragma once

#include <glm/glm.hpp>


#define PI_F 3.141592654f

#define PI_D 3.14159265358979323846264338327950288


typedef glm::dvec3 vec3;
typedef glm::dvec2 vec2;
typedef glm::dvec3::value_type scalar;

//typedef glm::vec<4, uint8_t> pixel;
//typedef glm::vec<3, uint8_t> pixel24;
//typedef glm::vec<3, float> fpixel;

struct pixel {
	uint8_t r,g,b,a;
	pixel():r(0),g(0),b(0),a(0){};
	pixel(char r, char g, char b, char a):r(r),g(g),b(b),a(a){};
};
struct pixel24 {
	uint8_t r,g,b;
	pixel24():r(0),g(0),b(0){};
	pixel24(uint8_t r, uint8_t g, uint8_t b):r(r),g(g),b(b){};
};

struct fpixel {
	float r,g,b;
	fpixel():r(0),g(0),b(0){};
	fpixel(float r, float g, float b):r(r),g(g),b(b){};
	fpixel(pixel p) : r(p.r/255.0f), g(p.g/255.0f), b(p.b/255.0f) {}
	fpixel(pixel24 p) : r(p.r/255.0f), g(p.g/255.0f), b(p.b/255.0f) {}
	inline void operator += (const fpixel& v){	r += v.r; g += v.g; b += v.b;}
	inline void operator += (const float& v){	r += v; g += v; b += v;}
	inline void operator *= (const float& a){	r *= a; g *= a; b *=a;}
	inline void operator *= (const fpixel& p){	r *= p.r; g *= p.g; b *=p.b;}

};
inline fpixel operator * (const float s, const fpixel& a){ return fpixel(s * a.r, s * a.g, s * a.b); }
inline fpixel operator * (const fpixel&a, const fpixel& b){ return fpixel(b.r * a.r, b.g * a.g, b.b * a.b); }
