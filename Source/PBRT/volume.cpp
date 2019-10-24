#include "volume.h"

// Volume Scattering Local Definitions
struct MeasuredSS {
	const char *name;
	float sigma_prime_s[3], sigma_a[3]; // mm^-1
};


static MeasuredSS mss[] = {
	// From "A Practical Model for Subsurface Light Transport"
	// Jensen, Marschner, Levoy, Hanrahan
	// Proc SIGGRAPH 2001
	{ "Apple",{ 2.29f, 2.39f, 1.97f },{ 0.0030f, 0.0034f, 0.046f }, },
	{ "Chicken1",{ 0.15f, 0.21f, 0.38f },{ 0.015f, 0.077f, 0.19f }, },
	{ "Chicken2",{ 0.19f, 0.25f, 0.32f },{ 0.018f, 0.088f, 0.20f }, },
	{ "Cream",{ 7.38f, 5.47f, 3.15f },{ 0.0002f, 0.0028f, 0.0163f }, },
	{ "Ketchup",{ 0.18f, 0.07f, 0.03f },{ 0.061f, 0.97f, 1.45f }, },
	{ "Marble",{ 2.19f, 2.62f, 3.00f },{ 0.0021f, 0.0041f, 0.0071f }, },
	{ "Potato",{ 0.68f, 0.70f, 0.55f },{ 0.0024f, 0.0090f, 0.12f }, },
	{ "Skimmilk",{ 0.70f, 1.22f, 1.90f },{ 0.0014f, 0.0025f, 0.0142f }, },
	{ "Skin1",{ 0.74f, 0.88f, 1.01f },{ 0.032f, 0.17f, 0.48f }, },
	{ "Skin2",{ 1.09f, 1.59f, 1.79f },{ 0.013f, 0.070f, 0.145f }, },
	{ "Spectralon",{ 11.6f, 20.4f, 14.9f },{ 0.00f, 0.00f, 0.00f }, },
	{ "Wholemilk",{ 2.55f, 3.21f, 3.77f },{ 0.0011f, 0.0024f, 0.014f }, },

	// From "Acquiring Scattering Properties of Participating Media by Dilution",
	// Narasimhan, Gupta, Donner, Ramamoorthi, Nayar, Jensen
	// Proc SIGGRAPH 2006
	{ "Lowfat Milk",{ 0.912600f, 1.074800f, 1.250000f },{ 0.000200f, 0.000400f, 0.000800f } },
	{ "Reduced Milk",{ 1.075000f, 1.221300f, 1.394100f },{ 0.000200f, 0.000400f, 0.001000f } },
	{ "Regular Milk",{ 1.187400f, 1.329600f, 1.460200f },{ 0.000100f, 0.000300f, 0.001300f } },
	{ "Espresso",{ 0.437600f, 0.511500f, 0.604800f },{ 0.166900f, 0.228700f, 0.307800f } },
	{ "Mint Mocha Coffee",{ 0.190000f, 0.260000f, 0.350000f },{ 0.098400f, 0.151900f, 0.204000f } },
	{ "Lowfat Soy Milk",{ 0.141900f, 0.162500f, 0.274000f },{ 0.000100f, 0.000500f, 0.002500f } },
	{ "Regular Soy Milk",{ 0.243400f, 0.271900f, 0.459700f },{ 0.000100f, 0.000500f, 0.003400f } },
	{ "Lowfat Chocolate Milk",{ 0.428200f, 0.501400f, 0.579100f },{ 0.000500f, 0.001600f, 0.006800f } },
	{ "Regular Chocolate Milk",{ 0.735900f, 0.917200f, 1.068800f },{ 0.000700f, 0.003000f, 0.010000f } },
	{ "Coke",{ 0.714300f, 1.168800f, 1.716900f },{ 0.696600f, 1.148000f, 1.716900f } },
	{ "Pepsi",{ 0.643300f, 0.999000f, 1.442000f },{ 0.637500f, 0.984900f, 1.442000f } },
	{ "Sprite",{ 0.129900f, 0.128300f, 0.139500f },{ 0.123000f, 0.119400f, 0.130600f } },
	{ "Gatorade",{ 0.400900f, 0.418500f, 0.432400f },{ 0.161700f, 0.125800f, 0.057900f } },
	{ "Chardonnay",{ 0.157700f, 0.174800f, 0.351200f },{ 0.154700f, 0.170100f, 0.344300f } },
	{ "White Zinfandel",{ 0.176300f, 0.237000f, 0.291300f },{ 0.173200f, 0.232200f, 0.284700f } },
	{ "Merlot",{ 0.763900f, 1.642900f, 1.919600f },{ 0.758600f, 1.642900f, 1.919600f } },
	{ "Budweiser Beer",{ 0.148600f, 0.321000f, 0.736000f },{ 0.144900f, 0.314100f, 0.728600f } },
	{ "Coors Light Beer",{ 0.029500f, 0.066300f, 0.152100f },{ 0.026800f, 0.060800f, 0.152100f } },
	{ "Clorox",{ 0.160000f, 0.250000f, 0.330000f },{ 0.017500f, 0.077700f, 0.137200f } },
	{ "Apple Juice",{ 0.121500f, 0.210100f, 0.440700f },{ 0.101400f, 0.185800f, 0.408400f } },
	{ "Cranberry Juice",{ 0.270000f, 0.630000f, 0.830000f },{ 0.257200f, 0.614500f, 0.810400f } },
	{ "Grape Juice",{ 0.550000f, 1.250000f, 1.530000f },{ 0.542800f, 1.250000f, 1.530000f } },
	{ "Ruby Grapefruit Juice",{ 0.251300f, 0.351700f, 0.430500f },{ 0.089600f, 0.191100f, 0.263600f } },
	{ "White Grapefruit Juice",{ 0.360900f, 0.380000f, 0.563200f },{ 0.009600f, 0.013100f, 0.039500f } },
	{ "Shampoo",{ 0.028800f, 0.071000f, 0.095200f },{ 0.018400f, 0.059600f, 0.080500f } },
	{ "Strawberry Shampoo",{ 0.021700f, 0.078800f, 0.102200f },{ 0.018900f, 0.075600f, 0.098900f } },
	{ "Head & Shoulders Shampoo",{ 0.367400f, 0.452700f, 0.521100f },{ 0.088300f, 0.163700f, 0.212500f } },
	{ "Lemon Tea",{ 0.340000f, 0.580000f, 0.880000f },{ 0.260200f, 0.490200f, 0.772700f } },
	{ "Orange Juice Powder",{ 0.337700f, 0.557300f, 1.012200f },{ 0.144900f, 0.344100f, 0.786300f } },
	{ "Pink Lemonade",{ 0.240000f, 0.370000f, 0.450000f },{ 0.116500f, 0.236600f, 0.319500f } },
	{ "Cappuccino Powder",{ 0.257400f, 0.353600f, 0.484000f },{ 0.192000f, 0.265400f, 0.327200f } },
	{ "Salt Powder",{ 0.760000f, 0.868500f, 0.936300f },{ 0.511500f, 0.586300f, 0.614700f } },
	{ "Sugar Powder",{ 0.079500f, 0.175900f, 0.278000f },{ 0.065000f, 0.159700f, 0.257800f } },
	{ "Suisse Mocha",{ 0.509800f, 0.647600f, 0.794400f },{ 0.187500f, 0.289300f, 0.379600f } },
	{ "Pacific Ocean Surface Water",{ 3.364500f, 3.315800f, 3.242800f },{ 3.184500f, 3.132400f, 3.014700f } },

};

bool GetVolumeScatteringProperties(const std::string &name, Spectrum *sigma_a,
	Spectrum *sigma_prime_s) {
	for (uint32_t i = 0; i < sizeof(mss) / sizeof(mss[0]); ++i) {
		if (name == mss[i].name) {
			*sigma_a = Spectrum::FromRGB(mss[i].sigma_a);
			*sigma_prime_s = Spectrum::FromRGB(mss[i].sigma_prime_s);
			return true;
		}
	}
	return false;
}