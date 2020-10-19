#pragma once

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
	virtual	const fpixel& GetFPixel();
	virtual const pixel& GetPixel();
	virtual const glm::vec3& Get3DVec();
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

class FieldPixel: public Field
{
public:
	FieldPixel(pixel val);
	virtual const pixel& GetPixel();
	pixel val;
};

class FieldFPixel: public Field
{
public:
	FieldFPixel(fpixel val);
	virtual const fpixel& GetFPixel();
	fpixel val;
};

class Field3DVec: public Field
{
public:
	Field3DVec(glm::vec3 val);
	virtual const glm::vec3& Get3DVec();
	glm::vec3 val;
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