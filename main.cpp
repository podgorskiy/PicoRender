#include <stdlib.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>
#include <vector>
#include <time.h>
#include <thread>

#include "main.h"
#include "ConfigReader.h"

#define THREADCOUNT 8

//////////////////////////////
short width;
short height;
short voxelCount;
short sampleCount;
float lightDirX;
float lightDirY;
float lightDirZ;
int bounce;
bool renderInTex;
///////////////////////////////

voxel* rvoxel;
int voxelsCount;
int lastVoxelCount;
int everageFaceInVoxel;
int maxFaceInVoxel;

voxel* voxelPool;
int voxelPoolCount;

inline voxel* NewVoxel()
{
	return &voxelPool[voxelPoolCount++];
}

Config* config;

struct exeArg
{
	model* m;
	int offset;
	int count;
	int height;
	int iterk;

	int lightmapSize;

	fpixel* fpixelbuff_albido;
	_3DVec* fpixelbuff_normal;
	float* fpixelbuff_depth;
	fpixel** fpixelbuff_gi_by_bounce;
	fpixel* fpixelbuff_gi;
	_3DVec* fpixelbuff_gi_normal;
	int*	fpixelbuff_count;
	
	texture* tex;

	_3DVec boundMax;
	_3DVec boundMin;

	exeArg(){};
	int* facelist;
	voxel** voxellist;
};


_3DVec lambertNoTangent(_3DVec normal)
{
    _2DVec uv(rnd::rand(), rnd::rand());
    float theta = 2.0 * pi * uv.x;
    uv.y = 2.0 * uv.y - 1.0;
    _2DVec p = _2DVec(cos(theta), sin(theta)) * sqrt(1.0 - uv.y * uv.y);
    _3DVec spherePoint = _3DVec(p.x, p.y, uv.y);
    spherePoint += normal;
    spherePoint.Normalize();
    return spherePoint;
}

inline void Trace(_3DVec pos, _3DVec v, _3DVec normal, int i, int j, exeArg* arg)
{
	model* m = arg->m;
	int* facelist = arg->facelist;
	int facelistCount;
	voxel** voxellist = arg->voxellist;
	int voxellistCount;

	fpixel light(0.0,0.0,0.0);
	fpixel color(1.0,1.0,1.0);
	fpixel albido(1.0,1.0,1.0);
	_3DVec bent_normal(0.0, 0.0, 0.0);
	fpixel attenuation;

	intersection is;
	is.cp = pos;

	int k = 0;
	for (; k < bounce; k++)
	{
		if (renderInTex && k == 0)  //we assume that we have point of intersection of imaginary ray with surface
		{
			is.face = 0;
			is.normal = normal;
			is.texcoord.x = (float)(j) / width;
			is.texcoord.y = (float)(i) / width;
		}
		else
		{
			is = findintersection(m, v, is.cp, facelist, facelistCount, voxellist, voxellistCount);
		}
		if (is.face != -1)
		{
			is.normal.Normalize();
			int uv_u = ((int) (is.texcoord.x * arg->tex->width)) % arg->tex->width;
			int uv_v = ((int) (is.texcoord.y * arg->tex->height)) % arg->tex->height;
			attenuation = arg->tex->buff[uv_u + uv_v * arg->tex->width];

			v = lambertNoTangent(is.normal);

			if (k == 0)
			{
				albido = attenuation;
				attenuation = fpixel(1.0,1.0,1.0);
				bent_normal = v;
			}

            if (color.r > 0.01 || color.g > 0.01 || color.b > 0.01)
                color = color * attenuation;
            else
            {
                color = fpixel(0.0f, 0.0f, 0.0f);
                break;
            }
		}
		else
		{
			light = fpixel(1.0, 1.0, 1.0);
			break;
		}
	}

	if (k != 0) // was hit
	{
		fpixel lightColor = light * color;
		float magnitude = sqrt(lightColor.r + lightColor.g + lightColor.b) / 3.0f;

		arg->fpixelbuff_normal[i * width + j] += normal;
		arg->fpixelbuff_albido[i * width + j] += albido;

		arg->fpixelbuff_gi_normal[i * width + j] += bent_normal * magnitude;
		if (k != bounce)
			arg->fpixelbuff_gi_by_bounce[k][i * width + j] += lightColor;
		arg->fpixelbuff_gi[i * width + j] += lightColor;
		arg->fpixelbuff_count[i * width + j] += 1;
	}
}

typedef void(*function)(_3DVec pos, _3DVec normal, _3DVec l, int i, int j, int iterk, exeArg* arg, int treug);

void renderTriangle(model* m, int lightmapSize, int i, int iterk, exeArg *arg, function);

void renderFunction(_3DVec cp, _3DVec n, _3DVec l, int i_x, int i_y, int iterk, exeArg* arg, int treug)
{
	for (int i = 0; i < iterk; i++)
	{
		Trace(cp, _3DVec(0, 0, 0), n, i_y, i_x, arg);
	}
}

