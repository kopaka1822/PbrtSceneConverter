#pragma once
#include "Shape.h"
#include "SubdivisionHelper.h"

class LoopSubDiv : public TriangleMesh
{
	struct SplitData
	{
		int index;
		int callee;
		int newPoint;
		int t1;
		int t2;
	};

	struct Triangle
	{
		// indices to vertices
		int i[3];
	};
public:
	LoopSubDiv()
		: m_levels(3)
	{
	}
	~LoopSubDiv() override = default;

	void init(ParamSet& set) override
	{
		m_levels = std::max(set.getInt("levels", 3), 0);
		if (!set.getInts("indices", m_geom->m_indices))
			throw PbrtMissingParameter("integer indices");
		if (!set.getPoints("P", m_geom->m_p))
			throw PbrtMissingParameter("point P");

		/*if(m_levels > 0)
		{
			subdiv::SubdivisionHelper h = subdiv::SubdivisionHelper(m_geom->m_indices, m_geom->m_p);

			h.findAdjTriangles();
			for (int i = 0; i < m_levels; ++i)
				h.subdivide();

			h.createModel(m_geom->m_indices, m_geom->m_p, m_geom->m_n);
		}*/
		verifyData(false);
	}

	Shape* clone() const override
	{
		return new LoopSubDiv(*this);
	}

private:
	int m_levels;
};
