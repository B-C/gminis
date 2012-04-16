#include "KDtree.h"
#include "Mesh.h"

using namespace std;

KDtree::KDtree(const Mesh &m):mesh(m),
							  left(nullptr), right(nullptr),
							  splitAxis(Axis::NONE),
							  boundingBox(mesh.boundingBox()) {
  triangles.resize(mesh.T.size());
  for(unsigned int i = 0 ; i < mesh.T.size() ; i++)
	triangles[i] = i;

  next();
}

void KDtree::next() {
  if(triangles.size() <= MIN_TRIANGLES) return;//leaf

  findSplitAxis();
  float med = (boundingBox[1][splitAxis]+boundingBox[0][splitAxis])/2;

  array<Vec3Df, 2> lb, rb;
  tie (lb, rb) = splitBoundingBox(med);

  vector<unsigned> lt, rt;
  tie (lt, rt) = splitTriangles(med);

  triangles.clear();//not a leaf

  left = new KDtree(mesh, lt, lb);
  right = new KDtree(mesh, rt, rb);
}

void KDtree::findSplitAxis() {
  Vec3Df delta = boundingBox[1]-boundingBox[0];

  if(delta[0] <= delta[1]) {
	if(delta[1] <= delta[2])
	  splitAxis = Axis::Z;
	else
	  splitAxis = Axis::Y;
  }
  else {
	if(delta[0] <= delta[2])
	  splitAxis = Axis::Z;
	else
	  splitAxis = Axis::X;
  }
}

tuple<vector<unsigned>, vector<unsigned> > KDtree::splitTriangles(float med) const {
  vector<unsigned> left, right;

  for(unsigned t : triangles) {
	bool isInLeft = false;

	for(unsigned v : mesh.T[t].v)
	  if(mesh.V[v].p[splitAxis] <= med) {
		isInLeft = true;
		break;
	  }

	if(isInLeft)
	  left.push_back(t);
	else
	  right.push_back(t);
  }

  return make_tuple(left, right);
}

tuple<array<Vec3Df, 2>, array<Vec3Df, 2>> KDtree::splitBoundingBox(float med) const {
  array<Vec3Df, 2> left = boundingBox, right = boundingBox;

  left[1][splitAxis] = med;
  right[0][splitAxis] = med;

  return make_tuple(left, right);
}