bool renderInTexRutine(exeArg* arg)
{
	printf("+");
	fflush(stdout);
	int progress = 0;
	for (int i = arg->offset; i < arg->count; i += THREADCOUNT)
	{
		if (((i)* 80 / THREADCOUNT) / (arg->count) >progress){
			progress++;
			printf(".");
			fflush(stdout);
		}
		renderTriangle(arg->m, arg->lightmapSize, i, arg->iterk, arg, renderFunction);
	}
	printf("-");
	fflush(stdout);
	return true;
}


void floatToIntF(_2DVec in, int size, double &x, double &y)
{
	double half = 1.0 / static_cast<double>(size) / 2.0;
	x = (in.x - half) * static_cast<double>(size);
	y = (in.y - half) * static_cast<double>(size);
}
void floatToInt(_2DVec in, int size, int &x, int &y)
{
	double xf, yf;
	floatToIntF(in, size, xf, yf);
	x = static_cast<int>(lround(xf));
	y = static_cast<int>(lround(yf));
}
void intToFloat(_2DVec &in, int size, int x, int y)
{
	double half = 1.0 / static_cast<double>(size) / 2.0;
	in.x = x / static_cast<double>(size)+half;
	in.y = y / static_cast<double>(size)+half;
}


void renderTriangle(model* m, int lightmapSize, int i, int iterk, exeArg *arg, function f)
{
	_2DVec v[3];
	for (int k = 0; k < 3; k++)
	{
		v[k] = m->texcoord[m->face_array[i].ivt[k]];
	}
	_3DVec v3[3];
	for (int k = 0; k < 3; k++)
	{
		v3[k] = m->position[m->face_array[i].iv[k]];
	}

	_3DVec normal[3];
	for (int k = 0; k < 3; k++)
	{
		normal[k] = m->normal[m->face_array[i].ivn[k]];
		normal[k].Normalize();
	}

	//sorting by y coord
	for (int iter = 0; iter < 4; iter++)
	{
		int it = iter % 2;
		if (v[it].y < v[it + 1].y)
		{
			_2DVec tmp = v[it];
			v[it] = v[it + 1];
			v[it + 1] = tmp;
			{
				_3DVec tmp = v3[it];
				v3[it] = v3[it + 1];
				v3[it + 1] = tmp;
			}
			{
				_3DVec tmp = normal[it];
				normal[it] = normal[it + 1];
				normal[it + 1] = tmp;
			}
		}
	}

	double ymax = v[0].y;
	double ymid = v[1].y;
	double ymin = v[2].y;
	double line1l[3];
	double line1r[3];
	double line2l[3];
	double line2r[3];
	{
		double tmpx = v[1].x;
		double a, b, c;
		GetLine(a, b, c, v[0], v[2]);
		double x = (-c - b * ymid) / a;
		if (x < 0 || x> 1.0)
			return;
		_2DVec vc(x, ymid);
		if (tmpx < x)
		{
			GetLine(line1l[0], line1l[1], line1l[2], v[0], v[1]);
			GetLine(line1r[0], line1r[1], line1r[2], v[0], vc);
			GetLine(line2l[0], line2l[1], line2l[2], v[1], v[2]);
			GetLine(line2r[0], line2r[1], line2r[2], vc, v[2]);
		}
		else{
			GetLine(line1l[0], line1l[1], line1l[2], v[0], vc);
			GetLine(line1r[0], line1r[1], line1r[2], v[0], v[1]);
			GetLine(line2l[0], line2l[1], line2l[2], vc, v[2]);
			GetLine(line2r[0], line2r[1], line2r[2], v[1], v[2]);
		}
	}
	for (int part = 0; part < 2; part++)
	{
		double ytop, ybottom;
		double linel[3];
		double liner[3];
		if (part == 0)
		{
			ytop = ymax;
			ybottom = ymid;
			for (int k = 0; k < 3; k++)
			{
				linel[k] = line1l[k];
				liner[k] = line1r[k];
			}
		}
		else
		{
			ytop = ymid;
			ybottom = ymin;
			for (int k = 0; k < 3; k++)
			{
				linel[k] = line2l[k];
				liner[k] = line2r[k];
			}
		}
		if (ytop < ybottom)
			continue;

		double f_ytop, f_ybottom, tmp;
		floatToIntF(_2DVec(0, ytop), lightmapSize, tmp, f_ytop);
		floatToIntF(_2DVec(0, ybottom), lightmapSize, tmp, f_ybottom);

		int i_ytop = (int)round(f_ytop);
		int i_ybottom = (int)round(f_ybottom);

		for (int i_y = i_ytop; i_y >= i_ybottom; i_y--)
		{
			_2DVec v_;
			intToFloat(v_, lightmapSize, 0, i_y);
			double d_y = v_.y;

			d_y = d_y < ybottom + 1e-6 ? ybottom + 1e-4 : d_y;
			d_y = d_y > ytop - 1e-6 ? ytop - 1e-4 : d_y;
			double d_xl = (-linel[2] - linel[1] * d_y) / linel[0];
			double d_xr = (-liner[2] - liner[1] * d_y) / liner[0];

			double f_xl, f_xr, tmp;
			floatToIntF(_2DVec(d_xl, 0), lightmapSize, f_xl, tmp);
			floatToIntF(_2DVec(d_xr, 0), lightmapSize, f_xr, tmp);

			int i_xl = (int)round(f_xl);
			int i_xr = (int)round(f_xr);

			for (int i_x = i_xl; i_x <= i_xr; i_x++)
			{
				if (i_x >= 0 && i_x <= (double)lightmapSize && i_y >= 0 && i_y <= lightmapSize)
				{
					_2DVec v_;
					intToFloat(v_, lightmapSize, i_x, 0);
					double d_x = v_.x;

					d_x = d_x < d_xl+1e-4  ? d_xl+1e-4 : d_x;
					d_x = d_x > d_xr-1e-4 ? d_xr-1e-4 : d_x;

					//calc barycentric coordinates
					double T[2][2];
					double iT[2][2];
					T[0][0] = v[0].x - v[2].x;	T[0][1] = v[1].x - v[2].x;
					T[1][0] = v[0].y - v[2].y;	T[1][1] = v[1].y - v[2].y;
					double d = T[0][0] * T[1][1] - T[0][1] * T[1][0];
					iT[0][0] = T[1][1] / d;	iT[0][1] = -T[0][1] / d;
					iT[1][0] = -T[1][0] / d;	iT[1][1] = T[0][0] / d;

					double lambda0 = iT[0][0] * (d_x - v[2].x) + iT[0][1] * (d_y - v[2].y);
					double lambda1 = iT[1][0] * (d_x - v[2].x) + iT[1][1] * (d_y - v[2].y);
					double lambda2 = 1.0 - lambda0 - lambda1;

					double x = v3[0].x * lambda0 + v3[1].x * lambda1 + v3[2].x * lambda2;
					double y = v3[0].y * lambda0 + v3[1].y * lambda1 + v3[2].y * lambda2;
					double z = v3[0].z * lambda0 + v3[1].z * lambda1 + v3[2].z * lambda2;

					double nx = normal[0].x * lambda0 + normal[1].x * lambda1 + normal[2].x * lambda2;
					double ny = normal[0].y * lambda0 + normal[1].y * lambda1 + normal[2].y * lambda2;
					double nz = normal[0].z * lambda0 + normal[1].z * lambda1 + normal[2].z * lambda2;

					_3DVec cp(x, y, z);
					_3DVec l(lambda0, lambda1, lambda2);
					_3DVec n(nx, ny, nz);
					n.Normalize();
					f(cp, n, l, i_x, i_y, iterk, arg, i);
				}
			}
		}
	}
}


