#include "ParamSet.h"
#include <iostream>
#include <memory>
#include "../system.h"

#define PARAM_SET_DECL(type,vec)	void ParamSet::add##type(const std::string& name, std::vector<type> d){ \
								erase##type(name);												\
								(vec).push_back( Item<type>( name , d ) );	}								\
								bool ParamSet::erase##type(const std::string& n) {						\
								for(auto i = (vec).begin(); i != (vec).end(); ++i){				\
								if( i->name == n ) {(vec).erase(i); return true;}} return false; }	\
								bool ParamSet::get##type##s(const std::string& n, std::vector<type>& d) const{			\
								for(const auto& i : (vec) ){											\
									if(i.name == n){ i.lookedUp = true; d = i.data; return true;}	\
								} return false;}													\
								ParamSet::type ParamSet::get##type(const std::string& n, type defau) const{			\
								for(const auto& i : (vec) ){											\
									if(i.name == n && i.data.size() == 1) {i.lookedUp = true; return i.data[0];}	\
								} return defau;}	\
								const std::vector<ParamSet::Item<ParamSet::type>>& ParamSet::get##type##Vector() const {	\
									return vec;																	\
								}

PARAM_SET_DECL(Float, floats);
PARAM_SET_DECL(Int, ints);
PARAM_SET_DECL(Bool, bools);
PARAM_SET_DECL(Point, points);
PARAM_SET_DECL(Vector, vectors);
PARAM_SET_DECL(Normal, normals);
PARAM_SET_DECL(String, strings);

void ParamSet::addRGBSpectrum(const std::string& n, std::vector<float> d)
{
	eraseSpectrum(n);
	if (d.size() % 3 != 0) System::warning("invalid number of rgb pairs supplied" + n);
	std::vector<Spectrum> specs;
	for(size_t i = 0; i < d.size() / 3; i++)
		specs.push_back(Spectrum::FromRGB(d[i * 3], d[i * 3 + 1], d[i * 3 + 2]));
	
	spectra.push_back(Item<Spectrum>(n, move(specs)));
}

void ParamSet::addXYZSpectrum(const std::string& n, std::vector<float> d)
{
	eraseSpectrum(n);
	if (d.size() % 3 != 0) System::warning("invalid number of xyz pairs supplied" + n);
	std::vector<Spectrum> specs;
	for (size_t i = 0; i < d.size() / 3; i++)
		specs.push_back(Spectrum::FromXYZ((&d[i * 3])));
	spectra.push_back(Item<Spectrum>(n, move(specs)));
}

void ParamSet::addBlackbodySpectrum(const std::string& n, std::vector<float> d)
{
	eraseSpectrum(n);
	if (d.size() % 2 != 0) System::warning("invalid number of spectrum pairs supplied: " + n);
	std::vector<Spectrum> specs;
	auto v = std::unique_ptr<float[]>(new float[nCIESamples]);
	for(size_t i = 0; i < d.size() / 2; i++)
	{
		Blackbody(CIE_lambda, nCIESamples, d[2 * i], v.get());
		specs.push_back(d[2 * i + 1] * Spectrum::FromSampled(CIE_lambda, v.get(), nCIESamples));
	}
	spectra.push_back(Item<Spectrum>(n, specs));
}

bool ParamSet::eraseSpectrum(const std::string& n)
{
	for (auto i = spectra.begin(); i != spectra.end(); ++i) 
	{
		if (i->name == n) 
		{
			spectra.erase(i);
			return true;
		}
	}
	return false;
}

bool ParamSet::getSpectra(const std::string& n, std::vector<Spectrum>& d) const
{
	for (const auto& i : spectra) 
	{
		if (i.name == n)
		{
			i.lookedUp = true;
			d = i.data;
			return true;
		}
	} 
	return false;
}

Spectrum ParamSet::getSpectrum(const std::string& n, Spectrum defau) const
{
	for (const auto& i : spectra) 
	{
		if (i.name == n && i.data.size() == 1)
		{
			i.lookedUp = true; 
			return i.data[0];
		}	
	}
	return defau;
}

void ParamSet::addTexture(const std::string& n, const std::string& d)
{
	eraseTexture(n);
	textures.push_back(Item<String>(n, { d }));
}

bool ParamSet::eraseTexture(const std::string& n)
{
	for (auto i = textures.begin(); i != textures.end(); ++i)
	{
		if (i->name == n)
		{
			textures.erase(i);
			return true;
		}
	}
	return false;
}

std::string ParamSet::getTexture(const std::string& n)
{
	for (const auto& i : textures)
	{
		if (i.name == n && i.data.size() == 1)
		{
			i.lookedUp = true;
			return i.data[0];
		}
	}
	return "";
}

std::string ParamSet::getFilename(const std::string& n, const std::string& def) const
{
	auto filename = getString(n, "");
	if (filename == "")
		return def;

	// TODO implement absolute path
	return System::fixPath(System::getCurrentDirectory() + filename);
}

const std::vector<ParamSet::Item<RGBSpectrum>>& ParamSet::getSpectrumVector() const
{
	return spectra;
}

const std::vector<ParamSet::Item<std::basic_string<char>>>& ParamSet::getTextureVector() const
{
	return textures;
}
