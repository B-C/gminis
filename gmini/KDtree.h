#pragma once

#include <vector>
#include <array>
#include <tuple>

#include "Vec3D.h"

class Mesh;

enum Axis {X = 0, Y = 1, Z = 2, NONE = -1};

class KDtree {
protected:
  const Mesh &mesh;
  std::vector<unsigned> triangles;// sth only if leaf;
  KDtree *left, *right;
  Axis splitAxis;

public:
  static const unsigned MIN_TRIANGLES = 10;
  const std::array<Vec3Df, 2> boundingBox;

  KDtree(const Mesh &m);

  ~KDtree() {
	delete left;
	delete right;
  }

  Axis getSplitAxis() const { return splitAxis; };
  std::tuple<const KDtree*, const KDtree*> getSons() const {
	return std::make_tuple(left, right);
  }
  const KDtree* getLeft() const { return left; }
  const KDtree* getRight() const { return right; }

  bool exec(bool (*f)(const KDtree *)) const {
	return f(this) && (splitAxis == Axis::NONE ||
					   (left->exec(f) && right->exec(f)));
  }

  void exec(void (*f)(const KDtree *)) const {
	f(this);
	if(splitAxis != Axis::NONE) {
	  left->exec(f);
	  right->exec(f);
	}
  }

private:
  KDtree(const Mesh &m, const std::vector<unsigned> &triangles,
		 const std::array<Vec3Df, 2> &boundingBox):
	mesh(m), triangles(triangles),
	left(nullptr), right(nullptr),
	splitAxis(Axis::NONE),
	boundingBox(boundingBox) {
	next();
  }

  void next();

  inline void findSplitAxis();
  inline std::tuple<std::array<Vec3Df, 2>, std::array<Vec3Df, 2>> splitBoundingBox(float med) const;
  inline std::tuple<std::vector<unsigned>, std::vector<unsigned>> splitTriangles(float med) const;
};