bool renderRutine(exeArg *arg)
{
	printf("+");
	fflush(stdout);

	int progress = 0;

	_3DVec boundMax = arg->boundMax;
	_3DVec boundMin = arg->boundMin;

	for (int i = arg->offset; i<arg->height; i += THREADCOUNT){
		if (((i)* 80 / THREADCOUNT) / (arg->height) >progress){
				progress++;
				printf(".");
				fflush(stdout);
			}
			float betta = (float)i / (float)(height - 1) * boundMax.y + (height - i - 1) / (float)(height - 1) * boundMin.y;
			for (int j = 0; j<width; j++){
				float alpha = (float)j / (float)(width - 1) * boundMax.x + (width - j - 1) / (float)(width - 1) * boundMin.x;
				int iterK = sampleCount;
				for (int k = 0; k < iterK; k++)
				{
					float dx = rnd::rand(-0.5, 0.5) * 1.1f;
					float dy = rnd::rand(-0.5, 0.5) * 1.1f;
					_3DVec pos(alpha + dx * (boundMax.x - boundMin.x) / width,
					           betta + dy * (boundMax.y - boundMin.y) / height, -1000.0);
					_3DVec v(0, 0, 1.0f);
					Trace(pos, v, _3DVec(1.0f, 0.0f, 0.0f), i, j, arg);
				}
			}
		}
		printf("-");
		fflush(stdout);
		return true;
}


inline float clamp(float x)
{
	if (x > 1.0f) return 1.0f;
	return x;
}

uint8_t to8bit(float x)
{
	x *= 255.;
	x = x < 0 ? 0. : x;
	int r = int(round(x));
	return r > 255 ? 255 : (r < 0 ? 0 : r);
}

float gamma(float x)
{
	return pow(x, 1. / 2.2);
}

