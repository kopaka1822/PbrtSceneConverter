#pragma once
#include "Shape.h"
#include <vector>
#include <cassert>
#include "../system.h"
#include <numeric>

class TriangleMesh : public Shape
{
	struct CoreGeometry
	{
		std::vector<int> m_indices;
		std::vector<Vector> m_p;
		std::vector<Vector> m_n; // per vector normals
		std::vector<Vector> m_s; // per vector tangent
		std::vector<ei::Vec2> m_uv; // per vector texture coordinates
		std::shared_ptr<Texture<float>> m_alpha;
	};
public:
	TriangleMesh()
		:
		m_geom(new CoreGeometry()),
		m_trans(ei::identity4x4())
	{}
	TriangleMesh(const TriangleMesh&) = default;
	virtual ~TriangleMesh() override
	{
	}

	virtual Shape* clone() const override
	{
		return new TriangleMesh(*this);
	}

	virtual void init(ParamSet& set) override
	{
		assert(m_geom->m_indices.size() == 0);

		if (!set.getInts("indices", m_geom->m_indices))
			throw PbrtMissingParameter("integer indices");
		if (!set.getPoints("P", m_geom->m_p))
			throw PbrtMissingParameter("point P");

		set.getNormals("N", m_geom->m_n);
		set.getVectors("S", m_geom->m_s);
		std::vector<float> uvs;
		set.getFloats("uv", uvs);
		if (!uvs.size()) set.getFloats("st", uvs);
		bool discardDegenerateUVs = set.getBool("discarddegenerateUVs", false);

		if (discardDegenerateUVs)
			discardDegenerateUvs(uvs);

		copyToVec2(m_geom->m_uv, uvs);

		verifyData(false);
	}

	virtual void applyTransform(const Matrix& mat) override
	{
		m_trans *= mat;
	}

	void applyTransformFront(const Matrix& mat) override
	{
		m_trans = mat * m_trans;
	}

	void setAlphaTexture(std::shared_ptr<Texture<float>> ta)
	{
		m_geom->m_alpha = ta;
	}

	void flipNormals() override
	{
		for (auto& n : m_geom->m_n)
			n *= -1.0f;
	}

	virtual size_t estimateSize(size_t vertexSize) const override
	{
		// vertex count + index count
		return (m_geom->m_indices.size() * sizeof(int) * 4) / 3 + m_geom->m_p.size() * vertexSize;
	}
protected:
	static void copyToVec2(std::vector<ei::Vec2>& dst, std::vector<float>& src)
	{
		dst.assign(src.size() / 2, ei::Vec2(0.0f));
		memcpy(dst.data(), src.data(), dst.size() * sizeof(dst[0]));
	}
	void verifyData(bool nothrow)
	{
		if (!m_geom->m_p.size())
			throw PbrtMissingParameter("vertex");
		if (!m_geom->m_indices.size())
			throw PbrtMissingParameter("indices");

		// check number of points, normals...
		if (m_geom->m_n.size() != 0 && m_geom->m_n.size() != m_geom->m_p.size())
			throw PbrtArgMismatch("normals");

		if (m_geom->m_s.size() != 0 && m_geom->m_s.size() != m_geom->m_p.size())
			throw PbrtArgMismatch("tangents");

		if (m_geom->m_uv.size() != 0 && m_geom->m_uv.size() != m_geom->m_p.size())
			throw PbrtArgMismatch("texture coordinates");

		if (m_geom->m_indices.size() % 3 != 0)
		{
			if (!nothrow)
				throw PbrtArgMismatch("indices count");
			System::warning("invalid number of indices in mesh found: " + std::to_string(m_geom->m_indices.size()));
			// discard some indices
			m_geom->m_indices.pop_back();
			if (m_geom->m_indices.size() % 3 != 0)
				m_geom->m_indices.pop_back();
		}

		// test indices
		for (const auto& i : m_geom->m_indices)
			if (i < 0 || i >= m_geom->m_indices.size())
				throw std::exception("trianglemesh has out of-bounds indices");

		if (m_geom->m_n.size())
		{
			float edgeFactor = System::args.get("autoedge", 360.0f);
			if (edgeFactor < 360.0f)
				flatNormalEdge(edgeFactor / 360.0f * 2.0f * ei::PI);
		}
		else if(System::args.has("autoflat"))
		{
			makeFlatNormals();
			verifyData(nothrow);
		}
	}

