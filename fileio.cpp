#include <stdlib.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>
#include <vector>

#include <math.h>
#include "main.h"


inline bool exists (const std::string& name) {
    if (FILE *file = fopen(name.c_str(), "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }   
}

int save2file(pixel* b,short w, short h, const char* path)
{	
	std::ofstream  renderoutfile(path, std::ios::binary);
	if (!renderoutfile) {
		std::cout << "Could not create output file" << std::endl;
		return -2;
    }
	char buff[18];
	for (int i=0;i<18;buff[i++]=0);
	buff[2]=2;
	buff[0xc]=(char)w;
	buff[0xd]=*((char*)&w+1);
	buff[0xe]=(char)h;
	buff[0xf]=*((char*)&h+1);
	buff[0x10]=32;
	buff[0x11]=8;
	renderoutfile.write(buff,18);
	renderoutfile.write((char*)b,w*h*sizeof(pixel));
	renderoutfile.close();
	return 0;
}


texture* loadtexture(const char* path){	
	texture* t = new texture;	
	std::ifstream  tgafile(path,std::ios::binary);
	char buff[18];
	tgafile.read(buff,18);
	t->width  = *((short*)&buff[0xc]);
	t->height = *((short*)&buff[0xe]);
	int size = sizeof(pixel24)*t->width*t->height;
	t->buff = (pixel24*)malloc(size);
	tgafile.read((char*)t->buff,size);
	return t;
}

atexture* loadatexture(const char* path){	
	atexture* t = new atexture;	
	std::ifstream  tgafile(path,std::ios::binary);
	char buff[18];
	tgafile.read(buff,18);
	t->width  = *((short*)&buff[0xc]);
	t->height = *((short*)&buff[0xe]);
	int size = sizeof(pixel)*t->width*t->height;
	t->buff = (pixel*)malloc(size);
	tgafile.read((char*)t->buff,size);
	return t;
}

model* loadmodel(const char* path){	
	char cachename[255];
	strcpy(cachename, path);
	strcat(cachename, ".cache");
	model* m;
	if(!exists(cachename))
	{

	int portion = 2048*128;
	m= new model;
	m->position = (glm::vec3*)malloc(portion*sizeof(glm::vec3));
	m->normal   = (glm::vec3*)malloc(portion*sizeof(glm::vec3));
	m->texcoord = (glm::vec2*)malloc(portion*sizeof(glm::vec2));
	m->face_array = (face*)malloc(portion*sizeof(face));
	int iv=0, ivt=0, ivn=0, iface=0;		
	std::ifstream  objfile(path);
	std::string line;
	while(getline(objfile, line))    //read stream line by line
	{
		std::istringstream in(line);     //make a stream for the line itself
		std::string type;
		in >> type;                
		if(type == "v"){
			float x, y, z;
			in >> x >> y >> z;      
			m->position[iv++] = glm::vec3(x,y,-z);
		}
		if(type == "vn"){
			float x, y, z;
			in >> x >> y >> z;      
			m->normal[ivn++] = glm::vec3(x,y,-z);
		}
		if(type == "vt")				
		{
			float u, v;
			in >> u >> v;      
			m->texcoord[ivt++] = glm::vec2(u,v);
		}
		if(type == "f"){
			std::string v1,v2,v3;
			int iv_[3],ivt_[3],ivn_[3];
			in >> v1 >> v2 >> v3;      
			sscanf(v1.c_str(),"%d/%d/%d",&iv_[0],&ivt_[0],&ivn_[0]); 
			sscanf(v2.c_str(),"%d/%d/%d",&iv_[1],&ivt_[1],&ivn_[1]); 
			sscanf(v3.c_str(),"%d/%d/%d",&iv_[2],&ivt_[2],&ivn_[2]); 
			for(int i=0;i<3;i++){
				m->face_array[iface].iv[i]  = iv_[i]-1;
				m->face_array[iface].ivt[i] = ivt_[i]-1;
				m->face_array[iface].ivn[i] = ivn_[i]-1;
			}
			iface++;
		}
	}
	m->setsize(iv,ivt,ivn,iface);

	FILE* cacheFile;
	cacheFile = fopen(cachename, "wb");
	fwrite(&iv,sizeof(int),1,cacheFile);
	fwrite(&ivt,sizeof(int),1,cacheFile);
	fwrite(&ivn,sizeof(int),1,cacheFile);
	fwrite(&iface,sizeof(int),1,cacheFile);
	for(int i=0;i<iv;i++)
		fwrite(&m->position[i],sizeof(glm::vec3),1,cacheFile);
	for(int i=0;i<ivn;i++)
		fwrite(&m->normal[i],sizeof(glm::vec3),1,cacheFile);
	for(int i=0;i<ivt;i++)
		fwrite(&m->texcoord[i],sizeof(glm::vec2),1,cacheFile);
	face_r tmp;
	for(int i = 0;i<iface;i++)
	{
		tmp.iv[0] = m->face_array[i].iv[0];
		tmp.iv[1] = m->face_array[i].iv[1];
		tmp.iv[2] = m->face_array[i].iv[2];
		tmp.ivn[0] = m->face_array[i].ivn[0];
		tmp.ivn[1] = m->face_array[i].ivn[1];
		tmp.ivn[2] = m->face_array[i].ivn[2];
		tmp.ivt[0] = m->face_array[i].ivt[0];
		tmp.ivt[1] = m->face_array[i].ivt[1];
		tmp.ivt[2] = m->face_array[i].ivt[2];
		fwrite(&tmp,sizeof(face_r),1,cacheFile);
	}
	fclose(cacheFile);
	}else{
		
	FILE* cacheFile;
	cacheFile = fopen(cachename, "rb");
		
	int portion = 2048*128;
	m= new model;
	m->position = (glm::vec3*)malloc(portion*sizeof(glm::vec3));
	m->normal   = (glm::vec3*)malloc(portion*sizeof(glm::vec3));
	m->texcoord = (glm::vec2*)malloc(portion*sizeof(glm::vec2));
	m->face_array = (face*)malloc(portion*sizeof(face));
	int iv=0, ivt=0, ivn=0, iface=0;	

	fread(&iv,sizeof(int),1,cacheFile);
	fread(&ivt,sizeof(int),1,cacheFile);
	fread(&ivn,sizeof(int),1,cacheFile);
	fread(&iface,sizeof(int),1,cacheFile);
	for(int i=0;i<iv;i++)
		fread(&m->position[i], sizeof(glm::vec3),1,cacheFile);
	for(int i=0;i<ivn;i++)
		fread(&m->normal[i], sizeof(glm::vec3),1,cacheFile);
	for(int i=0;i<ivt;i++)
		fread(&m->texcoord[i], sizeof(glm::vec2),1,cacheFile);
	face_r tmp;
	for(int i = 0;i<iface;i++)
	{
		fread(&tmp,sizeof(face_r),1,cacheFile);
		m->face_array[i].iv[0] = tmp.iv[0];
		m->face_array[i].iv[1] = tmp.iv[1];
		m->face_array[i].iv[2] = tmp.iv[2];
		m->face_array[i].ivn[0] = tmp.ivn[0];
		m->face_array[i].ivn[1] = tmp.ivn[1];
		m->face_array[i].ivn[2] = tmp.ivn[2];
		m->face_array[i].ivt[0] = tmp.ivt[0];
		m->face_array[i].ivt[1] = tmp.ivt[1];
		m->face_array[i].ivt[2] = tmp.ivt[2];
	}
	m->setsize(iv,ivt,ivn,iface);
	fclose(cacheFile);
	}
	return m;
}