int main(int argc, char* argv[])
{
	int startTime = time(nullptr);
	printf("reading data\n");

	config = new Config;
	config->LoadConfig("config.txt");
	width = config->GetField("width")->GetInt();
	height = config->GetField("height")->GetInt();
	voxelCount = config->GetField("voxelCount")->GetInt();
	lightDirX = config->GetField("lightDirX")->GetFloat();
	lightDirY = config->GetField("lightDirY")->GetFloat();
	lightDirZ = config->GetField("lightDirZ")->GetFloat();
	sampleCount = config->GetField("sampleCount")->GetInt();
	bounce = config->GetField("bounce")->GetInt();
	renderInTex = config->GetField("renderInTex")->GetBool();
	printf("Sample count:%d\n", sampleCount);

	const char* obj_str = config->GetField("obj")->GetStr();
	const char* tex_str = config->GetField("texture")->GetStr();
	const char* outfile_prefix = config->GetField("outfile_prefix")->GetStr();

	model* m = loadmodel(obj_str);
	texture* tex =  loadtexture(tex_str);

	_3DVec boundMax;
	_3DVec boundMin;

	printf("prepareing model\n");
	printf("number of faces: %d\n",m->iface);
	preparemodel(m,boundMax,boundMin);
	
	rvoxel = new voxel;
	rvoxel->last = false;
	rvoxel->empty = false;
	rvoxel->boundMax = boundMax;
	rvoxel->boundMin = boundMin;
	rvoxel->center = (rvoxel->boundMin + rvoxel->boundMax)/2.0f;
	rvoxel->size = (rvoxel->boundMax - rvoxel->boundMin)/2.0f;

	int vcount = pow(2,voxelCount);
	vcount = pow(vcount,3);
	voxelPool = new voxel[2*vcount];
	voxelPoolCount = 0;

	printf("generating voxels\n");
	createVoxels(rvoxel,voxelCount);

	sortTriags(m,rvoxel);

	float boundaspect = ((float) boundMin.x - (float) boundMax.x) / ((float) boundMin.y - (float) boundMax.y);
	if (boundaspect > ((float) width / (float) height))
	{
		float boundys = (float) height / (float) width * ((float) boundMax.x - (float) boundMin.x);
		float bc = (boundMin.y + boundMax.y) / 2.0f;
		boundMin.y = bc - boundys / 2.0f;
		boundMax.y = bc + boundys / 2.0f;
	}
	else
	{
		float boundxs = (float) width / (float) height * ((float) boundMax.y - (float) boundMin.y);
		float bc = (boundMin.x + boundMax.x) / 2.0f;
		boundMin.x = bc - boundxs / 2.0f;
		boundMax.x = bc + boundxs / 2.0f;
	}

	int*	fpixelbuff_count = new int[2 * width*height];
	fpixel* fpixelbuff_albido = new fpixel[2 * width*height];
	_3DVec* fpixelbuff_normal = new _3DVec[2 * width*height];
	float* fpixelbuff_depth = new float[2 * width*height];
	_3DVec* fpixelbuff_gi_normal = new _3DVec[2 * width*height];
	fpixel* fpixelbuff_gi = new fpixel[2 * width*height];
	fpixel** fpixelbuff_gi_by_bounce = new fpixel*[bounce];

	for (int i = 0; i < bounce; i++)
	{
		fpixelbuff_gi_by_bounce[i] = new fpixel[2 * width*height];
	}

	for(int i=0;i<height;i++){
		for (int j = 0; j<width; j++){
			for (int k = 0; k < bounce; k++)
			{
				fpixelbuff_gi_by_bounce[k][i * width + j].Set(0.0f);
			}
			fpixelbuff_gi_normal[i * width + j].Set(0, 0, 0);
			fpixelbuff_gi[i * width + j].Set(0, 0, 0);
			fpixelbuff_normal[i * width + j].Set(0, 0, 0);
			fpixelbuff_depth[i * width + j] = 0;
			fpixelbuff_albido[i * width + j].Set(0, 0, 0);
		}
	}

	printf("begin tracing\n");
	int traceStartTime = time(NULL);


	std::vector<std::thread> aThreads;

	exeArg args[THREADCOUNT];

	for (int i = 0; i < THREADCOUNT; i++)
	{
		args[i].boundMax = boundMax;
		args[i].boundMin = boundMin;
		args[i].offset = i ;
		args[i].m = m;
		args[i].tex = tex;
		args[i].count = m->iface;
		args[i].iterk = sampleCount;
		args[i].lightmapSize = height;
		args[i].fpixelbuff_count = fpixelbuff_count;
		args[i].fpixelbuff_albido = fpixelbuff_albido;
		args[i].fpixelbuff_normal = fpixelbuff_normal;
		args[i].fpixelbuff_depth = fpixelbuff_depth;
		args[i].fpixelbuff_gi = fpixelbuff_gi;
		args[i].fpixelbuff_gi_normal = fpixelbuff_gi_normal;
		args[i].fpixelbuff_gi_by_bounce = fpixelbuff_gi_by_bounce;
		args[i].height = height;
		args[i].voxellist = new voxel*[lastVoxelCount * 2];
		args[i].facelist = new int[m->iface * 2];

		if (!renderInTex)
		{
			aThreads.emplace_back(renderRutine, &args[i]);
		}
		else
		{
			aThreads.emplace_back(renderInTexRutine, &args[i]);
		}
	}

	for (auto& thread: aThreads)
	{
		thread.join();
	}

	printf("\nend tracing\n");
	traceStartTime=time(nullptr) - traceStartTime;
	printf("writing data\n");

	pixel* pixelbuff;
	pixelbuff = new pixel[width*height];

	//nomal
	for(int i=0;i<height;i++){
		for(int j=0;j<width;j++){
			int c = fpixelbuff_count[i * width + j];
			if (c > 0)
			{
				_3DVec normal = fpixelbuff_normal[i * width + j] / c;
				normal.Normalize();
				pixelbuff[i * width + j].r = to8bit((normal.x + 1.0f) / 2.0f);
				pixelbuff[i * width + j].g = to8bit((normal.y + 1.0f) / 2.0f);
				pixelbuff[i * width + j].b = to8bit((normal.z + 1.0f) / 2.0f);
			}
			pixelbuff[i * width + j].a = to8bit(float(c) / sampleCount);
		}
	}
	save2file(pixelbuff, width, height, (outfile_prefix + std::string("_normal.tga")).c_str());

	//nomal
	for(int i=0;i<height;i++){
		for(int j=0;j<width;j++){
			int c = fpixelbuff_count[i * width + j];
			if (c > 0)
			{
				pixelbuff[i * width + j].r = to8bit(fpixelbuff_albido[i * width + j].r / c);
				pixelbuff[i * width + j].g = to8bit(fpixelbuff_albido[i * width + j].g / c);
				pixelbuff[i * width + j].b = to8bit(fpixelbuff_albido[i * width + j].b / c);
			}
			pixelbuff[i * width + j].a = to8bit(float(c) / sampleCount);
		}
	}
	save2file(pixelbuff, width, height, (outfile_prefix + std::string("_albido.tga")).c_str());

	//gi normal
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			int c = fpixelbuff_count[i * width + j];
			if (c > 0)
			{
				_3DVec ginormal = fpixelbuff_gi_normal[i * width + j] / c;
				ginormal.Normalize();
				fpixel gi = fpixelbuff_gi[i * width + j];
				float alpha = (gi.r + gi.g + gi.b) / 3.0;
				pixelbuff[i * width + j].r = to8bit((ginormal.x + 1.0f) / 2.0f);
				pixelbuff[i * width + j].g = to8bit((ginormal.y + 1.0f) / 2.0f);
				pixelbuff[i * width + j].b = to8bit((ginormal.z + 1.0f) / 2.0f);
				pixelbuff[i * width + j].a = to8bit(alpha);
			}
			else
			{
				pixelbuff[i * width + j] = pixel();
			}
		}
	}
	save2file(pixelbuff, width, height, (outfile_prefix +  std::string("_gi_normal.tga")).c_str());

	//gi
	for(int i=0;i<height;i++){
		for(int j=0;j<width;j++){
			int c = fpixelbuff_count[i * width + j];
			if (c > 0)
			{
				pixelbuff[i * width + j].r = to8bit(fpixelbuff_gi[i * width + j].r / c);
				pixelbuff[i * width + j].g = to8bit(fpixelbuff_gi[i * width + j].g / c);
				pixelbuff[i * width + j].b = to8bit(fpixelbuff_gi[i * width + j].b / c);
			}
			pixelbuff[i * width + j].a = to8bit(float(c) / sampleCount);
		}
	}
	save2file(pixelbuff, width, height, (outfile_prefix + std::string("_gi.tga")).c_str());

	for (int k = 0; k < bounce;k++)
	{
		char name_[255];

		//gi
		for (int i = 0; i < height; i++){
			for (int j = 0; j < width; j++){
				int c = fpixelbuff_count[i * width + j];
				if (c > 0)
				{
					pixelbuff[i * width + j].r = to8bit(fpixelbuff_gi_by_bounce[k][i * width + j].r / c);
					pixelbuff[i * width + j].g = to8bit(fpixelbuff_gi_by_bounce[k][i * width + j].g / c);
					pixelbuff[i * width + j].b = to8bit(fpixelbuff_gi_by_bounce[k][i * width + j].b / c);
				}
				pixelbuff[i * width + j].a = to8bit(float(c) / sampleCount);
			}
		}
		std::string name("_gi_by_bounce%d.tga");
		sprintf(name_, name.c_str(), k);
		name = name_;
		save2file(pixelbuff, width, height, (outfile_prefix + name).c_str());
	}

	printf("\nDone\ntotal time is %d\n", int(time(nullptr)-startTime));
	printf("trace time is %d\n",traceStartTime);
	printf("[press any key]\n");
	getchar();
}

