
#include "SubdivisionHelper.h"

#include <algorithm>
#include <functional>
#include <assert.h>

namespace subdiv {
	Triangle::Triangle()
	{
		for (int i = 0; i < 3; ++i)
		{
			v[i] = n[i] = t[i] = -1; // invalid start value
		}
	}

	int Triangle::containsEdge(int n1, int n2) const
	{
		for (int i = 0; i < 3; ++i)
		{
			int inext = (i + 1) % 3;
			if (v[i] == n1 && v[inext] == n2)
				return i;

		}

		return -1;
	}

	int Triangle::findNeighborIndex(int n) const
	{
		for (int i = 0; i < 3; ++i)
		{
			if (t[i] == n) return i;
		}

		// not found
		return -1;
	}

	int Triangle::getVertexIndex(int n) const
	{
		for (int i = 0; i < 3; ++i)
		{
			if (v[i] == n) return i;
		}

		// not found
		return -1;
	}

	SubdivisionHelper::SubdivisionHelper(std::vector<int> indices, std::vector<ei::Vec3> vertices)
	{
		vertices_ = move(vertices);
		normals_.reserve(vertices_.size());
		triangles_.reserve(vertices_.size());

		for(size_t i = 0; i < indices.size(); i += 3)
		{
			Triangle t;
			t.v[0] = indices[i];
			t.v[1] = indices[i + 1];
			t.v[2] = indices[i + 2];
			t.n[0] = t.v[0];
			t.n[1] = t.v[1];
			t.n[2] = t.v[2];
			triangles_.push_back(t);
		}
	}

	void SubdivisionHelper::computeNormals()
	{
		// initialize to zero
		normals_ = std::vector<ei::Vec3>(vertices_.size());

		for (std::vector<Triangle>::const_iterator t = triangles_.begin(); t != triangles_.end(); ++t)
		{
			ei::Vec3 normal = cross(
				vertices_[t->v[1]] - vertices_[t->v[0]],
				vertices_[t->v[2]] - vertices_[t->v[0]]);
			normal = normalize(normal);

			normals_[t->v[0]] = normals_[t->v[0]] + normal;
			normals_[t->v[1]] = normals_[t->v[1]] + normal;
			normals_[t->v[2]] = normals_[t->v[2]] + normal;
		}

		for (auto& n : normals_)
			n = normalize(n);
	}

	void SubdivisionHelper::createModel(std::vector<int>& dstIndices, std::vector<ei::Vec3>& dstVertices, std::vector<ei::Vec3>& dstNormals)
	{
		dstIndices.resize(triangles_.size() * 3);
		for(size_t i = 0; i < triangles_.size(); ++i)
		{
			dstIndices[i * 3] = triangles_[i].v[0];
			dstIndices[i * 3 + 1] = triangles_[i].v[1];
			dstIndices[i * 3 + 2] = triangles_[i].v[2];
		}

		dstVertices = vertices_;
		dstNormals = normals_;
	}

	void SubdivisionHelper::computeNewVerts()
	{
		std::vector<int> ks(newverts_.size(), 0);
		for (std::vector<Triangle>::const_iterator t = triangles_.begin(); t != triangles_.end(); ++t)
		{
			for (int i = 0; i < 3; ++i)
			{
				// all vertices are summed double
				newverts_[t->v[i]] = newverts_[t->v[i]] + vertices_[t->v[(i + 1) % 3]] + vertices_[t->v[(i + 2) % 3]];
				++ks[t->v[i]];
			}
		}

		for (std::vector<ei::Vec3>::iterator nv = newverts_.begin(); nv != newverts_.end(); ++nv)
		{
			int num = ks[nv - newverts_.begin()];
			float beta = 3.0f / (8.0f * (float)num);
			if (num == 3)
				beta = 3.0f / 16.0f;

			//float interm = (3.0/8.0 - 0.25 * cos ( 2 * M_PI / num ));
			//float beta = 1.0/num*(5.0/8.0 - interm*interm);

			*nv = (*nv * (0.5f * beta)) + vertices_[nv - newverts_.begin()] * (1 - beta*num);

		}
	}

