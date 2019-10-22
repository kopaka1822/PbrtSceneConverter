#pragma once
#include <string>
#include "spectrum.h"

bool GetVolumeScatteringProperties(const std::string &name, Spectrum *sigma_a,
	Spectrum *sigma_prime_s);
