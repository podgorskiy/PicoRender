#pragma once

#include <stdlib.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>
#include <vector>
#include <time.h>

#include "main.h"
#include "ConfigReader.h"

const char* Field::GetType()
{
	return "none";
}

void Field::wrongType()
{
	printf("wrong type\n");
	printf("The type is \"%s\"\n",this->GetType());
	assert(false);
}

int Field::GetInt(){
	wrongType();
	return 0;
}

bool Field::GetBool(){
	wrongType();
	return false;
}

float Field::GetFloat(){
	wrongType();
	return 0.0f;
}

const fpixel& Field::GetFPixel(){
		wrongType();
		return fpixel();
}

const pixel& Field::GetPixel(){
	wrongType();
	return pixel();
}

const vec3& Field::Get3DVec(){
	wrongType();
	return vec3();
}

const char* Field::GetStr(){
	wrongType();
	return "";
}


FieldInt::FieldInt(int val):val(val){}

int FieldInt::GetInt()
{
	return val;
}

FieldBool::FieldBool(bool val):val(val){}
bool FieldBool::GetBool()
{
	return val;
}

FieldFloat::FieldFloat(float val):val(val){};
float FieldFloat::GetFloat()
{
	return val;
}

FieldPixel::FieldPixel(pixel val):val(val){};
const pixel& FieldPixel::GetPixel()
{
	return val;
}

FieldFPixel::FieldFPixel(fpixel val):val(val){}
const fpixel& FieldFPixel::GetFPixel()
{
	return val;
}

Field3DVec::Field3DVec(vec3 val):val(val){}
const vec3& Field3DVec::Get3DVec()
{
	return val;
}

FieldStr::FieldStr(const char* val):val(val){}
const char* FieldStr::GetStr()
{
	return val.c_str();
}

Config::Config()
{
	config = new ConfigMap;
}

Config::~Config()
{
	delete config;
}

void Config::LoadConfig(const char* filepath)
{
	config->clear();
	FILE * configFile;
	printf("Loading configuration from %s file\n",filepath);
	configFile = fopen(filepath,"r");
	if (!configFile)
	{
		printf ("Missing %s file\n",filepath);
	}
	int lineNumber=1;
	int res;
	char s_type[255];
	char s_name[255];
	char s1[255];
	char s2[255];
	char s3[255];
	char s4[255];
	do 
	{
		res = fscanf(configFile,"%s %s = %s, %s, %s, %s\n", s_type, s_name, s1, s2, s3, s4);
		lineNumber++;
		if (strcmp(s_type,"int")==0){
			(*config)[std::string(s_name)] = new FieldInt(atoi(s1));
		}
		if (strcmp(s_type,"bool")==0){
			(*config)[std::string(s_name)] = new FieldBool(atoi(s1));
		}
		if (strcmp(s_type,"float")==0){
			(*config)[std::string(s_name)] = new FieldFloat(atof(s1));
		}
		if (strcmp(s_type,"3dvec")==0){
			(*config)[std::string(s_name)] = new Field3DVec(vec3(atof(s1),atof(s2),atof(s3)));
		}
		if (strcmp(s_type,"fpixel")==0){
			(*config)[std::string(s_name)] = new FieldFPixel(fpixel(atof(s1),atof(s2),atof(s3)));
		}
		if (strcmp(s_type,"pixel")==0){
			(*config)[std::string(s_name)] = new FieldPixel(pixel(atoi(s1),atoi(s2),atoi(s3),atoi(s4)));
		}
		if (strcmp(s_type,"str")==0){
			(*config)[std::string(s_name)] = new FieldStr(std::string(s1).c_str());
		}
	}while(res!=EOF);		
	fclose(configFile);
};

Field* Config::GetField(const char* fname)
{
	return (*config)[fname];
}