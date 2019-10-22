#pragma once
#include <cassert>
#include <cmath>
#include <algorithm>
#include <vector>
#include <ei/vector.hpp>

// Spectrum Utility Declarations
static const int sampledLambdaStart = 400;
static const int sampledLambdaEnd = 700;
static const int nSpectralSamples = 30;

extern bool SpectrumSamplesSorted(const float *lambda, const float *vals, int n);
extern void SortSpectrumSamples(float *lambda, float *vals, int n);
extern float InterpolateSpectrumSamples(const float *lambda, const float *vals,
	int n, float l);

inline void XYZToRGB(const float xyz[3], float rgb[3]) {
	rgb[0] = 3.240479f*xyz[0] - 1.537150f*xyz[1] - 0.498535f*xyz[2];
	rgb[1] = -0.969256f*xyz[0] + 1.875991f*xyz[1] + 0.041556f*xyz[2];
	rgb[2] = 0.055648f*xyz[0] - 0.204043f*xyz[1] + 1.057311f*xyz[2];
}


inline void RGBToXYZ(const float rgb[3], float xyz[3]) {
	xyz[0] = 0.412453f*rgb[0] + 0.357580f*rgb[1] + 0.180423f*rgb[2];
	xyz[1] = 0.212671f*rgb[0] + 0.715160f*rgb[1] + 0.072169f*rgb[2];
	xyz[2] = 0.019334f*rgb[0] + 0.119193f*rgb[1] + 0.950227f*rgb[2];
}

extern void Blackbody(const float *wl, int n, float temp, float *vals);

// Spectral Data Declarations
static const int nCIESamples = 471;
extern const float CIE_X[nCIESamples];
extern const float CIE_Y[nCIESamples];
extern const float CIE_Z[nCIESamples];
extern const float CIE_lambda[nCIESamples];
static const float CIE_Y_integral = 106.856895f;

// Spectrum Declarations
template <int nSamples> class CoefficientSpectrum
{
public:
	// CoefficientSpectrum Public Methods
	CoefficientSpectrum(float v = 0.f) {
		for (int i = 0; i < nSamples; ++i)
			c[i] = v;
		assert(!HasNaNs());
	}
	bool HasNaNs() const {
		for (int i = 0; i < nSamples; ++i)
			if (isnan(c[i])) return true;
		return false;
	}
#pragma region "MATH HELPER"
	CoefficientSpectrum &operator+=(const CoefficientSpectrum &s2) {
		assert(!s2.HasNaNs());
		for (int i = 0; i < nSamples; ++i)
			c[i] += s2.c[i];
		return *this;
	}
	CoefficientSpectrum operator+(const CoefficientSpectrum &s2) const {
		assert(!s2.HasNaNs());
		CoefficientSpectrum ret = *this;
		for (int i = 0; i < nSamples; ++i)
			ret.c[i] += s2.c[i];
		return ret;
	}
	CoefficientSpectrum operator-(const CoefficientSpectrum &s2) const {
		assert(!s2.HasNaNs());
		CoefficientSpectrum ret = *this;
		for (int i = 0; i < nSamples; ++i)
			ret.c[i] -= s2.c[i];
		return ret;
	}
	CoefficientSpectrum operator/(const CoefficientSpectrum &s2) const {
		assert(!s2.HasNaNs());
		CoefficientSpectrum ret = *this;
		for (int i = 0; i < nSamples; ++i)
			ret.c[i] /= s2.c[i];
		return ret;
	}
	CoefficientSpectrum operator*(const CoefficientSpectrum &sp) const {
		assert(!sp.HasNaNs());
		CoefficientSpectrum ret = *this;
		for (int i = 0; i < nSamples; ++i)
			ret.c[i] *= sp.c[i];
		return ret;
	}
	CoefficientSpectrum &operator*=(const CoefficientSpectrum &sp) {
		assert(!sp.HasNaNs());
		for (int i = 0; i < nSamples; ++i)
			c[i] *= sp.c[i];
		return *this;
	}
	CoefficientSpectrum operator*(float a) const {
		CoefficientSpectrum ret = *this;
		for (int i = 0; i < nSamples; ++i)
			ret.c[i] *= a;
		assert(!ret.HasNaNs());
		return ret;
	}
	CoefficientSpectrum &operator*=(float a) {
		for (int i = 0; i < nSamples; ++i)
			c[i] *= a;
		assert(!HasNaNs());
		return *this;
	}
	friend inline
		CoefficientSpectrum operator*(float a, const CoefficientSpectrum &s) {
		assert(!isnan(a) && !s.HasNaNs());
		return s * a;
	}
	CoefficientSpectrum operator/(float a) const {
		assert(!isnan(a));
		CoefficientSpectrum ret = *this;
		for (int i = 0; i < nSamples; ++i)
			ret.c[i] /= a;
		assert(!ret.HasNaNs());
		return ret;
	}
	CoefficientSpectrum &operator/=(float a) {
		assert(!isnan(a));
		for (int i = 0; i < nSamples; ++i)
			c[i] /= a;
		return *this;
	}
	bool operator==(const CoefficientSpectrum &sp) const {
		for (int i = 0; i < nSamples; ++i)
			if (c[i] != sp.c[i]) return false;
		return true;
	}
	bool operator!=(const CoefficientSpectrum &sp) const {
		return !(*this == sp);
	}
	bool IsBlack() const {
		for (int i = 0; i < nSamples; ++i)
			if (c[i] != 0.) return false;
		return true;
	}
	friend CoefficientSpectrum Sqrt(const CoefficientSpectrum &s) {
		CoefficientSpectrum ret;
		for (int i = 0; i < nSamples; ++i)
			ret.c[i] = sqrtf(s.c[i]);
		assert(!ret.HasNaNs());
		return ret;
	}
	template <int n> friend inline CoefficientSpectrum<n> Pow(const CoefficientSpectrum<n> &s, float e);
	CoefficientSpectrum operator-() const {
		CoefficientSpectrum ret;
		for (int i = 0; i < nSamples; ++i)
			ret.c[i] = -c[i];
		return ret;
	}
	friend CoefficientSpectrum Exp(const CoefficientSpectrum &s) {
		CoefficientSpectrum ret;
		for (int i = 0; i < nSamples; ++i)
			ret.c[i] = expf(s.c[i]);
		assert(!ret.HasNaNs());
		return ret;
	}
	CoefficientSpectrum Clamp(float low = 0, float high = INFINITY) const {
		CoefficientSpectrum ret;
		for (int i = 0; i < nSamples; ++i)
			ret.c[i] = std::min(high, std::max( low, c[i] ));
		assert(!ret.HasNaNs());
		return ret;
	}
#pragma endregion 
protected:
	// CoefficientSpectrum Protected Data
	float c[nSamples];
};

