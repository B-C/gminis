#pragma once

#include <vector>
#include "Mesh.h"

class AmbientOcclusion {
private:
  std::vector<Vec3Df> colors;
  const Mesh &mesh;
  float R, AngleMax, VarMax;
  int N;

public:
  AmbientOcclusion(const Mesh &mesh, float R, float AngleMax, float VarMax, int N);
  const std::vector<Vec3Df>& getColors() { return colors; }
  
private:
  Vec3Df getColor(const Vertex &pos);
  bool intersectTriangle(const Vertex &ray);
  float getAleat();

  static inline bool checkOrig(const Vertex &ray, const Vertex & a, 
							   const Vertex & b, const Vertex & c){
	return ray.p != a.p && ray.p != b.p && ray.p != c.p;
  }

  bool intersect(const Vertex &ray,
				 const Vertex & a, const Vertex & b, const Vertex & c) const;
};