intersection findintersection(model* m, _3DVec v, _3DVec& p, int* facelist, int &facelistCount, voxel** voxellist, int &voxellistCount){
	v.Normalize();
	intersection r; 
	r.face=-1;
	r.distance = 1e6;
	int key = rnd::_rand();
	face* fa = m->face_array;
	createFaceList(v, p, facelist, facelistCount, voxellist, voxellistCount);
	for (int i_=0;i_<facelistCount;i_++){
		int i = facelist[i_];
		face& f = fa[i];
		if (f.key != key){
			f.key = key;
			//find distanse
			double t = v * (f.center - p);
			double dist = (v * t - f.center + p).Length2();
			if (dist<f.size_)
			{
				_3DVec planev = _3DVec(f.a, f.b, f.c);
				double tmp = planev * v;
				if (tmp != 0){
					double t = (-planev * p - f.d) / tmp;
					if (t>0.000001f){
						_3DVec cp = p + v * t;
						_3DVec v1 = m->position[m->face_array[i].iv[0]];
						_3DVec v2 = m->position[m->face_array[i].iv[1]];
						_3DVec v3 = m->position[m->face_array[i].iv[2]];
						bool ch1 = check(cp, v1, v2, planev);
						bool ch2 = check(cp, v2, v3, planev);
						bool ch3 = check(cp, v3, v1, planev);
						if ((ch1&ch2&ch3)){
							if (t<r.distance){
								r.distance = t;
								r.face = i;
								r.cp = cp;
								r.normal = f.normal;
							}
						}
					}
				}
			}
		}
	}
	if (r.face!=-1){
		_3DVec pn = m->face_array[r.face].normal;
		_3DVec v1 = m->position[m->face_array[r.face].iv[0]];
		_3DVec v2 = m->position[m->face_array[r.face].iv[1]];
		_3DVec v3 = m->position[m->face_array[r.face].iv[2]];
		v2-=v1;
		v3-=v1;
		_3DVec lcp = r.cp - v1;
		_3DVec b1 = v2;
		_3DVec b2 = Cross(b1,pn);
		b1.Normalize();
		b2.Normalize();
		v2 = _3DVec(b1*v2,b2*v2,0);
		v3 = _3DVec(b1*v3,b2*v3,0);
		lcp = _3DVec(1,b1*lcp,b2*lcp);
		float id = 1/(v2.x*v3.y - v3.x*v2.y);
		_3DVec c1(1, 0, 0);
		_3DVec c2( (v2.y - v3.y)*id,  v3.y*id, - v2.y*id);
		_3DVec c3(-(v2.x - v3.x)*id, -v3.x*id,   v2.x*id);
		///////////////////////////////////////////////////////
		_3DVec n1 = m->normal[m->face_array[r.face].ivn[0]];   
		_3DVec n2 = m->normal[m->face_array[r.face].ivn[1]];   
		_3DVec n3 = m->normal[m->face_array[r.face].ivn[2]];
		transpose(&n1,&n2,&n3);
		_3DVec ax(c1*n1,c2*n1,c3*n1);
		_3DVec ay(c1*n2,c2*n2,c3*n2);
		_3DVec az(c1*n3,c2*n3,c3*n3);
		r.normal = _3DVec(ax*lcp,ay*lcp,az*lcp);
		///////////////////////////////////////////////////////
		_2DVec t1 = m->texcoord[m->face_array[r.face].ivt[0]];   
		_2DVec t2 = m->texcoord[m->face_array[r.face].ivt[1]]; 
		_2DVec t3 = m->texcoord[m->face_array[r.face].ivt[2]];
		_3DVec tu(t1.x,t2.x,t3.x);
		_3DVec tv(t1.y,t2.y,t3.y);
		_3DVec au(c1*tu,c2*tu,c3*tu);
		_3DVec av(c1*tv,c2*tv,c3*tv);
		r.texcoord = _2DVec(au*lcp,av*lcp);			   
	}
	return r;
}

