// -------------------------------------------
// gMini : a minimal OpenGL/GLUT application
// for 3D graphics.
// Copyright (C) 2006-2008 Tamy Boubekeur
// All rights reserved.
// -------------------------------------------

#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <map>

#include <GL/glut.h>
#include "Vec3D.h"

#include "Noise.h"

using namespace std;

// -------------------------------------------
// OpenGL/GLUT application code.
// -------------------------------------------

static GLint window;
static unsigned int SCREENWIDTH = 640;
static unsigned int SCREENHEIGHT = 480;
static unsigned int FPS = 0;

static float timeCounter = 0.f;
static float timeZ = 0.f;

typedef enum {WaveletNoise, PerlinNoise, PerlinMarble, GaborNoise} NoiseType;
static NoiseType noiseType = PerlinNoise;
static Wavelet wNoise(20, 0.f, -5, 4.f);
static Perlin perlin(0.5f, 4);
static Gabor gabor(1.f, 0.05, 0.0625, M_PI/4.0, 16, std::time(0), 3.f, false);
static float f0 = 1.f;

void printUsage () {
  cerr << endl
	   << "gMini: a minimal OpenGL/GLUT application" << endl
	   << "for 3D graphics." << endl
	   << "Author : Tamy Boubekeur (http://www.labri.fr/~boubek)" << endl << endl
	   << "Usage : ./gmini [<file.off>]" << endl
	   << "Kesyboard commands" << endl
	   << "------------------" << endl
	   << " p: Perlin noise" << endl
	   << " m: Perlin, marble effect" << endl
	   << " w: Wavelet noise" << endl
	   << " g: Gabor noise" << endl
	   << " s/S: increase/decrease scale" << endl
	   << " t/T: activate/increase/decrease time" << endl
	   << " ?: Print help" << endl
	   << " q, <esc>: Quit" << endl
	   << "------------------" << endl;
  if(noiseType == PerlinNoise || noiseType == PerlinMarble)
	cerr << " n/N: increase/decrease nb iterations/octaves" << endl
		 << " a/A: increase/decrease persistence" << endl;
  else if (noiseType == WaveletNoise)
	cerr << " r: recompute tile" << endl
		 << " n/N: increase/decrease tile size" << endl
		 << " c/C: increase/decrease gaussian clamp" << endl
		 << " f/F: increase/decrease first band" << endl
		 << " s/S: increase/decrease band offset" << endl
		 << " b/B: increase/decrease band size" << endl
		 << " o: compute Wi - 1/2^i" << endl
		 << " l: compute Wi - 1-i/10" << endl
		 << " k: compute Wi - 1" << endl
		 << " i: compute Wi - 1/(i+1)" << endl;
  else if (noiseType ==GaborNoise)
	cerr << " i: isotrope/anisotrope noise" << endl
		 << " k/K: increase/decrease K" << endl
		 << " a/A: increase/decrease a" << endl
		 << " f/F: increase/decrease f0" << endl
		 << " n/N: increase/decrease number impulses per kernel" << endl
		 << " o/O: increase/decrease omega0 (only anisotropic)" << endl
		 << " v/V: increase/decrease variance" << endl
		 << " r: new random offset" << endl;

  cerr << endl;
}

void usage () {
  printUsage ();
  exit (EXIT_FAILURE);
}

// ------------------------------------


void init (const char * modelFilename, const char * textureFilename) {

}

// ------------------------------------
// Replace the code of this
// functions for cleaning memory,
// closing sockets, etc.
// ------------------------------------

void clear () {

}

// ------------------------------------
// Replace the code of this
// functions for alternative rendering.
// ------------------------------------

