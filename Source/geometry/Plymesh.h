#pragma once
#include "TriangleMesh.h"

class Plymesh : public TriangleMesh
{
public:

	Plymesh()
	{}

	virtual ~Plymesh() override = default;
	virtual Shape* clone() const override
	{
		return new Plymesh(*this);
	}
	virtual void init(ParamSet& set) override;

private:
	static void copyToVec3(std::vector<Vector>& dst, std::vector<float>& src)
	{
		dst.assign(src.size() / 3, Vector(0.0f));
		memcpy(dst.data(), src.data(), dst.size() * sizeof(dst[0]));
	}
};