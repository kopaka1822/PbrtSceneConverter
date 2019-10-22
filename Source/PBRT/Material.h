#pragma once
#include "TextureParams.h"
#include <ei/stdextensions.hpp>
#include "Light.h"

struct Material
{
	using TexFloat = std::shared_ptr<Texture<float>>;
	using TexSpec = std::shared_ptr<Texture<Spectrum>>;

	enum Type
	{
		Glass,
		Kdsubsurface,
		Matte,
		Measured,
		Metal,
		Mirror,
		Mix,
		Plastic,
		Shinymetal,
		Substrate,
		Subsurface,
		Translucent,
		Uber,
		Fourier,
		Hair,
		None, // "" material that will be ignored for ray intersection
		ERROR
	} type; // all

	TexFloat bumpmap; // all
	
	TexSpec Kr; // glass, kdsubsurface, mirror, shinymetal, uber
	TexSpec Kt; // glass
	TexFloat index; // glass, kdsubsurface, subsurface, uber

	TexSpec Kd; // kdsubsurface, matte, plastic, substrate, translucent, uber
	TexFloat meanfreepath; // kdsubsurface

	TexFloat sigma; // matte

	std::string filename; // measured, fourier
	
	TexSpec eta; // metal
	TexSpec k; // metal
	TexFloat roughness; // metal, plastic, shinymetal, translucent, uber

	TexSpec amount; // mix
	std::shared_ptr<Material> material1; // mix
	std::shared_ptr<Material> material2; // mix

	TexSpec Ks; // plastic, shinymetal, substrate, translucent, uber
	TexFloat uroughness; // substrate
	TexFloat vroughness; // substrate

	TexSpec sigma_a; // subsurface, hair
	TexSpec sigma_prime_s; // subsurface
	float scale = 0.0f; // subsurface

	TexSpec reflect; // translucent
	TexSpec transmit; // translucent

	TexSpec opacity; // uber

	TexSpec color; // hair
	TexFloat eumelanin; // hair
	TexFloat pheomelanin; // hair
	TexFloat eta_hair;  // hair
	TexFloat beta_m;  // hair
	TexFloat beta_n;  // hair
	TexFloat alpha;  // hair

	std::shared_ptr<AreaLight> areaLight;

	bool operator==(const Material& o) const
	{
		if (type != o.type) return false;
		if (!Texture<float>::texCompare(bumpmap, o.bumpmap)) return false;
		if (!Texture<Spectrum>::texCompare(Kr, o.Kr)) return false;
		if (!Texture<Spectrum>::texCompare(Kt, o.Kt)) return false;
		if (!Texture<float>::texCompare(index, o.index)) return false;
		if (!Texture<Spectrum>::texCompare(Kd, o.Kd)) return false;
		if (!Texture<float>::texCompare(meanfreepath, o.meanfreepath)) return false;
		if (!Texture<float>::texCompare(sigma, o.sigma)) return false;
		if (filename != o.filename) return false;
		if (!Texture<Spectrum>::texCompare(eta, o.eta)) return false;
		if (!Texture<Spectrum>::texCompare(k, o.k)) return false;
		if (!Texture<float>::texCompare(roughness, o.roughness)) return false;
		if (!Texture<Spectrum>::texCompare(amount, o.amount)) return false;
		// material
		if (!matCompare(material1, o.material1)) return false;
		if (!matCompare(material2, o.material2)) return false;

		if (!Texture<Spectrum>::texCompare(Ks, o.Ks)) return false;
		if (!Texture<float>::texCompare(uroughness, o.uroughness)) return false;
		if (!Texture<float>::texCompare(vroughness, o.vroughness)) return false;
		if (!Texture<Spectrum>::texCompare(sigma_a, o.sigma_a)) return false;
		if (!Texture<Spectrum>::texCompare(sigma_prime_s, o.sigma_prime_s)) return false;
		if (scale != o.scale) return false;
		if (!Texture<Spectrum>::texCompare(reflect, o.reflect)) return false;
		if (!Texture<Spectrum>::texCompare(transmit, o.transmit)) return false;
		if (!Texture<Spectrum>::texCompare(opacity, o.opacity)) return false;
		if (!Texture<Spectrum>::texCompare(color, o.color)) return false;
		if (!Texture<float>::texCompare(eumelanin, o.eumelanin)) return false;
		if (!Texture<float>::texCompare(pheomelanin, o.pheomelanin)) return false;
		if (!Texture<float>::texCompare(eta_hair, o.eta_hair)) return false;
		if (!Texture<float>::texCompare(beta_m, o.beta_m)) return false;
		if (!Texture<float>::texCompare(beta_n, o.beta_n)) return false;
		if (!Texture<float>::texCompare(alpha, o.alpha)) return false;

		// Area Light
		if (!lightCompare(areaLight, o.areaLight)) return false;

		return true;
	}
	bool operator!=(const Material& o) const
	{
		return !(*this == o);
	}
	static bool matCompare(const std::shared_ptr<Material>& m1,const std::shared_ptr<Material>& m2)
	{
		if (!m1 && !m2)
			return true;

		if (!m1 || !m2)
			return false;

		return (*m1) == (*m2);
	}
	static bool lightCompare(const std::shared_ptr<AreaLight>& a1, const std::shared_ptr<AreaLight>& a2)
	{
		if (!a1 && !a2)
			return true;

		if (!a1 || !a2)
			return false;

		return (*a1) == (*a2);
	}
};

Material::Type getMaterialTypeFromString(const std::string& s);