#pragma once
#include "spectrum.h"
#include <ei/vector.hpp>
using Vector = ei::Vec3;

struct Light
{
	enum Type
	{
		Distant,
		Goniometric,
		Infinite,
		Point,
		Projection,
		Spot,
		ERROR
	} type; // all lights
	Spectrum spectrum; // all lights
	
	Vector position; // point, projection, gionometric, spot
	
	Vector dir; // distant
	Vector from; // (distant), point, spotlight
	Vector to; // (distant), spotlight

	std::string mapname; // goniometric, infinite, projection
	
	union
	{
		struct
		{
			int nsamples; // infinite
			float fov; // projection (in degrees)
		};
		struct
		{
			float coneangle; // spotlight
			float conedeltaangle; // spotlight
		};
	};
};

Light::Type getLightTypeFromString(const std::string& s);

struct AreaLight
{
	// always diffuse
	Spectrum spectrum;
	int nSamples;

	bool operator==(const AreaLight& r) const
	{
		return (nSamples == r.nSamples) && (spectrum == r.spectrum);
	}
	bool operator!=(const AreaLight& r) const
	{
		return !((*this) == r);
	}
};