void CreateVoxelList(voxel* vox, const _3DVec& v, const _3DVec& p, int* c, voxel** voxellist)
{
	if (IsRayInV(*vox, v, p))
	{
		if(vox->last)
		{
			voxellist[(*c)++] = vox;
		}
		else
		{
			for(int i=0;i<8;i++)
			{
				if(!vox->voxels[i]->empty)
					CreateVoxelList(vox->voxels[i], v, p, c, voxellist);
			}
		}
	}
}

void createFaceList(_3DVec& v, _3DVec& p, int* facelist, int &facelistCount, voxel** voxellist, int &voxellistCount)
{
	facelistCount = 0;
	voxellistCount = 0;
	CreateVoxelList(rvoxel, v, p, &voxellistCount, voxellist);
	//sorting voxels
	bool changed = true;
	while(changed){
		changed = false;
		for( int i = 0; i <voxellistCount-1; i++)
		{
			if((voxellist[i]->center - p).Length2()>(voxellist[i+1]->center - p).Length2())
			{
				voxel* tmp = voxellist[i];
				voxellist[i] = voxellist[i+1];
				voxellist[i+1] = tmp;
				changed = true;
			}
		}
	}
	for( int i = 0; i <voxellistCount; i++)
	{
		voxel* v = voxellist[i];
		for(int j = 0, l = v->faces.size(); j<l; j++ )
		{
			facelist[facelistCount++] = v->faces[j];
		}
	}
}

inline bool IsRayInV(const voxel& vox, const _3DVec& v, const _3DVec& p)
{
	const _3DVec size = vox.size;
	const _3DVec p_ = vox.center - p;
	const _3DVec p_p_s = 1.0 / (p_ + size);
	const _3DVec p_m_s = 1.0 / (p_ - size);
	const double kx = p_p_s.x*v.x;
	const double kx_ = p_m_s.x*v.x;
	const double ky = p_p_s.y*v.y;
	const double ky_ = p_m_s.y*v.y;
	const double kz = p_p_s.z*v.z;
	const double kz_ = p_m_s.z*v.z;
	if ((abs(v.y - p_.y*kx) <= size.y*kx && abs(v.z - p_.z*kx) <= size.z*kx) || 
		(abs(v.y - p_.y*kx_) <= size.y*kx_ && abs(v.z - p_.z*kx_) <= size.z*kx_) ||
		(abs(v.x - p_.x*ky) <= size.x*ky && abs(v.z - p_.z*ky) <= size.z*ky) ||
		(abs(v.x - p_.x*ky_) <= size.x*ky_ && abs(v.z - p_.z*ky_) <= size.z*ky_) ||
		(abs(v.y - p_.y*kz) <= size.y*kz && abs(v.x - p_.x*kz) <= size.x*kz) ||
		(abs(v.y - p_.y*kz_) <= size.y*kz_ && abs(v.x - p_.x*kz_) <= size.x*kz_))
	{
		return true;
	}
	return false;
}

