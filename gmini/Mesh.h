#pragma once

#include <vector>
#include <list>
#include "Vec3D.h"

class Vertex {
public:
  inline Vertex () {}
  inline Vertex (const Vec3Df & p, const Vec3Df & n) : p (p), n (n) {}
  inline Vertex (const Vertex & v) : p (v.p), n (v.n) {}
  inline virtual ~Vertex () {}
  inline Vertex & operator= (const Vertex & v) {
	p = v.p;
	n = v.n;
	return (*this);
  }
  Vec3Df p;
  Vec3Df n;
};

class Triangle {
public:
  inline Triangle () {
	v[0] = v[1] = v[2] = 0;
  }
  inline Triangle (const Triangle & t) {
	v[0] = t.v[0];
	v[1] = t.v[1];
	v[2] = t.v[2];
  }
  inline Triangle (unsigned int v0, unsigned int v1, unsigned int v2) {
	v[0] = v0;
	v[1] = v1;
	v[2] = v2;
  }
  inline virtual ~Triangle () {}
  inline Triangle & operator= (const Triangle & t) {
	v[0] = t.v[0];
	v[1] = t.v[1];
	v[2] = t.v[2];
	return (*this);
  }
  unsigned int v[3];
};

class Mesh {
private:
  std::vector<Vertex> V_orig;
  std::vector<Triangle> T_orig;
  std::vector<std::list<int> > voisins;
  bool ponderate_normal;

public:
  std::vector<Vertex> V;
  std::vector<Triangle> T;

  Mesh() {
	ponderate_normal = false;
  }

  bool changeNormalComputation() {
	ponderate_normal=!ponderate_normal;
	recomputeNormals();
	return ponderate_normal;
  }

  void loadOFF(const std::string & filename);
  void makeCube();
  void makeSphere(unsigned int resU, unsigned int resV);

  void smooth(float alpha);
  void simplifyMesh(unsigned int resolution);
  void subdivideLoop();

  void reset() {
	V = V_orig;
	T = T_orig;
  }

private:
  void scaleUnit();
  void recomputeNormals();
  void centerAndScaleToUnit();
  void initPostLoad() {
	centerAndScaleToUnit();
	recomputeNormals();
	V_orig = V;
	T_orig = T;
  }

  //smooth
  void compute1voisinages();

  //simplifyMesh
  std::vector<Vec3Df> getCube() const; //compube surrounding cube
  inline static Vec3D<int> getIndice(Vec3Df point, Vec3Df offset, Vec3Df pas);
  inline static int getIndice(Vec3D<int> indices,
							  unsigned int resolution);
  inline static int getIndice(Vec3Df point, Vec3Df offset, Vec3Df pas,
							  unsigned int resolution);
};
