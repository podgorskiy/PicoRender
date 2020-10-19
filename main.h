#pragma once
#include <float.h>
#include <math.h>
#include <glm/glm.hpp>

#define    maxFloat        FLT_MAX
#define    epsilon        FLT_EPSILON

#define pi 3.1415926535897932384626433832795

inline void transpose(glm::dvec3* v1, glm::dvec3* v2, glm::dvec3* v3)
{
	double tmp = v1->y;
	v1->y = v2->x;
	v2->x = tmp;
	tmp = v1->z;
	v1->z = v3->x;
	v3->x = tmp;
	tmp = v2->z;
	v2->z = v3->y;
	v3->y = tmp;
}

inline double saturate(const float& x){if(x<0) return 0; else if(x>1) return 1; else return x;}
/////////////////////////////////////////////////////////////////////////////////////////////////////////

struct pixel {
	unsigned char r,g,b,a;
	pixel():r(0),g(0),b(0),a(0){};
	pixel(char r, char g, char b, char a):r(r),g(g),b(b),a(a){};
};
struct pixel24 {
	unsigned char r,g,b;
	pixel24():r(0),g(0),b(0){};
	pixel24(char r, char g, char b):r(r),g(g),b(b){};
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
	void Set (const float& x){r=x;g=x;b=x;}
	void Set (const float& xr, const float& xg, const float& xb){r=xr;g=xg;b=xb;}
};
inline fpixel operator * (const float s, const fpixel& a){ return fpixel(s * a.r, s * a.g, s * a.b); }
inline fpixel operator * (const fpixel&a, const fpixel& b){ return fpixel(b.r * a.r, b.g * a.g, b.b * a.b); }


struct face {
	int iv[3];
	int ivt[3];
	int ivn[3];
	double a,b,c,d;
	double size_;
	glm::dvec3 size;
	glm::dvec3 center;
	glm::dvec3 normal;
	int key;
	face(){};
};

struct face_r {
	int iv[3];
	int ivt[3];
	int ivn[3];
	face_r(){};
};
struct intersection{
	glm::dvec3 cp;
	glm::dvec3 normal;
	glm::dvec2 texcoord;

	int face;
	double distance;
	intersection(){distance = 1e16; face = -1;}
};
struct model{
	glm::dvec3* position;
	glm::dvec3* normal;
	glm::dvec2* texcoord;
	face*   face_array;
	int iv, ivt, ivn, iface;
	model():position(NULL),normal(NULL),texcoord(NULL),face_array(NULL),iv(0),ivt(0),ivn(0),iface(0){};
	void setsize(int iv_, int ivt_, int ivn_, int iface_){iv = iv_; ivt = ivt_; ivn = ivn_; iface = iface_;};
};
struct texture{
	pixel24* buff;
	int width;
	int height;
};
struct atexture{
	pixel* buff;
	int width;
	int height;
};
struct camera{
	glm::dvec3 position;
	glm::dvec3 rotation;
	camera(glm::dvec3 position, glm::dvec3 rotation):position(position),rotation(rotation){};
};

struct voxel{
	std::vector<int> faces;
	glm::dvec3 boundMax;
	glm::dvec3 boundMin;
	glm::dvec3 center;
	glm::dvec3 size;
	bool last;
	bool empty;
	voxel* voxels[8];
	int id;
};

inline bool check(glm::dvec3 cp, glm::dvec3 v1, glm::dvec3 v2, glm::dvec3 planev)
{
	return glm::dot(glm::cross(v2 - v1, cp - v1), planev) > 0;
}

//fileio.cpp
int save2file(pixel* b,short w, short h, const char* path);
model* loadmodel(const char* path);
texture* loadtexture(const char* path);
atexture* loadatexture(const char* path);

//tracer
void makeBound(model* m, int triag, glm::dvec3& boundMax, glm::dvec3& boundMin);
bool triagInVoxel(model* m, int triag, voxel* v);
void sortTriags(model* m, voxel* root);
void createVoxels(voxel* voxels, int count);

inline bool IsRayInV(const voxel&vox, const glm::dvec3& v, const glm::dvec3& p);
void createFaceList(const glm::dvec3& v, const glm::dvec3& p, int* facelist, int &facelistCount, voxel** voxellist, int &voxellistCount);
void preparemodel(model* m, glm::dvec3& boundMax, glm::dvec3& boundMin);

intersection findintersection(model* m, glm::dvec3 v, glm::dvec3 p, int* facelist, int &facelistCount, voxel** voxellist, int &voxellistCount);


#define SB_GET_LINE(a,b,c,p1,p2)	\
	a = p1.y - p2.y; \
	b = p2.x - p1.x; \
	c = p1.x*p2.y - p2.x*p1.y;

#define SB_GET_CLOTHEST_POINT(a,b,c,p,cp) \
{\
	float32 c1 = b*p.x - a*p.y; \
	float32 det = a*a + b*b; \
	cp.Set(-(a*c - b*c1) / det, -(a*c1 + b*c) / det); \
}


#define SB_POINT_IN_SEGMENT(p1,p2,c) \
	(\
	(((p2.x >= c.x) && (c.x >= p1.x)) || \
	((p2.x <= c.x) && (c.x <= p1.x))) && \
	(((p2.y >= c.y) && (c.y >= p1.y)) || \
	((p2.y <= c.y) && (c.y <= p1.y)))  \
	) \

/* calc crossing point

a1x+b1y+c1=0
a2x+b2y+c2=0

-c1-b1y
x = ---------
a1
a2       a2
- ----c1 - ----b1y + b2y+c2 = 0
a1       a1

a2
---- c1  - c2
a1
y = -------------
a2
b2 - ---- b1
a1

a2c1 - a1c2
y = -------------
a1b2 - a2b1
*/

#define SB_GET_CROSSPOINT(a1,b1,c1,a2,b2,c2,l1p1,l1p2,l2p1,l2p2,cp,action) \
	SB_GET_LINE(a1, b1, c1, l1p1, l1p2)\
	SB_GET_LINE(a2, b2, c2, l2p1, l2p2)\
{\
	float tmp = a1*b2 - a2*b1; \
if (tmp == 0.0f)\
	action; \
	cp.Set((b1*c2 - b2*c1) / tmp, (a2*c1 - a1*c2) / tmp); \
}
