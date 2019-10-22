#pragma once
#include <ei/vector.hpp>
#include "../PBRT/ParamSet.h"
#include "../parser_exception.h"
#include "../PBRT/Material.h"
#include "../PBRT/Light.h"

using Vector = ei::Vec3;
using Matrix = ei::Matrix<float, 4, 4>;
using ei::Vec4;

class Shape
{
public:
	virtual ~Shape()
	{
	}

	virtual void init(ParamSet& set) = 0;
	virtual void applyTransform(const Matrix& mat) = 0;
	virtual void applyTransformFront(const Matrix& mat) = 0;

	void setMaterial(std::shared_ptr<Material> mat)
	{
		m_material = mat;
	}

	void setBimMaterialID(int id)
	{
		m_bimMaterialID = id;
	}

	int getBimMaterialID() const
	{
		return m_bimMaterialID;
	}

	void setAreaLight(std::shared_ptr<AreaLight> l)
	{
		assert(m_material);
		m_material->areaLight = l;
	}

	virtual Shape* clone() const = 0;

	static Vector transformPoint(const Vector& p, const Matrix& mat)
	{
		Vec4 v4 = mat * Vec4(p.x, p.y, p.z, 1.0f);
		return Vector(v4.x, v4.y, v4.z);
	}

	static Vector transformVector(const Vector& p, const Matrix& mat)
	{
		Vec4 v4 = mat * Vec4(p.x, p.y, p.z, 0.0f);
		return Vector(v4.x, v4.y, v4.z);
	}

	const Material& getMaterial() const
	{
		assert(m_material);
		return *m_material;
	}
	virtual void flipNormals() = 0;
	virtual size_t estimateSize(size_t vertexSize) const = 0;
private:
	std::shared_ptr<Material> m_material;
	int m_bimMaterialID = -1;
};
