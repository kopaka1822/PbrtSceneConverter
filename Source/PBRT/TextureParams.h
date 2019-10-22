#pragma once
#include "ParamSet.h"
#include <map>
#include <memory>
#include <iostream>
#include <tuple>
#include <ei/stdextensions.hpp>
#include "../system.h"

enum class TexMapping
{
	Uv,
	Spherical,
	Cylindrical,
	Planar
};

enum class TexWrapMode
{
	Repeat,
	Black,
	Clamp
};

template <class T>
struct Texture
{
	enum Type
	{
		Bilerp,
		Checkerboard,
		Constant,
		Dots,
		Fbm,
		Imagemap,
		Marble,
		Mix,
		Scale,
		Uv,
		Windy,
		Wrinkled,
		ERROR
	} type;

	TexMapping mapping = TexMapping::Uv; // ImageMap
	TexWrapMode wrapping = TexWrapMode::Repeat;

	T value = T(0.0);
	std::shared_ptr<Texture<T>> tex1;
	std::shared_ptr<Texture<T>> tex2;
	std::shared_ptr<Texture<float>> amount;

	float su = 0.0f, sv = 0.0f, du = 0.0f, dv = 0.0f; // ImageMap
	Vector v1 = Vector(0.0f), v2 = Vector(0.0f); // ImageMap
	float vdelta = 0.0f, udelta = 0.0f; // ImageMap

	float maxAniso = 0.0f;
	bool trilerp = false;
	float scale = 0.0f;
	float gamma = 0.0f;

	std::string filename;
	//Matrix transform = ei::identity4x4();

	T v00 = T(0.0), v01 = T(0.0), v10 = T(0.0), v11 = T(0.0); // Bilerp

	int dimension = 0;
	bool aamode = false; // true = closedform | false = none

	Spectrum inside, outside; // dots

	int octaves = 0; // fbm
	float roughness = 0.0f; // fbm
	float variation = 0.0f; // marble

	bool operator==(const Texture& o) const
	{
		if (!texCompare(tex1, o.tex1))
			return false;
		if (!texCompare(tex2, o.tex2))
			return false;
		if (!texCompare(amount, o.amount))
			return false;
		// compare trivial values
		return 
			std::make_tuple(type, mapping, wrapping, value, su, sv, du, dv, /*v1, v2,*/ vdelta, udelta,
							maxAniso, trilerp, scale, gamma, filename/*, transform*/, v00, v01, v10, v11,
							dimension, aamode, inside, outside, octaves, roughness, variation) ==
			std::make_tuple(o.type, o.mapping, o.wrapping, value, o.su, o.sv, o.du, o.dv, /*o.v1, o.v2,*/ o.vdelta, o.udelta,
							o.maxAniso, o.trilerp, o.scale, o.gamma, o.filename/*, o.transform*/, o.v00, o.v01, o.v10, o.v11,
							o.dimension, o.aamode, o.inside, o.outside, o.octaves, o.roughness, o.variation);
	}

	template<class T2>
	static bool texCompare(const std::shared_ptr<Texture<T2>>& t1, const std::shared_ptr<Texture<T2>>& t2)
	{
		// both empty
		if (!t1 && !t2)
			return true;

		// one empty
		if (!t1 || !t2)
			return false;

		// both valid -> compare
		return (*t1) == (*t2);
	}
};

class TextureParams
{
public:
	TextureParams(ParamSet& geomp, ParamSet& matp,
	              std::map<std::string, std::shared_ptr<Texture<float>>>& ft,
	              std::map<std::string, std::shared_ptr<Texture<Spectrum>>>& st)
		:
		floatTextures(ft), spectrumTextures(st),
		geomParams(geomp), materialParams(matp)
	{
	}

	using Float = float;
	using Int = int;
	using Bool = bool;
	using String = std::string;
	using Vector = ei::Vec3;
	using Normal = Vector;
	using Point = Vector;
	using Filename = String;
#define GETTER(type) type get##type(const std::string& n, type d) \
{ return geomParams.get##type(n, materialParams.get##type(n, d)); }

	GETTER(Float);

	GETTER(String);

	GETTER(Int);

	GETTER(Bool);

	GETTER(Point);

	GETTER(Vector);

	GETTER(Normal);

	GETTER(Spectrum);

	GETTER(Filename);

	template <class T>
	std::shared_ptr<Texture<T>> getTexture(const std::string& name, T def)
	{
		return std::shared_ptr<Texture<T>>();
	}

	template <>
	std::shared_ptr<Texture<float>> getTexture(const std::string& n, float def)
	{
		std::string name = geomParams.getTexture(n);
		if (name == "") name = materialParams.getTexture(n);

		if (name != "")
		{
			auto it = floatTextures.find(name);
			if (it != floatTextures.end()) return it->second;
			System::error("Couldn't find float texture named \"" + name + "\" for parameter \""
				+ n + "\"");
		}
		float val = geomParams.getFloat(n, materialParams.getFloat(n, def));
		// return constant texture
		auto tex = std::shared_ptr<Texture<float>>(new Texture<float>);
		tex->type = Texture<float>::Constant;
		tex->value = val;
		return tex;
	}

	template <>
	std::shared_ptr<Texture<Spectrum>> getTexture(const std::string& n, Spectrum def)
	{
		std::string name = geomParams.getTexture(n);
		if (name == "") name = materialParams.getTexture(n);

		if (name != "")
		{
			auto it = spectrumTextures.find(name);
			if (it != spectrumTextures.end()) return it->second;
			System::error("Couldn't find spectrum texture named \"" + name + "\" for parameter \""
				+ n + "\"");
		}
		Spectrum val = geomParams.getSpectrum(n, materialParams.getSpectrum(n, def));
		// return constant texture
		auto tex = std::shared_ptr<Texture<Spectrum>>(new Texture<Spectrum>);
		tex->type = Texture<Spectrum>::Constant;
		tex->value = val;
		return tex;
	}

	std::shared_ptr<Texture<float>> getTextureOrNull(const std::string& n)
	{
		std::string name = geomParams.getTexture(n);
		if (name == "") name = materialParams.getTexture(n);
		if (name == "")
			return nullptr;

		auto it = floatTextures.find(name);
		if (it != floatTextures.end()) return it->second;
		System::error("Couldn't find float texture named \"" + name + "\" for parameter \""
			+ n + "\"");
		return nullptr;
	}

private:
	std::map<std::string, std::shared_ptr<Texture<float>>>& floatTextures;
	std::map<std::string, std::shared_ptr<Texture<Spectrum>>>& spectrumTextures;
	ParamSet& geomParams;
	ParamSet& materialParams;
};

size_t getTextureTypeFromString(const std::string& s);
