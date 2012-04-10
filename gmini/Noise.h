#pragma once

#include <vector>

#include "Vec3D.h"

class Noise {
private:
  static const unsigned int gaussianNoiseIterations = 100;

public:
  static float uniform(); //[-1,1]
  static float uniform(float a, float b);
  static float gaussianNoise(); //mean = 0, var = 1
  static float gaussianNoise(float var, float mean=0);

  static float cosineInterpolation(float a, float b, float x);

  static float octave(unsigned int i) {
	return 1.0f/pow(2,i);
  }
  static float constant(unsigned int i) {
	return 1.0f;
  }
  static float invLinear(unsigned int i) {
	return 1.0f/(i+1);
  }
  static float linear(unsigned int i) {
	return 1.0f-i/10.f;
  }
};

/******************************************************************************/

class Wavelet: public Noise {
private:
  float *noiseTileData;
  int noiseTileSize;
  float gaussianClamp;

public:
  float s;
  int firstBand;
  std::vector<float > w;

  Wavelet(int n) {
	generateNoiseTile(n, 4.f);
	s = 0.f;
	firstBand = -5;
	setW(5, octave);
  }

  ~Wavelet() {
	delete[] noiseTileData;
  }

  void generateNoiseTile(int n, float clamp=0.f) {
	if(clamp > 0.f) gaussianClamp = clamp;
	if (n%2) n++; // a tile size must be even
	noiseTileSize = n;
	generateNoiseTile();
  }

  void generateNoiseTile();

  void varClamp(float i) {
	generateNoiseTile(noiseTileSize, gaussianClamp+i);
  }

  void generateGreaterTile() {
	generateNoiseTile(noiseTileSize+2);
  }
  void generateSmalerTile() {
	if(noiseTileSize>2)
	  generateNoiseTile(noiseTileSize-2);
  }

  void setW(float (*f) (unsigned int)) {
	for(unsigned int i = 0 ; i < w.size() ; i++)
	  w[i] = f(i);
  }

  void setW(unsigned int nbands, float (*f) (unsigned int)) {
	if(nbands<=0)
	  return;

	w.resize(nbands);
	for(unsigned int i = 0 ; i < nbands ; i++)
	  w[i] = f(i);
  }

  inline float multibandNoise(const Vec3Df &p) {
	return multibandNoise(p, NULL);
  }

  inline float multibandNoise(const Vec3Df &p, const Vec3Df &normal) {
	return multibandNoise(p, &normal);
  }

  int getNoiseTileSize() { return noiseTileSize; }
  float getNoiseClamp() { return gaussianClamp; }

private:
  float random() {
  	float noise = Noise::gaussianNoise(gaussianClamp);

  	return (noise<-1.f)?-1.f:((noise>1.f)?1.f:noise);
	//	return uniform();
  }

  static int mod(int x, int n) {
	int m=x%n;
	return (m<0) ? m+n : m;
  }

  static void downsample (float *from, float *to, int n, int stride);
  static void upsample( float *from, float *to, int n, int stride);

  float wNoise(const Vec3Df &p);
  float wProjectedNoise(const Vec3Df &p, const Vec3Df &normal);
  float multibandNoise(const Vec3Df &p, const Vec3Df *normal);
};

/******************************************************************************/

class Perlin: Noise {
public:
  float fo, p;
  int n;

  Perlin(float fo, float persistence, float nbIterations):
	fo(fo), p(persistence), n(nbIterations) {}
  float noise2D(float x, float y);

private:
  static float noise(int x, int y);
  static float smoothNoise1(float x, float y);
  static inline float interpolate(float a, float b, float x) {
	return cosineInterpolation(a, b, x);
  }
  static float interpolatedNoise_1(float x, float y);
};
