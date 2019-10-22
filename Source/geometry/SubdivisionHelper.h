#pragma once
#include <string>
#include <vector>
#include <ei/vector.hpp>

namespace subdiv {
	class Triangle
	{
	public:
		enum TriangleMode { VERTICES = 0, NORMALS = 1, TEXTURE = 2 };
		Triangle();

		int containsEdge(int n1, int n2) const;

		int v[3];	// index to vertices
		int n[3];	// index to normals
		int t[3];	// index to triangle neighbours

		int findNeighborIndex(int) const;
		int getVertexIndex(int) const;

		int mode;
	};

	class SplitData
	{
	public:
		SplitData(int i, int c, int n, int _t1, int _t2) : index(i), callee(c), newPoint(n), t1(_t1), t2(_t2) {}
		int index, callee, newPoint, t1, t2;
	};

	class SubdivisionHelper
	{
	public:

		SubdivisionHelper(std::vector<int> indices, std::vector<ei::Vec3> vertices);
		void computeNormals();
		//GLuint createModel(bool flat = false);
		void createModel(std::vector<int>& dstIndices, std::vector<ei::Vec3>& dstVertices, std::vector<ei::Vec3>& dstNormals);
		void findAdjTriangles();
		void subdivide();

	protected:

		void triangleSplit(int index, int callee, int newPoint, int t1, int t2);
		void loadNormalIndices();
		//	void moveVertex (int n);
		//	int  cycleVertices ( int n, int tri, Point3D32f& p, int endIndex ) const;
		void computeNewVerts();

		std::vector<ei::Vec3>		normals_;
		std::vector<ei::Vec3>		vertices_;
		std::vector<Triangle>		triangles_;
		std::vector<bool>			processed_;

		std::vector<ei::Vec3>		newverts_;

		std::vector<int> midPoints_;
		std::vector<int> midTriangles_;
		std::vector<Triangle> tmpTriangles_;
		std::vector<SplitData> splits_;
	};
}