class RGBSpectrum : public CoefficientSpectrum<3> 
{
	using CoefficientSpectrum<3>::c;
public:
	// RGBSpectrum Public Methods
	RGBSpectrum(float v = 0.f) : CoefficientSpectrum<3>(v) { }
	RGBSpectrum(const CoefficientSpectrum<3> &v)
		: CoefficientSpectrum<3>(v) { }
	static RGBSpectrum FromRGB(const float* d)
	{
		return FromRGB(d[0], d[1], d[2]);
	}
	static RGBSpectrum FromRGB(float r, float g, float b)
	{
		RGBSpectrum s;
		s.c[0] = r;
		s.c[1] = g;
		s.c[2] = b;
		assert(!s.HasNaNs());
		return s;
	}
	float getR() const
	{
		return c[0];
	}
	float getG() const
	{
		return c[1];
	}
	float getB() const
	{
		return c[2];
	}
	static RGBSpectrum FromXYZ(const float xyz[3])
	{
		RGBSpectrum r;
		XYZToRGB(xyz, r.c);
		return r;
	}
	float y() const {
		const float YWeight[3] = { 0.212671f, 0.715160f, 0.072169f };
		return YWeight[0] * c[0] + YWeight[1] * c[1] + YWeight[2] * c[2];
	}
	
	static RGBSpectrum FromSampled(const float *lambda, const float *v,
		int n) {
		// Sort samples if unordered, use sorted for returned spectrum
		if (!SpectrumSamplesSorted(lambda, v, n)) {
			std::vector<float> slambda(&lambda[0], &lambda[n]);
			std::vector<float> sv(&v[0], &v[n]);
			SortSpectrumSamples(&slambda[0], &sv[0], n);
			return FromSampled(&slambda[0], &sv[0], n);
		}
		float xyz[3] = { 0, 0, 0 };
		float yint = 0.f;
		for (int i = 0; i < nCIESamples; ++i) {
			yint += CIE_Y[i];
			float val = InterpolateSpectrumSamples(lambda, v, n,
				CIE_lambda[i]);
			xyz[0] += val * CIE_X[i];
			xyz[1] += val * CIE_Y[i];
			xyz[2] += val * CIE_Z[i];
		}
		xyz[0] /= yint;
		xyz[1] /= yint;
		xyz[2] /= yint;
		return FromXYZ(xyz);
	}
};

using Spectrum = RGBSpectrum;

// Spectrum Inline Functions
template <int nSamples> inline CoefficientSpectrum<nSamples>
Pow(const CoefficientSpectrum<nSamples> &s, float e) {
	CoefficientSpectrum<nSamples> ret;
	for (int i = 0; i < nSamples; ++i)
		ret.c[i] = powf(s.c[i], e);
	assert(!ret.HasNaNs());
	return ret;
}


inline Spectrum Lerp(float t, const Spectrum &s1, const Spectrum &s2) {
	return (1.f - t) * s1 + t * s2;
}

inline ei::Vec3 specToVec(const Spectrum& s)
{
	return ei::Vec3(s.getR(), s.getG(), s.getB());
}