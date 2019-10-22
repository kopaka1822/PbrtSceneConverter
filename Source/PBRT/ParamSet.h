#pragma once
#include <string>
#include <vector>
#include <ei/vector.hpp>
#include "spectrum.h"

#define PARAM_SET_ADD(type)		void add##type(const std::string&, std::vector<type>); \
								bool erase##type(const std::string&);					\
								bool get##type##s(const std::string&, std::vector<type>&) const; \
								type get##type(const std::string&, type defaul) const;			\
								const std::vector<Item<type>>& get##type##Vector() const

using Vector = ei::Vec3;
using Matrix = ei::Matrix<float, 4, 4>;
class ParamSet
{
	using Float = float;
	using Int = int;
	using Bool = bool;
	using Vector = ei::Vec3;
	using Point = Vector;
	using Normal = Vector;
	using String = std::string;

	template <class T>
	class Item
	{
	public:
		Item() {}
		Item(const std::string& n, std::vector<T> data)
		: name(n), data(data){}

		std::string name;
		std::vector<T> data;
		mutable bool lookedUp = false;
	};

public:
	ParamSet() = default;
	ParamSet(ParamSet&&) = default;
	ParamSet& operator=(ParamSet&&) = default;
	ParamSet(const ParamSet&) = default;
	ParamSet& operator=(const ParamSet&) = default;
	PARAM_SET_ADD(Float);
	PARAM_SET_ADD(Int);
	PARAM_SET_ADD(Bool);
	PARAM_SET_ADD(Point);
	PARAM_SET_ADD(Vector);
	PARAM_SET_ADD(Normal);
	PARAM_SET_ADD(String);
	void addRGBSpectrum(const std::string& n, std::vector<float> d);
	void addXYZSpectrum(const std::string& n, std::vector<float> d);
	void addBlackbodySpectrum(const std::string& n, std::vector<float> d);
	bool eraseSpectrum(const std::string& n);
	bool getSpectra(const std::string& n, std::vector<Spectrum>& d) const;
	Spectrum getSpectrum(const std::string& n, Spectrum defua) const;
	void addTexture(const std::string&, const std::string&);
	bool eraseTexture(const std::string&);
	std::string getTexture(const std::string&);
	std::string getFilename(const std::string& n, const std::string& def) const;

	const std::vector<Item<Spectrum>>& getSpectrumVector() const;
	const std::vector<Item<String>>& getTextureVector() const;
private:
	std::vector<Item<float>> floats;
	std::vector<Item<int>> ints;
	std::vector<Item<bool>> bools;
	std::vector<Item<Point>> points;
	std::vector<Item<Vector>> vectors;
	std::vector<Item<Normal>> normals;
	std::vector<Item<String>> strings;
	std::vector<Item<Spectrum>> spectra;
	// first = item name | second = texture name
	std::vector<Item<String>> textures;
};

#undef PARAM_SET_ADD
