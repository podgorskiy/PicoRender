#pragma once
#include <float.h>
#include <math.h>
#define	epsilon		FLT_EPSILON

#define pi 3.1415926535897932384626433832795

struct _3DVec
{
	/// Default constructor does nothing (for performance).
	_3DVec() {};
	/// Construct using coordinates.
	_3DVec(double x, double y, double z) : x(x), y(y), z(z) {};	
	/// Set this vector to some specified coordinates.
	inline void Set(const double& x_, const double& y_, const double& z_) { x = x_; y = y_; z = z_;}
	/// Negate this vector.
	inline _3DVec operator -() const { _3DVec v; v.Set(-x, -y, -z); return v; }	
	/// Add a vector to this vector.
	inline void operator += (const _3DVec& v){	x += v.x; y += v.y; z += v.z;}	
	/// Subtract a vector from this vector.
	inline void operator -= (const _3DVec& v){	x -= v.x; y -= v.y; z -= v.z;}
	/// Multiply this vector by a scalar.
	inline void operator *= (const double& a){	x *= a; y *= a; z *=a;}
	/// Get the length of this vector (the norm).
	inline double Length() const{ return std::sqrt(x * x + y * y + z * z);}
	/// Get the length squared. 
	inline double Length2() const{ return x * x + y * y + z * z;}
	/// Convert this vector into a unit vector. Returns the length.
	inline void Normalize()
	{
		double length = Length();
		if (length > epsilon)
		{
			double invLength = 1.0f / length;
			x *= invLength;
			y *= invLength;
			z *= invLength;
		}
	}
	inline void rotatex(const double& a)
	{
		double tmp = cosf(a) * y - sinf(a) * z;
		z = sinf(a) * y + cosf(a) * z;
		y = tmp;
	}
	inline void rotatey(const double& a)
	{
		double tmp = cosf(a) * x + sinf(a) * z;
		z = - sinf(a) * x + cosf(a) * z;
		x = tmp;
	}
	inline void rotatez(const double& a)
	{
		double tmp = cosf(a) * x - sinf(a) * y;
		y = sinf(a) * x + cosf(a) * y;
		x = tmp;
	}
	double x, y, z;
};
/// Perform the dot product on two vectors.
inline double operator * (const _3DVec& a, const _3DVec& b){	return a.x * b.x + a.y * b.y + a.z * b.z;}
/// Perform the cross product on two vectors. In 2D this produces a scalar.
inline _3DVec Cross(const _3DVec& a, const _3DVec& b){ return _3DVec(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);}
/// Add two vectors component-wise.
inline _3DVec operator + (const _3DVec& a, const _3DVec& b){ return _3DVec(a.x + b.x, a.y + b.y, a.z + b.z);}
/// Subtract two vectors component-wise.
inline _3DVec operator - (const _3DVec& a, const _3DVec& b){ return _3DVec(a.x - b.x, a.y - b.y, a.z - b.z);}
inline _3DVec operator * (const double& s, const _3DVec& a){ return _3DVec(s * a.x, s * a.y, s * a.z);}
inline _3DVec operator * (const _3DVec& a, const double& s){ return _3DVec(s * a.x, s * a.y, s * a.z); }
inline _3DVec operator / (const _3DVec& a, const double& s){ return _3DVec(a.x / s, a.y / s, a.z / s); }
inline _3DVec operator / (const double& s, const _3DVec& a){ return _3DVec(s / a.x, s / a.y, s / a.z); }

inline void transpose(_3DVec* v1, _3DVec* v2, _3DVec* v3)
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
inline fpixel operator * (const fpixel& a, const float s){ return fpixel(s * a.r, s * a.g, s * a.b); }
inline fpixel operator * (const fpixel&a, const fpixel& b){ return fpixel(b.r * a.r, b.g * a.g, b.b * a.b); }

struct _2DVec {
	float x, y;
	_2DVec() :x(0), y(0){};
	_2DVec(float x, float y) :x(x), y(y){};
	void Set(const float& x_, const float& y_) { x = x_; y = y_; }
};
inline _2DVec operator * (const _2DVec& a, const double& s){ return _2DVec(s * a.x, s * a.y); }

struct face {
	int iv[3];
	int ivt[3];
	int ivn[3];
	double a,b,c,d;
	double size_;
	_3DVec size;
	_3DVec center;
	_3DVec normal;
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
	_3DVec cp;
	_3DVec normal;
	_2DVec texcoord;
	_3DVec veiw;
	int face;
	double distance;
	intersection(){distance = 1e16; face = -1;}
};
struct model{
	_3DVec* position;
	_3DVec* normal;
	_2DVec* texcoord;
	face*   face_array;
	int iv, ivt, ivn, iface;
	model():position(nullptr),normal(nullptr),texcoord(nullptr),face_array(nullptr),iv(0),ivt(0),ivn(0),iface(0){};
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
	_3DVec position;
	_3DVec rotation;
	camera(_3DVec position, _3DVec rotation):position(position),rotation(rotation){};
};

struct voxel{
	std::vector<int> faces;
	_3DVec boundMax;
	_3DVec boundMin;
	_3DVec center;
	_3DVec size;
	bool last;
	bool empty;
	voxel* voxels[8];
	int id;
};

inline bool check(_3DVec& cp, _3DVec& v1, _3DVec& v2, _3DVec& planev){
	return Cross(v2 - v1, cp - v1)*planev > 0;
}

//fileio.cpp
int save2file(pixel* b,short w, short h, const char* path);
model* loadmodel(const char* path);
texture* loadtexture(const char* path);
atexture* loadatexture(const char* path);

//tracer
void makeBound(model* m, int triag, _3DVec& boundMax, _3DVec& boundMin);
bool triagInVoxel(model* m, int triag, voxel* v);
void sortTriags(model* m, voxel* root);
void createVoxels(voxel* voxels, int count);

bool IsRayInV(const voxel& vox, const _3DVec& v, const _3DVec& p);
void createFaceList(_3DVec& v, _3DVec& p, int* facelist, int &facelistCount, voxel** voxellist, int &voxellistCount);
void preparemodel(model* m, _3DVec& boundMax, _3DVec& boundMin);
intersection findintersection(model* m, _3DVec v, _3DVec& p, int* facelist, int &facelistCount, voxel** voxellist, int &voxellistCount);

//random generator
namespace rnd
{
	uint32_t _rand();
	double rand();
	double rand(float a, float b);
}

//get line equation coefficients
template<typename T>
inline void GetLine(T& a, T& b, T& c, const _2DVec& p1, const _2DVec& p2)
{
	a = p1.y - p2.y;
	b = p2.x - p1.x;
	c = p1.x * p2.y - p2.x * p1.y;
}
