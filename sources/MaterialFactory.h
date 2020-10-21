#pragma once

#include <optix.h>
#include <optixu/optixpp.h>
#include <functional>


enum MaterialTypes{
    Lambertian
};

typedef std::function<optix::Material(MaterialTypes)> MatFactory;

MatFactory MakeMaterialFactory(optix::Context& context);

