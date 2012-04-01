#pragma once

#include "Vec3D.h"

class Texture {
private:
  unsigned int texMap;
  bool on;
public:
  Texture(const char * textureFilename);

  bool turnOnOff() {
	on = !on;
	if(on) {
	  glEnable (GL_TEXTURE_2D);
	  glBindTexture (GL_TEXTURE_2D, texMap);
	}
	else
	  glDisable(GL_TEXTURE_2D);

	return on;
  }
  
  void mapSphere(const Vec3Df p) {
	if(!on)
	  return;

	Vec3Df polar = Vec3Df::cartesianToPolar(p);
	polar/=2;
	for(unsigned i = 1 ; i <3 ; i++) {
	  while(polar[i] > 1)
		polar[i]-=1.0f;
	}
	glTexCoord2f(polar[1], polar[2]);//OK for sphere
  }

private:
static unsigned char* ppmRead(const char* filename, unsigned int* width, unsigned int* height);
};
