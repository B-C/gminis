#pragma once

#include <vector>
#include <GL/glut.h>

#include "Vec3D.h"

class Brdf {
private:
  bool on;
  std::vector<Vec3Df> posLights;

public:
  typedef enum {
	Ambient = 1,
	Lambert = 1<<1,
	Phong   = 1<<2,
  } Type;
  Vec3Df colorDif, colorSpec, colorAmbient;
  float Kd, Ks, Ka;
  float alpha; // Phong

  Brdf(std::vector<Vec3Df> posLights,
	   Vec3Df colorDif, Vec3Df colorSpec, Vec3Df colorAmbient,
	   float Kd, float Ks, float Ka,
	   float alpha):
	on(false),
	posLights(posLights),
	colorDif(colorDif), colorSpec(colorSpec), colorAmbient(colorAmbient),
	Kd(Kd), Ks(Ks), Ka(Ka),
	alpha(alpha) {};

  bool turnOnOff() {
	on = !on;
	if(on)
	  glDisable (GL_LIGHTING);
	else
	  glEnable(GL_LIGHTING);

	return on;
  }

  bool isOn() { return on; }

  Vec3Df getColor(const Vec3Df &p, const Vec3Df &n,
				  const Vec3Df posCam, int type = Ambient|Lambert|Phong);

private:
  inline Vec3Df ambient();
  inline Vec3Df lambert(Vec3Df i, Vec3Df n);
  inline Vec3Df phong(Vec3Df r, Vec3Df i, Vec3Df n);
};
