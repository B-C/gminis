#include <algorithm>

#include "Brdf.h"

using namespace std;

Vec3Df Brdf::ambient() {
  return colorAmbient * Ka;
}

Vec3Df Brdf::lambert(Vec3Df i, Vec3Df n) {
  return colorDif * Kd * max(Vec3Df::dotProduct(i,n),0.0f);
}

Vec3Df Brdf::phong(Vec3Df r, Vec3Df i, Vec3Df n) {
  Vec3Df ref = 2*Vec3Df::dotProduct(n,i)*n - i;
  ref.normalize();
  return colorSpec * Ks * pow(max(Vec3Df::dotProduct(ref,r),0.0f), alpha);
}

Vec3Df Brdf::getColor(const Vec3Df &p, const Vec3Df &n,
					  const Vec3Df posCam, int type) {
  Vec3Df color;

  Vec3Df ra=(posCam-p);
  ra.normalize();

  for(vector<Vec3Df>::const_iterator light = posLights.begin() ;
	  light < posLights.end() ; light++) {
	Vec3Df ir=(p-*light);
	ir.normalize();

	if(type&Ambient)
	  color += ambient();
	if(type&Lambert)
	  color += lambert(ir, n);
	if(type&Phong)
	  color += phong(ra, ir, n);
  }
  return color;
}