bool makeTreeNodeEmpty(voxel* v)
{
	if(v->last == true)
	{
		v->empty = v->faces.size() == 0;
		return v->empty;
	}
	else
	{
		bool empty = true;
		for(int i=0;i<8;i++)
		{
			if(!makeTreeNodeEmpty(v->voxels[i]))
				empty = false;
		}
		v->empty = empty;
		return empty;
	}
}

void Clean(voxel* v)
{
	if (v->last != true && v->empty != true)
	{
		if (v->faces.size() < 4)
		{
			v->last = true;
		}
		for (int i = 0; i < 8; i++)
		{
			Clean(v->voxels[i]);
		}
	}
}

void Statistics(voxel* v, int& voxelsCount, int& lastVoxelCount, int& everageFaceInVoxel, int& maxFaceInVoxel)
{
	voxelsCount++;
	if (v->last)
	{
		int c = v->faces.size();
		if (c != 0)
		{
			if (c > maxFaceInVoxel)
			{
				maxFaceInVoxel = c;
			}
			everageFaceInVoxel += c;
			lastVoxelCount++;
		}
	}
	else
	{
		for (int i = 0; i<8; i++)
		{
			Statistics(v->voxels[i], voxelsCount, lastVoxelCount, everageFaceInVoxel, maxFaceInVoxel);
		}
	}
}

void checkTriagInVoxel(model* m,int id,voxel* v)
{
	if( triagInVoxel(m, id, v ) )
	{
		v->faces.push_back(id);
		if(v->last)
		{
			v->empty = false;
		}
		else
		{
			for(int i=0;i<8;i++)
			{
				checkTriagInVoxel(m,id,v->voxels[i]);
			}
		}
	}
}

void sortTriags(model* m, voxel* root)
{
	for (int i=0;i<m->iface;i++){
		_3DVec boundMax;
		_3DVec boundMin;
		makeBound(m,i,boundMax,boundMin);
		m->face_array[i].center = (boundMin + boundMax) / 2.0f;
		m->face_array[i].size = (boundMax - boundMin) / 2.0f;
		_3DVec v1 = m->position[m->face_array[i].iv[0]];
		_3DVec v2 = m->position[m->face_array[i].iv[1]];
		_3DVec v3 = m->position[m->face_array[i].iv[2]];
		double l1 = (v1 - m->face_array[i].center).Length();
		double l2 = (v2 - m->face_array[i].center).Length();
		double l3 = (v3 - m->face_array[i].center).Length();
		l1 = (l1>l2)?l1:l2;
		m->face_array[i].size_ = (l1>l3)?l1*l1:l3*l3;
		checkTriagInVoxel(m,i,root);
	}
	makeTreeNodeEmpty(rvoxel);
	Clean(rvoxel);
	voxelsCount = 0;
	lastVoxelCount = 0;
	everageFaceInVoxel = 0;
	maxFaceInVoxel = 0;
	Statistics(rvoxel, voxelsCount, lastVoxelCount, everageFaceInVoxel, maxFaceInVoxel);
	everageFaceInVoxel /= lastVoxelCount;

	printf("everage = %d\nmax triags %d\n", everageFaceInVoxel, maxFaceInVoxel);
	printf("number of voxels: %d , %d\n", lastVoxelCount, voxelsCount);
}

bool triagInVoxel(model* m, int triag, voxel* v)
{
	face* f = &m->face_array[triag];
	_3DVec dist = v->center - f->center;
	if( fabs(dist.x) <= f->size.x + v->size.x )
	{
		if( fabs(dist.y) <= f->size.y + v->size.y )
		{
			if( fabs(dist.z) <= f->size.z + v->size.z )
			{
				return true;
			}
		}
	}
	return false;
}

void makeBound(model* m, int triag, _3DVec& boundMax, _3DVec& boundMin)
{
	_3DVec v1 = m->position[m->face_array[triag].iv[0]];
	_3DVec v2 = m->position[m->face_array[triag].iv[1]];
	_3DVec v3 = m->position[m->face_array[triag].iv[2]];
	boundMax = v1;
	boundMin = v1;

	if( boundMax.x < v2.x ) boundMax.x = v2.x;
	if( boundMax.y < v2.y ) boundMax.y = v2.y;
	if( boundMax.z < v2.z ) boundMax.z = v2.z;
	if( boundMin.x > v2.x ) boundMin.x = v2.x;
	if( boundMin.y > v2.y ) boundMin.y = v2.y;
	if( boundMin.z > v2.z ) boundMin.z = v2.z;

	if( boundMax.x < v3.x ) boundMax.x = v3.x;
	if( boundMax.y < v3.y ) boundMax.y = v3.y;
	if( boundMax.z < v3.z ) boundMax.z = v3.z;
	if( boundMin.x > v3.x ) boundMin.x = v3.x;
	if( boundMin.y > v3.y ) boundMin.y = v3.y;
	if( boundMin.z > v3.z ) boundMin.z = v3.z;
}

