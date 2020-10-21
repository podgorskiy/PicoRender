#pragma once
#include "types.h"
#include "rnd.h"


struct PerRayData
{
	enum ScatterEvent
	{
		rayGotBounced,
		rayGotCancelled,
		rayDidntHitAnything
	};

	rnd::RandomState* rs;

	ScatterEvent scatterEvent;
    vec3    scattered_origin;
    vec3    scattered_direction;
    vec3    attenuation;
};
