#pragma once
#include "types.h"
#include "rnd.h"


struct RayPayload
{
	enum ScatterEvent
	{
		rayGotBounced,
		rayGotCancelled,
		rayDidntHitAnything
	};

	rnd::RandomState* rs;

	ScatterEvent scatterEvent;
    vec3    origin;
    vec3    direction;
    vec3    attenuation;
    vec3    normal;
};