void createVoxels(voxel* voxels, int count)
{
	if(count==0)
	{
		voxels->last = true;
		voxels->empty = false;
	}
	else
	{
		voxels->last = false;
		voxels->empty = false;
		for( int i=0,l = 0;i<2;i++)
		{
			for( int j = 0;j<2;j++)
			{
				for( int k = 0;k<2;k++,l++)
				{
					voxels->voxels[l] = NewVoxel();
					voxels->voxels[l]->boundMin.Set( 
						voxels->boundMin.x + i*(voxels->boundMax.x - voxels->boundMin.x)/2.0f, 
						voxels->boundMin.y + j*(voxels->boundMax.y - voxels->boundMin.y)/2.0f, 
						voxels->boundMin.z + k*(voxels->boundMax.z - voxels->boundMin.z)/2.0f
						);
					voxels->voxels[l]->boundMax.Set(
						voxels->boundMin.x + (i+1)*(voxels->boundMax.x - voxels->boundMin.x)/2.0f, 
						voxels->boundMin.y + (j+1)*(voxels->boundMax.y - voxels->boundMin.y)/2.0f, 
						voxels->boundMin.z + (k+1)*(voxels->boundMax.z - voxels->boundMin.z)/2.0f
						);
					voxels->voxels[l]->center = (voxels->voxels[l]->boundMin + voxels->voxels[l]->boundMax)/2.0f;
					voxels->voxels[l]->size = (voxels->voxels[l]->boundMax - voxels->voxels[l]->boundMin)/2.0f;
					createVoxels(voxels->voxels[l],count - 1);
				}
			}
		}
	}
}

void preparemodel(model* m, _3DVec& boundMax, _3DVec& boundMin){
	float rotx = config->GetField("rotx")->GetFloat();
	float roty = config->GetField("roty")->GetFloat();
	float rotz = config->GetField("rotz")->GetFloat();
	for(int i=0;i<m->iv;i++)
	{
		m->position[i].rotatex(rotx);
		m->position[i].rotatey(roty);
		m->position[i].rotatez(rotz);
	}	
	for(int i=0;i<m->ivn;i++)
	{
		m->normal[i].rotatex(rotx);
		m->normal[i].rotatey(roty);
		m->normal[i].rotatez(rotz);
	}

	for (int i=0;i<m->iface;i++){
		_3DVec v1 = m->position[m->face_array[i].iv[0]];
		_3DVec v2 = m->position[m->face_array[i].iv[1]];
		_3DVec v3 = m->position[m->face_array[i].iv[2]];
		m->face_array[i].a = v1.y*v2.z-v2.y*v1.z-v1.y*v3.z+v3.y*v1.z+v2.y*v3.z-v3.y*v2.z;
		m->face_array[i].b = v2.x*v1.z-v1.x*v2.z+v1.x*v3.z-v3.x*v1.z-v2.x*v3.z+v3.x*v2.z;
		m->face_array[i].c = v1.x*v2.y-v2.x*v1.y-v1.x*v3.y+v3.x*v1.y+v2.x*v3.y-v3.x*v2.y;
		m->face_array[i].d = v1.x*v3.y*v2.z-v1.x*v2.y*v3.z+v2.x*v1.y*v3.z-v2.x*v3.y*v1.z-v3.x*v1.y*v2.z+v3.x*v2.y*v1.z;
		m->face_array[i].normal.Set(m->face_array[i].a, m->face_array[i].b, m->face_array[i].c);
		m->face_array[i].normal.Normalize();
	}
	boundMax = m->position[0];
	boundMin = m->position[0];
	for( int i=0;i<m->iv;i++)
	{
		if( m->position[i].x > boundMax.x) boundMax.x = m->position[i].x;
		if( m->position[i].y > boundMax.y) boundMax.y = m->position[i].y;
		if( m->position[i].z > boundMax.z) boundMax.z = m->position[i].z;
		if( m->position[i].x < boundMin.x) boundMin.x = m->position[i].x;
		if( m->position[i].y < boundMin.y) boundMin.y = m->position[i].y;
		if( m->position[i].z < boundMin.z) boundMin.z = m->position[i].z;
	}
	_3DVec size = (boundMax - boundMin)/2.0f;
	_3DVec pos  = (boundMax + boundMin)/2.0f;
	float scale = config->GetField("scale")->GetFloat();
	float offsetx = config->GetField("offsetx")->GetFloat();
	float offsety = config->GetField("offsety")->GetFloat();
	float dx = size.x*offsetx - size.x;
	float dy = size.y*offsety - size.y;
	size*=scale;
	pos.x+=dx;
	pos.y+=dy;
	boundMax = pos + size;
	boundMin = pos - size;
}