	void SubdivisionHelper::findAdjTriangles()
	{
		for (std::vector<Triangle>::iterator t = triangles_.begin(); t != triangles_.end(); ++t)
			for (int e = 0; e < 3; ++e)
				t->t[e] = 0;//-1;

		for (std::vector<Triangle>::iterator t = triangles_.begin(); t != triangles_.end(); ++t)
		{
			// fill adj triangles
			for (int e = 0; e < 3; ++e) {
				int enext = (e + 1) % 3;

				if (t->t[e] == -1)
				{
					for (std::vector<Triangle>::iterator t2 = t + 1; t2 != triangles_.end(); ++t2)
					{
						int index;
						if ((index = t2->containsEdge(t->v[enext], t->v[e])) >= 0)
						{
							t->t[e] = t2 - triangles_.begin();
							t2->t[index] = t - triangles_.begin();
						}
					}
				}
			}
		}
	}

	void SubdivisionHelper::loadNormalIndices()
	{
		for (std::vector<Triangle>::iterator t = triangles_.begin(); t != triangles_.end(); ++t) {
			t->n[0] = t->v[0];
			t->n[1] = t->v[1];
			t->n[2] = t->v[2];
		}
	}

	void SubdivisionHelper::subdivide()
	{
		// first insert new edges
		midPoints_ = std::vector<int>(triangles_.size() * 3, -1);
		midTriangles_ = std::vector<int>(triangles_.size() * 3, -1);
		tmpTriangles_.clear();
		processed_ = std::vector<bool>(triangles_.size(), false);

		//	std::vector<int> oldVertexTriangle = vertexTriangle_;	
		//	vertexTriangle_ = std::vector<int> ( vertices_.size()*6, -1 );

		int oldVertSize = vertices_.size();

		// use list to avoid stack overflow problem
		splits_.clear();

		//updates the tmpTriangles_ and vertexTriangle_ structure

		splits_.push_back(SplitData(0, -1, -1, -1, -1));
		while (!splits_.empty())
		{
			SplitData sd = splits_.back();
			splits_.pop_back();
			triangleSplit(sd.index, sd.callee, sd.newPoint, sd.t1, sd.t2);
		}

		newverts_ = std::vector<ei::Vec3>(oldVertSize);
		computeNewVerts();

		std::copy(newverts_.begin(), newverts_.end(), vertices_.begin());

		triangles_.swap(tmpTriangles_);
		loadNormalIndices();
		computeNormals();
	}

