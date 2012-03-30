#pragma once

#include <vector>

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
  std::vector<std::list<int> > voisins;

public:
  std::vector<Vertex> V;
  std::vector<Triangle> T;

  void loadOFF (const std::string & filename);
  void recomputeNormals ();
  void centerAndScaleToUnit ();
  void scaleUnit ();
  void makeCube();
  void makeSphere(unsigned int resU, unsigned int resV);
  void smooth(float alpha);
  void compute1voisinages();
  void reset();
  void simplifyMesh (unsigned int r);

private:
  std::vector<Vec3Df> getCube() const;
};
