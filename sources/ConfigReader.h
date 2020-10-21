#pragma once

#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <cassert>
#include <map>
#include <string>

class Field
{
public:
	virtual const char* GetType();
	void wrongType();
	virtual int GetInt();
	virtual bool GetBool();
	virtual float GetFloat();
	virtual const char* GetStr();
};

class FieldInt: public Field
{
public:
	FieldInt(int val);
	int GetInt();
	int val;
};

class FieldBool: public Field
{
public:
	FieldBool(bool val);
	virtual bool GetBool();
	bool val;
};

class FieldFloat: public Field
{
public:
	FieldFloat(float val);
	virtual float GetFloat();
	float val;
};

class Field2DVec: public Field
{
public:
	Field2DVec(vec2 val);
	virtual const vec2& Get2DVec();
	vec2 val;
};

class Field3DVec: public Field
{
public:
	Field3DVec(vec3 val);
	virtual const vec3& Get3DVec();
	vec3 val;
};


class Field4DVec: public Field
{
public:
	Field4DVec(vec4 val);
	virtual const vec4& Get4DVec();
	vec4 val;
};

class FieldStr: public Field
{
public:
	FieldStr(const char* val);
	virtual const char* GetStr();
	std::string val;
};

class Config
{
public:
	typedef std::map <std::string, Field*> ConfigMap;
	ConfigMap* config;
	Config();
	~Config();
	void LoadConfig(const char* filepath);
	Field* GetField(const char* fname);
};