	void SubdivisionHelper::triangleSplit(int index, int callee, int newPoint, int t1, int t2)
	{
		if (processed_[index])
			return;

		int sz;
		int midPoints[3];
		int indices[3];
		bool process[3];

		const Triangle& cur = triangles_[index];
		Triangle tri[4];
		int off = tmpTriangles_.size();

		if (callee < 0) {
			sz = 3;
			indices[0] = 0; indices[1] = 1; indices[2] = 2;
		}
		else
		{
			sz = 2;
			int update = cur.findNeighborIndex(callee);

			if (update < 0)
			{
				throw std::exception("SubdivisionHelper::triangleSplit: semantic error !");
			}

			midPoints[update] = newPoint;
			midPoints_[3 * index + update] = newPoint;

			tri[update].t[0] = t1;
			tri[(update + 1) % 3].t[2] = t2;

			tmpTriangles_[t1].t[2] = off + update;
			tmpTriangles_[t2].t[0] = off + (update + 1) % 3;

			if (update == 0) {
				indices[0] = 1; indices[1] = 2;
			}
			else if (update == 1) {
				indices[0] = 0; indices[1] = 2;
			}
			else {
				indices[0] = 0; indices[1] = 1;
			}
		}

		// process the other ones
		for (int i = 0; i < sz; ++i)
		{
			// check whether already processed ?
			// get the neighbour of current edge
			int ti = cur.t[indices[i]];
			Triangle& t = triangles_[ti];
			int tv = 3 * ti + t.findNeighborIndex(index);
			int tvnext = 3 * ti + (t.findNeighborIndex(index) + 1) % 3;

			if (midPoints_[tv] >= 0) {
				midPoints[indices[i]] = midPoints_[3 * index + indices[i]] = midPoints_[tv];
				process[i] = false;
				// update triangle connectivity

				assert(midTriangles_[tv] != -1);
				assert(midTriangles_[tvnext] != -1);

				tri[indices[i]].t[0] = midTriangles_[tvnext];
				tri[(indices[i] + 1) % 3].t[2] = midTriangles_[tv];

				tmpTriangles_[midTriangles_[tvnext]].t[2] = off + indices[i];
				tmpTriangles_[midTriangles_[tv]].t[0] = off + (indices[i] + 1) % 3;

			}
			else {
				//insert new one
				int k = indices[i];
				int knext = (k + 1) % 3;
				int kprev = (k + 2) % 3;
				int opposite = (t.findNeighborIndex(index) + 2) % 3;

				ei::Vec3 p = (vertices_[cur.v[k]] + vertices_[cur.v[knext]]) * (3.0f / 8.0f) +
					(vertices_[cur.v[kprev]] + vertices_[t.v[opposite]]) * (1.0f / 8.0f);
				vertices_.push_back(p);
				midPoints[indices[i]] = midPoints_[3 * index + indices[i]] = vertices_.size() - 1;
				process[i] = true;
			}
		}

		// add subdivided triangles to new list
		//					2
		//					/\
		 	//			       /  \
		//				  /  2 \
		//               /      \
		//			    *--------*
	//             /  \  3 /  \
		//            /  0 \  /  1 \
		//           *------**------*
	//           0               1

		assert(midPoints[0] != -1);
		assert(midPoints[1] != -1);
		assert(midPoints[2] != -1);

		tri[0].v[0] = cur.v[0];
		tri[0].v[1] = midPoints[0];
		tri[0].v[2] = midPoints[2];
		tri[0].t[1] = off + 3;

		tri[3].v[0] = midPoints[2];
		tri[3].v[1] = midPoints[0];
		tri[3].v[2] = midPoints[1];
		tri[3].t[0] = off;
		tri[3].t[1] = off + 1;
		tri[3].t[2] = off + 2;

		tri[1].v[0] = cur.v[1];
		tri[1].v[1] = midPoints[1];
		tri[1].v[2] = midPoints[0];
		tri[1].t[1] = off + 3;

		tri[2].v[0] = cur.v[2];
		tri[2].v[1] = midPoints[2];
		tri[2].v[2] = midPoints[1];
		tri[2].t[1] = off + 3;
		//	vertexTriangle_[cur.v[0]] = vertexTriangle_[midPoints[0]] = off;
		//	vertexTriangle_[cur.v[1]] = vertexTriangle_[midPoints[1]] = off+1;
		//	vertexTriangle_[cur.v[2]] = vertexTriangle_[midPoints[2]] = off+2;

		midTriangles_[3 * index] = off;
		midTriangles_[3 * index + 1] = off + 1;
		midTriangles_[3 * index + 2] = off + 2;

		for (int i = 0; i < 4; ++i)
			tmpTriangles_.push_back(tri[i]);

		processed_[index] = true;

		// call the neigbor triangles
		for (int i = 0; i < sz; ++i) {
			if (process[i])
			{
				splits_.push_back(SplitData(cur.t[indices[i]], index, midPoints[indices[i]],
					off + (indices[i] + 1) % 3, off + indices[i]));
			}
		}
	}
}