	void makeFlatNormals()
	{
		System::warning("missing normals, making flat ones");
		auto& idx = m_geom->m_indices;
		m_geom->m_n.resize(m_geom->m_p.size());
		// keep track of used edges
		std::vector<bool> used;
		used.assign(m_geom->m_p.size(),false);

		for (int i = 0; i < m_geom->m_indices.size(); i += 3)
		{

			auto flatNormal = ei::cross(m_geom->m_p[idx[i + 1]] - m_geom->m_p[idx[i]], m_geom->m_p[idx[i + 2]] - m_geom->m_p[idx[i]]);
			if (lensq(flatNormal) == 0.0f)
			{
				// not visible..
				continue;
			}
			flatNormal = ei::normalize(flatNormal);

			for(int j = 0; j < 3; ++j)
			{
				// set normals
				if(!used[idx[i+j]])
				{
					// set normal
					m_geom->m_n[idx[i + j]] = flatNormal;
					used[idx[i + j]] = true;
				}
				else if(ei::dot(m_geom->m_n[idx[i+j]], flatNormal) < 0.98f)
				{
					// not the same normal :/ -> add new normal
					// add a new edge
					m_geom->m_p.push_back(m_geom->m_p[idx[i + j]]);
					m_geom->m_n.push_back(flatNormal);
					if (m_geom->m_s.size())
					{
						auto tangent = m_geom->m_s[idx[i + j]];
						ei::Vec3 normal = flatNormal;
						ei::orthonormalize(normal, tangent);
						m_geom->m_s.push_back(tangent);
					}
					if (m_geom->m_uv.size())
						m_geom->m_uv.push_back(m_geom->m_uv[idx[i + j]]);

					// change index
					m_geom->m_indices[i + j] = int(m_geom->m_p.size()) - 1;
					used.push_back(true);
				} // used but same
				
			}
		}
		//for (auto& n : m_geom->m_n)
			//n = ei::Vec3(0.0f, 0.0f, 1.0f);
	}

	void flatNormalEdge(float angle)
	{
		System::warning("flatting edge normals");
		// keep track of used vertices
		std::vector<bool> used;
		used.assign(m_geom->m_p.size(),false);

		auto& idx = m_geom->m_indices;
		for(int i = 0; i < m_geom->m_indices.size(); i += 3)
		{
			// calculate surface normal
			// 1 - 0, 2 - 0
			auto flatNormal = (ei::cross(m_geom->m_p[idx[i + 1]] - m_geom->m_p[idx[i]], m_geom->m_p[idx[i + 2]] - m_geom->m_p[idx[i]]));
			if(lensq(flatNormal) == 0)
				continue;

			flatNormal = normalize(flatNormal);
			// is surface normal pointing in the right direction?
			float d[3];
			d[0] = ei::dot(m_geom->m_n[idx[i]], flatNormal);
			d[1] = ei::dot(m_geom->m_n[idx[i+1]], flatNormal);
			d[2] = ei::dot(m_geom->m_n[idx[i+2]], flatNormal);
			if((d[0] < 0)+(d[1]<0)+(d[2]<0) >= 2)
			{
				// probably pointing in the wrong direction
				flatNormal *= -1.0f;
				d[0] *= -1.0f;
				d[1] *= -1.0f;
				d[2] *= -1.0f;
			}

			// test edges
			for(int j = 0; j < 3; ++j)
			{
				if (abs(d[j]) > angle)
				{
					// add a new edge
					m_geom->m_p.push_back(m_geom->m_p[idx[i + j]]);
					m_geom->m_n.push_back(flatNormal);
					if (m_geom->m_s.size())
					{
						auto tangent = m_geom->m_s[idx[i + j]];
						auto normal = flatNormal;
						ei::orthonormalize(normal, tangent);
						m_geom->m_s.push_back(tangent);
					}
					if (m_geom->m_uv.size())
						m_geom->m_uv.push_back(m_geom->m_uv[idx[i + j]]);

					// change index
					m_geom->m_indices[i + j] = int(used.size());
					used.push_back(true);
				}
				else
					used[idx[i + j]] = true;
			}
		}

		// test if vertices can be removed
		int toMuch = std::accumulate(used.begin(), used.end(), 0);
		toMuch = int(used.size()) - toMuch;
		if (toMuch)
			System::warning(std::to_string(toMuch) + " unused vertices after auto edging");
	}
	// applyafter setting normals
	void discardDegenerateUvs(std::vector<float>& uvs)
	{
		if (uvs.size() && m_geom->m_n.size())
		{
			// if there are normals, check for bad uv's that
			// give degenerate mappings; discard them if so
			const int* vp = m_geom->m_indices.data();
			for (int i = 0; i < m_geom->m_indices.size(); i += 3, vp += 3)
			{
				float area = .5f * ei::len(ei::cross(m_geom->m_p[vp[0]] - m_geom->m_p[vp[1]], m_geom->m_p[vp[2]] - m_geom->m_p[vp[1]]));
				if (area < 1e-7) continue; // ignore degenerate tris.
				if ((uvs[2 * vp[0]] == uvs[2 * vp[1]] &&
					uvs[2 * vp[0] + 1] == uvs[2 * vp[1] + 1]) ||
					(uvs[2 * vp[1]] == uvs[2 * vp[2]] &&
						uvs[2 * vp[1] + 1] == uvs[2 * vp[2] + 1]) ||
						(uvs[2 * vp[2]] == uvs[2 * vp[0]] &&
							uvs[2 * vp[2] + 1] == uvs[2 * vp[0] + 1]))
				{
					System::warning("Degenerate uv coordinates in triangle mesh.  Discarding all uvs");
					uvs.clear();
					break;
				}
			}
		}
	}
protected:
	std::shared_ptr<CoreGeometry> m_geom;
	Matrix m_trans;
};
