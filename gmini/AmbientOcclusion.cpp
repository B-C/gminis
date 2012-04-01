#include <cstdlib>
#include <cmath>
#include "AmbientOcclusion.h"

using namespace std;

bool AmbientOcclusion::intersect(const Vertex &ray, const Vertex & a,
								 const Vertex & b, const Vertex & c) const{
  Vec3Df u = (a.p-c.p);
  Vec3Df v = (b.p-c.p);
  u.normalize();
  v.normalize();

  Vec3Df nt = Vec3Df::crossProduct(u,v);

  Vec3Df Opt = ray.p-c.p;
  Opt.normalize();

  float ir = -Vec3Df::dotProduct(nt, Opt)/
	Vec3Df::dotProduct(nt, ray.n);
  float iu = Vec3Df::dotProduct(Vec3Df::crossProduct(Opt, v), ray.n)/
	Vec3Df::dotProduct(nt, ray.n);
  float iv = Vec3Df::dotProduct(Vec3Df::crossProduct(u, Opt), ray.n)/
	Vec3Df::dotProduct(nt, ray.n);

  return (0<=iu && iu<=1) && (0<=iv && iv<=1) && (ir>=0) && (iu+iv)<=1;
}

bool AmbientOcclusion::intersectTriangle(const Vertex &ray) {
  for (unsigned int i = 0; i < mesh.T.size (); i++)  {
	const Vertex &a = mesh.V[mesh.T[i].v[0]];
	const Vertex &b = mesh.V[mesh.T[i].v[1]];
	const Vertex &c = mesh.V[mesh.T[i].v[2]];

	if( checkOrig(ray, a, b, c) &&
		(ray.p-c.p).getSquaredLength() < R*R &&
		intersect(ray, a, b, c)){
	  return true;
	}
  }
  return false;
}

float AmbientOcclusion::getColor(const Vertex & pos) {
  float val=0;

  for(int i = 0 ; i < N ; i++) {
	Vertex ray = pos;

	Vec3Df aleat = Vec3Df(random(), random(), random());
	
	aleat -= Vec3Df::projectOntoVector(aleat, ray.n);
	aleat.normalize();
	aleat*=tan(random()*angleMax);

	ray.n += aleat;
	ray.n.normalize();

	val+= (intersectTriangle(ray)? 0:1);
  }
  val/=N;

  return val;
}

AmbientOcclusion::AmbientOcclusion(const Mesh &mesh, float R,
								   float angleMax, int N):
  mesh(mesh), R(R), angleMax(angleMax), N(N) {
  colors.resize(mesh.V.size());

  for(unsigned int i = 0 ; i < colors.size() ; i++)
	colors[i] = getColor(mesh.V[i]);
}

float AmbientOcclusion::random() {
  return 2*(((float)rand())/RAND_MAX)-1;
}
