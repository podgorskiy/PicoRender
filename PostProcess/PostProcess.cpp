#include <stdlib.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>
#include <vector>
#include <time.h>
#include <string>
#include <iostream>
#include <stdio.h>
#include <inttypes.h>

#include "../main.h"
#include "../ConfigReader.h"

//////////////////////////////
float	lightDirX;
float	lightDirY;
float	lightDirZ;
///////////////////////////////

Config* config;

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
	printf("reading data\n");

	config = new Config;
	config->LoadConfig("config_post.txt");

	lightDirX = config->GetField("lightDirX")->GetFloat();
	lightDirY = config->GetField("lightDirY")->GetFloat();
	lightDirZ = config->GetField("lightDirZ")->GetFloat();
	atexture* diffTex = loadatexture(config->GetField("diffTex")->GetStr());
	atexture* normalTex = loadatexture(config->GetField("normalTex")->GetStr());
	atexture* gi_normalTex = loadatexture(config->GetField("gi_normalTex")->GetStr());
	const char* outfile = config->GetField("outfile")->GetStr();
	assert(diffTex->height == normalTex->height);
	assert(diffTex->width == normalTex->width);
	assert(diffTex->height == gi_normalTex->height);
	assert(diffTex->width == gi_normalTex->width);
	
	int width = diffTex->width;
	int height = diffTex->height;

	fpixel* diff			= new fpixel[width*height];
	vec3* normal			= new vec3[width*height];
	float* gi				= new float[width*height];
	vec3* gi_normal		    = new vec3[width*height];
	float* alpha			= new float[width*height];

	for(int i=0;i<diffTex->height;i++){
		for(int j=0;j<diffTex->width;j++){
			float max = 255;
			diff[i * width + j] = fpixel(
				pow(diffTex->buff[i * width + j].r/max, 2.2),
				pow(diffTex->buff[i * width + j].g/max, 2.2),
				pow(diffTex->buff[i * width + j].b/max, 2.2));
			alpha[i * width + j] = diffTex->buff[i * width + j].a/max;
			normal[i * width + j] = vec3(
				(normalTex->buff[i * width + j].r/max - 0.5f)*2.0f,
				(normalTex->buff[i * width + j].g/max - 0.5f)*2.0f,
				(normalTex->buff[i * width + j].b/max - 0.5f)*2.0f);
			if(glm::length(normal[i * width + j])>0.8f)
			{
				normal[i * width + j] = glm::normalize(normal[i * width + j]);
			}
			gi_normal[i * width + j] = vec3(
				(gi_normalTex->buff[i * width + j].r/max - 0.5f)*2.0f,
				(gi_normalTex->buff[i * width + j].g/max - 0.5f)*2.0f,
				(gi_normalTex->buff[i * width + j].b/max - 0.5f)*2.0f);
			gi_normal[i * width + j] = glm::normalize(gi_normal[i * width + j]);
			gi[i * width + j] = gi_normalTex->buff[i * width + j].a/max;

		}
	}
	
	pixel* pixelbuff;
	pixelbuff = new pixel[width*height];

	for(int i=0;i<diffTex->height;i++){
		for(int j=0;j<diffTex->width;j++){
			vec3 normal_ = normal[i * width + j];
			if(normal_.Length()>0.8f)
				normal_.Normalize();
			vec3 gi_normal_ = gi_normal[i * width + j];
			gi_normal_.Normalize();
			if(j<width-1)gi_normal_ += gi_normal[i * width + j+1];
			if(j>0)gi_normal_ += gi_normal[i * width + j-1];
			if(i<height-1)gi_normal_ += gi_normal[(i+1) * width + j];
			if(i>0)gi_normal_ += gi_normal[(i-1) * width + j];
			gi_normal_.Normalize();
			vec3 light = vec3(lightDirX,lightDirY,lightDirZ);
			vec3 eye = vec3(0.0f,0.0f,-1.0f);
			light.Normalize();
			float diff_ = -normal_*light;
			if(diff_<0) diff_ = 0;
			vec3 reflect = -light + 2 * normal_ * (light * normal_);
			float spec = pow(saturate(-eye*reflect),config->GetField("rpower")->GetFloat())*config->GetField("r")->GetFloat();
			spec*= (diff[i * width + j].r + diff[i * width + j].g + diff[i * width + j].b)/3.0f;
			float gi_ = -gi_normal_*light;
			if(gi_<0) gi_ = 0;
			float ambiant = gi[i * width + j];
			float illumination = diff_*config->GetField("diff")->GetFloat()
				+ gi_*ambiant*config->GetField("gin")->GetFloat()
				+ ambiant*config->GetField("gi")->GetFloat()
				+config->GetField("ambient")->GetFloat();	
			float r,g,b;

			r = diff[i * width + j].r*illumination + spec;
			g = diff[i * width + j].g*illumination + spec;
			b = diff[i * width + j].b*illumination + spec;

			float a = alpha[i * width + j];

			pixelbuff[i * width + j].r = to8bit(gamma(r));
			pixelbuff[i * width + j].g = to8bit(gamma(g));
			pixelbuff[i * width + j].b = to8bit(gamma(b));
			pixelbuff[i * width + j].a = to8bit(a);

		}
	}
	save2file(pixelbuff,width,height,outfile);
}
