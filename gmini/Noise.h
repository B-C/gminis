#pragma once

class Noise {
private:
  static const unsigned int gaussianNoiseIterations = 100;
public:
  static float uniform(); //[-1,1]
  static float uniform(float a, float b);
  static float gaussianNoise(); //mean = 0, var = 1
  static float gaussianNoise(float var, float mean=0);
};

class Wavelet: public Noise {
private:
  float *noiseTileData;
  int noiseTileSize;

  static inline float random() {
  	float noise = Noise::gaussianNoise(4);

  	return (noise<-1.f)?-1.f:((noise>1.f)?1.f:noise);
	//	return uniform();
  }

  static int mod(int x, int n) {
	int m=x%n;
	return (m<0) ? m+n : m;
  }

  static void downsample (float *from, float *to, int n, int stride);
  static void upsample( float *from, float *to, int n, int stride);

  float wNoise(float p[3]);
  float wProjectedNoise(float p[3], float normal[3]);

public:
  ~Wavelet() {
	delete[] noiseTileData;
  }

  Wavelet(int n) {
	generateNoiseTile(n);
  }

  void generateNoiseTile(int n) {
	if (n%2) n++; // a tile size must be even
	noiseTileSize = n;
	generateNoiseTile();
  }
  void generateNoiseTile();

  float multibandNoise(float p[3], float s, float *normal,
					   int firstBand, int nbands, float *w);
};