void draw () {
  int size = SCREENHEIGHT*SCREENWIDTH;
  float* pixels = new float[size*3];

  /* Marble Perlin */
  static Vec3Df c1(0.7, 0.7, 0.7); // gris clair
  static Vec3Df c2(0.0,0.0,0.0); // blanc

  unsigned int it = 0;
  for(unsigned int i = 0 ; i < SCREENHEIGHT ; i++)
	for(unsigned int j = 0 ; j < SCREENWIDTH ; j++) {
	  float x = float(i)*f0, y = float(j)*f0;
	  if(noiseType == PerlinMarble) {
		float noise = perlin({x/10, y/10, timeZ});
		double valeur = 1 - sqrt(fabs(sin(2 * 3.141592 *noise)));

		for (int k=0; k<3; k++)
		  pixels[it++] = (c1[k] * (1 - valeur) + c2[k] * valeur);
	  }
	  else {
		float noise = 0.f;
		if(noiseType == WaveletNoise)
		  noise = wNoise({x, y, timeZ});
		else if(noiseType == PerlinNoise)
		  noise = perlin({x/10, y/10, timeZ});
		else if(noiseType == GaborNoise)
		  noise = gabor(x, y);

		for (int k=0; k<3; k++)
		  pixels[it++] = noise;
	  }
	}

  timeZ+=timeCounter;
  glDrawPixels(SCREENWIDTH, SCREENHEIGHT,GL_RGB,GL_FLOAT,pixels);

  delete []pixels;
}

void display () {
  glLoadIdentity ();
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  draw ();
  glFlush ();
  glutSwapBuffers ();
}

void idle () {
  static float lastTime = glutGet ((GLenum)GLUT_ELAPSED_TIME);
  static unsigned int counter = 0;
  counter++;
  float currentTime = glutGet ((GLenum)GLUT_ELAPSED_TIME);
  if (currentTime - lastTime >= 1000.0f) {
	FPS = counter;
	counter = 0;
	static char winTitle [64];
	sprintf (winTitle, "gmini - FPS: %d - Time: %f", FPS, timeZ);
	glutSetWindowTitle (winTitle);
	lastTime = currentTime;
  }
  glutPostRedisplay ();
}

