#pragma once
#include "types.h"
#include "utils.h"


inline void transpose(vec3* v1, vec3* v2, vec3* v3)
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


struct face {
	int iv[3];
	int ivt[3];
	int ivn[3];
	double a,b,c,d;
	double size_;
	vec3 size;
	vec3 center;
	vec3 normal;
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
	vec3 cp;
	vec3 normal;
	vec2 texcoord;

	int face;
	double distance;
	intersection(){distance = 1e16; face = -1;}
};
struct model{
	vec3* position;
	vec3* normal;
	vec2* texcoord;
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
	vec3 position;
	vec3 rotation;
	camera(vec3 position, vec3 rotation):position(position),rotation(rotation){};
};

struct voxel{
	std::vector<int> faces;
	vec3 boundMax;
	vec3 boundMin;
	vec3 center;
	vec3 size;
	bool last;
	bool empty;
	voxel* voxels[8];
	int id;
};

inline bool check(vec3 cp, vec3 v1, vec3 v2, vec3 planev)
{
	return glm::dot(glm::cross(v2 - v1, cp - v1), planev) > 0;
}

//fileio.cpp
int save2file(pixel* b,short w, short h, const char* path);
model* loadmodel(const char* path);
texture* loadtexture(const char* path);
atexture* loadatexture(const char* path);

//tracer
void makeBound(model* m, int triag, vec3& boundMax, vec3& boundMin);
bool triagInVoxel(model* m, int triag, voxel* v);
void sortTriags(model* m, voxel* root);
void createVoxels(voxel* voxels, int count);

inline bool IsRayInV(const voxel&vox, const vec3& v, const vec3& p);
void createFaceList(const vec3& v, const vec3& p, int* facelist, int &facelistCount, voxel** voxellist, int &voxellistCount);
void preparemodel(model* m, vec3& boundMax, vec3& boundMin);

intersection findintersection(model* m, vec3 v, vec3 p, int* facelist, int &facelistCount, voxel** voxellist, int &voxellistCount);


//get line equation coefficients
template<typename T>
inline void GetLine(T& a, T& b, T& c, const vec2& p1, const vec2& p2)
{
	a = p1.y - p2.y;
	b = p2.x - p1.x;
	c = p1.x * p2.y - p2.x * p1.y;
}
