#pragma once
#include "Shape.h"
#define _USE_MATH_DEFINES
#include <math.h>

class Sphere : public Shape
{
	static const int RESOLUTION = 50;
public:
	virtual ~Sphere() override
	{
	}

	virtual Shape* clone() const override
	{
		return new Sphere(*this);
	}

	virtual void init(ParamSet& set) override
	{
		m_radius = set.getFloat("radius", 1.0f);
		m_zmin = set.getFloat("zmin", m_radius);
		m_zmax = set.getFloat("zmax", m_radius);
		m_phimax = set.getFloat("phimax", 360.0f);
		m_transform = ei::identity4x4();
		// TODO scale radius?
	}

	virtual void applyTransform(const Matrix& mat) override
	{
		m_transform *= mat;
	}


	void applyTransformFront(const Matrix& mat) override
	{
		m_transform = mat * m_transform;
	}

	// method that was used to create sphere geometry. Leaving the code here for future reference
	/*void makeGeometry(std::vector<bim::Chunk::FullVertex>& vert, std::vector<int>& indices, bim::Property::Val props) const override
	{
		vert.clear();

		if(m_zmax != m_radius || m_zmin != m_radius)
			System::warning("Sphere is created with open top or bottom.");

		// Compute tessellation and step parameters
		const float phiMax = float(m_phimax / 360.0f * 2.0f * M_PI);
		float thetaMax = acosf(m_zmax / m_radius);
		float thetaMin = acosf(-m_zmin / m_radius);
		int resPhi = int(ceil(RESOLUTION * phiMax / 2.0f / M_PI));
		int resTheta = int(ceil(RESOLUTION * abs(thetaMax - thetaMin) / M_PI));
		float pStep = phiMax / (resPhi - 1);
		float tStep = (thetaMax - thetaMin) / (resTheta - 1);

		// Create vertices in rings
		for(int t = 0; t < resTheta; ++t)
		{
			float theta = t * tStep + std::min(thetaMin, thetaMax);
			float sinT = sin(theta);
			float cosT = cos(theta);
			for(int p = 0; p < resPhi; ++p)
			{
				float phi = p * pStep;

				bim::Chunk::FullVertex vertex;
				vertex.normal = Vector(sinT * sin(phi), sinT * cos(phi), cosT);
				vertex.position = ei::transform(vertex.normal * m_radius, m_transform);
				if(m_flipNormal) vertex.normal = - vertex.normal;
				vertex.tangent = cross(Vector(0.0f, 1.0f, 0.0f), vertex.normal);
				vertex.normal = transformDir(vertex.normal, m_transform);
				vertex.tangent = transformDir(vertex.tangent, m_transform);
				vertex.bitangent = cross(vertex.normal, vertex.tangent);
				vertex.texCoord0 = ei::Vec2(p / float(resPhi - 1.0f), t / float(resTheta - 1));
				vert.push_back(vertex);
			}
		}

		// Create indices
		for(int t = 0; t < resTheta-1; ++t)
		{
			int rowStart = t * resTheta;
			int nextRowStart = (t+1) * resTheta;
			for(int p = 0; p < resPhi-1; ++p)
			{
				//
				// r+0 r+1
				// *---*
				// |\  |
				// | \ |
				// |  \|
				// *---*
				// r+0 r+1
				//
				// Also, check if one of the triangles is degenerated (happens at caps).
				// upper triangle
				float area = len(cross(vert[rowStart + p + 1].position - vert[rowStart + p].position,
					vert[nextRowStart + p + 1].position - vert[rowStart + p].position));
				if(area > 0.0f)
				{
					indices.push_back(rowStart + p);
					indices.push_back(rowStart + p + 1);
					indices.push_back(nextRowStart + p + 1);
				}
				// lower triangle
				area = len(cross(vert[nextRowStart + p + 1].position - vert[rowStart + p].position,
					vert[nextRowStart + p].position - vert[rowStart + p].position));
				if(area > 0.0f)
				{
					indices.push_back(rowStart + p);
					indices.push_back(nextRowStart + p + 1);
					indices.push_back(nextRowStart + p);
				}
			}
		}

		// assert indices range
		for (const auto& i : indices)
		{
			assert(i >= 0 && i < vert.size());
		}
	}*/

	virtual size_t estimateSize(size_t vertexSize) const override
	{
		const float phiMax = float(m_phimax / 360.0f * 2.0f * M_PI);
		float thetaMax = acosf(m_zmax / m_radius);
		float thetaMin = acosf(-m_zmin / m_radius);
		int resPhi = int(ceil(RESOLUTION * phiMax / 2.0f / M_PI));
		int resTheta = int(ceil(RESOLUTION * abs(thetaMax - thetaMin) / M_PI));
		return resTheta * resPhi * vertexSize + (resTheta - 1) * (resPhi - 1) * 6 * sizeof(uint32);
	}

	void flipNormals() override
	{
		m_flipNormal = true;
	}

private:
	float m_radius = 1.0f;
	float m_zmin = 1.0f;
	float m_zmax = 1.0f;
	float m_phimax = 360.0f;
	Matrix m_transform;
	bool m_flipNormal = false;
};