bool noiseKey(unsigned char keyPressed) {
  if(noiseType == PerlinNoise || noiseType == PerlinMarble) {
	switch (keyPressed) {
	case 'n':
	  if(perlin.n>1)
		perlin.n--;
	  break;
	case 'N':
	  perlin.n++;
	  break;
	case 'A':
	  if(perlin.p >= 0.2f)
		perlin.p-=0.1f;
	  break;
	case 'a':
	  if(perlin.p <= 0.9)
		perlin.p+=0.1;
	  break;
	default:
	  return false;
	}
	cout << "Perlin: " << endl
		 << "\t" << perlin.n << " iterations/octaves" << endl
		 << "\tpersistence: " << perlin.p << endl;
  }
  else if(noiseType == WaveletNoise) {
	static map<char, float(*)(unsigned int)> ws = {
	  { 'o', [](unsigned i) ->float{ return 1.f/pow(2,i); } },
	  { 'l', [](unsigned i) ->float{ return 1.f-i/10.f; } },
	  { 'k', [](unsigned i) ->float{ return 1.f; } },
	  { 'i', [](unsigned i) ->float{ return 1.f/(i+1); } },
	};

	static char wsCurrent = 'o';

	switch (keyPressed) {
	case 'n':
	case 'N':
	  wNoise.generateNoiseTile(wNoise.getNoiseTileSize()+
							   (keyPressed=='n'?-2:2));
	  break;
	case 'c':
	case 'C':
	  wNoise.generateNoiseTile(wNoise.getNoiseTileSize(),
							   wNoise.getNoiseVar()+
							   (keyPressed=='c'?-1.f:1.f));
	  break;
	case 'f':
	  wNoise.firstBand--;
	  break;
	case 'F':
	  wNoise.firstBand++;
	  break;
	case 's':
	  wNoise.s-=1.f;
	  break;
	case 'S':
	  wNoise.s+=1.f;
	  break;
	case 'o':
	case 'l':
	case 'k':
	case 'i':
	  wsCurrent = keyPressed;
	case 'b':
	case 'B': {
	  unsigned int size = wNoise.w.size()+
		(keyPressed=='B'?1:keyPressed=='b'?-1:0);
	  wNoise.setW(ws[wsCurrent], size);
	}
	  break;
	case 'r':
	  wNoise.generateNoiseTile();
	  break;
	default:
	  return false;
	}
	cout << "Wavelet: " << endl
		 << "\tnoise tile size: " << wNoise.getNoiseTileSize() << endl
		 << "\ts: " << wNoise.s << endl
		 << "\tfirst band: " << wNoise.firstBand << endl
		 << "\tnumber of bands: " << wNoise.w.size()<< endl
		 << "\tnoise variance: " << wNoise.getNoiseVar()<< endl
		 << "\tw: ";
	for(const auto &it : wNoise.w)
	  cout << " " << it;
	cout << endl;
  }
  else if (noiseType ==GaborNoise) {
	switch (keyPressed) {
	case 'i':
	  gabor.isotropic = !gabor.isotropic;
	  break;
	case 'k':
	case 'K':
	  gabor.setK(gabor.getK() + (keyPressed=='K'?0.1f:-0.1f));
	  break;
	case 'a':
	case 'A':
	  gabor.setA(gabor.getA() + (keyPressed=='A'?0.01f:-0.01f));
	  break;
	case 'f':
	case 'F':
	  gabor.setF0(gabor.getF0() * (keyPressed=='F'?2.f:0.5f));
	  break;
	case 'n':
	case 'N':
	  gabor.setNbImpulsesPerKernel(gabor.getNbImpulses() +
								   (keyPressed=='N'?1:-1));
	  break;
	case 'o':
	case 'O':
	  gabor.omega0+= keyPressed=='O'?0.2f:-0.2f;
	  break;
	case 'v':
	case 'V':
	  gabor.varCoeff*= keyPressed=='V'?2.f:0.5f;
	  break;
	case 'r':
	  gabor.randomOffset = std::time(0);
	  break;
	default:
	  return false;
	}
	cout << "Gabor: " << endl
		 << "\t" << (gabor.isotropic?"isotropic":"anisotropic") << endl
		 << "\tK: " << gabor.getK() << endl
		 << "\ta: " << gabor.getA() << endl
		 << "\tf0: " << gabor.getF0() << endl
		 << "\tNumber impulses per kernel: " << gabor.getNbImpulses() << endl
		 << "\tomega0 (anisotropic): " << gabor.omega0 << endl
		 << "\tvariance coeff: " << gabor.varCoeff << endl;
  }

  return true;
}

void key (unsigned char keyPressed, int x, int y) {
  switch (keyPressed) {
  case 't':
	timeCounter/=2.f;
	break;
  case 'T':
	if(timeCounter == 0.f)
	  timeCounter = 1.f;
	else
	  timeCounter *=2.f;
	break;
  case 's':
  case 'S':
	f0*= keyPressed=='S'?2.f:0.5f;
	cout << "Scale: " << f0 << endl;
	break;
  case 'p':
	noiseType = PerlinNoise;
	break;
  case 'm':
	noiseType = PerlinMarble;
	break;
  case 'w':
	noiseType = WaveletNoise;
	break;
  case 'g':
	noiseType = GaborNoise;
	break;
  case 'q':
  case 27:
	clear ();
	exit (0);
	break;
  case '?':
  default:
	if(!noiseKey(keyPressed))
	  printUsage ();
	break;
  }
  idle ();
}

void reshape(int w, int h) {
  SCREENHEIGHT = h;
  SCREENWIDTH = w;
}

int main (int argc, char ** argv) {
  glutInit (&argc, argv);
  glutInitDisplayMode (GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
  glutInitWindowSize (SCREENWIDTH, SCREENHEIGHT);
  window = glutCreateWindow ("gMini");

  init (nullptr, nullptr);
  glutIdleFunc (idle);
  glutDisplayFunc (display);
  glutKeyboardFunc (key);
  glutReshapeFunc (reshape);
  key ('?', 0, 0);
  glutMainLoop ();
  return EXIT_SUCCESS;
}
