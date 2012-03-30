// -------------------------------------------
// gMini : a minimal OpenGL/GLUT application
// for 3D graphics.
// Copyright (C) 2006-2008 Tamy Boubekeur
// All rights reserved.
// -------------------------------------------

// -------------------------------------------
// Disclaimer: this code is dirty in the
// meaning that there is no attention paid to
// proper class attribute access, memory
// management or optimisation of any kind. It
// is designed for quick-and-dirty testing
// purpose.
// -------------------------------------------

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cmath>

#include <GL/glut.h>
#include <Vec3D.h>
#include "Camera.h"

using namespace std;

// -------------------------------------------
// Basic Mesh class
// -------------------------------------------

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
public:
  vector<Vertex> V;
  vector<Triangle> T;

  void loadOFF (const string & filename);
  void recomputeNormals ();
  void centerAndScaleToUnit ();
  void scaleUnit ();
  void makeCube();
  void makeSphere(unsigned int resU, unsigned int resV);
};

void Mesh::makeCube () {
  unsigned int sizeV = 8;
  unsigned int sizeT = 12;

  V.resize (sizeV);
  T.resize (sizeT);

  for(int j = 0 ; j < 2 ; j++) {
	V[4*j].p   = Vec3Df(1,1,j);
	V[4*j+1].p = Vec3Df(1,0,j);
	V[4*j+2].p = Vec3Df(0,0,j);
	V[4*j+3].p = Vec3Df(0,1,j);
  }

  unsigned int indices[] =
	{0, 1, 2,   0, 2, 3,
	 6, 5, 4,   7, 6, 4,
	 5, 1, 0,   4, 5, 0, 
	 2, 6, 3,   6, 7, 3, 
	 6, 2, 1,   5, 6, 1, 
	 3, 7, 0,   7, 4, 0};

  for(unsigned int i = 0 ; i<sizeT ; i++) {
	for(unsigned int j = 0 ; j<3 ; j++) {	
	  T[i].v[j] = indices[3*i+j];
	}
  }
  
  centerAndScaleToUnit ();
  recomputeNormals ();
}

void Mesh::makeSphere(unsigned int resU, unsigned int resV) {
  unsigned int sizeV = (resU+1)*resV;
  unsigned int sizeT = 2*sizeV;

  V.resize (sizeV);
  T.resize (sizeT);

  unsigned t = 0;
  for(unsigned int m = 0 ; m<=resU ; m++) {
	for(unsigned int n = 0 ; n < resV ; n++) { 
	  V[t++].p = Vec3Df(sin(M_PI * m/resU) * cos(2*M_PI * n/resV), 
						sin(M_PI * m/resU) * sin(2*M_PI * n/resV), 
						cos(M_PI * m/resU));
	}
  }

  t = 0;
  for(unsigned int j = 0 ; j <= resU ; j++) {
	for(unsigned int i = 0 ; i < resV-1 ; i++) {
	  T[t].v[2] = (j  )*(resV-1) + i  ;
	  T[t].v[1] = (j+1)*(resV-1) + i+1;
	  T[t].v[0] = (j  )*(resV-1) + i+1;
	  t++;				
						
	  T[t].v[2] = (j  )*(resV-1) + i  ;
	  T[t].v[1] = (j+1)*(resV-1) + i  ;
	  T[t].v[0] = (j+1)*(resV-1) + i+1;
	  t++;
	}
  }

  cout << sizeT << " " << t<<endl;

  centerAndScaleToUnit ();
  recomputeNormals ();
}


void Mesh::loadOFF (const string & filename) {
  ifstream in (filename.c_str ());
  if (!in) 
	exit (EXIT_FAILURE);
  string offString;
  unsigned int sizeV, sizeT, tmp;
  in >> offString >> sizeV >> sizeT >> tmp;
  V.resize (sizeV);
  T.resize (sizeT);
  for (unsigned int i = 0; i < sizeV; i++)
	in >> V[i].p;
  int s;
  for (unsigned int i = 0; i < sizeT; i++) {
	in >> s;
	for (unsigned int j = 0; j < 3; j++)
	  in >> T[i].v[j];
  }
  in.close ();
  centerAndScaleToUnit ();
  recomputeNormals ();
}

void Mesh::recomputeNormals () {
  for (unsigned int i = 0; i < V.size (); i++)
	V[i].n = Vec3Df (0.0, 0.0, 0.0);
  for (unsigned int i = 0; i < T.size (); i++) {
	Vec3Df e01 = V[T[i].v[1]].p -  V[T[i].v[0]].p;
	Vec3Df e02 = V[T[i].v[2]].p -  V[T[i].v[0]].p;
	Vec3Df e12 = V[T[i].v[2]].p -  V[T[i].v[1]].p;
	e01.normalize(); e02.normalize(); e12.normalize();

	Vec3Df n[3] = {
	  Vec3Df::crossProduct (e01, e02),
	  Vec3Df::crossProduct (e12,-e01),
	  Vec3Df::crossProduct (-e02,-e12)
	};

	float angles[3] = {
	  acos(Vec3Df::dotProduct(e01, e02)),
	  acos(Vec3Df::dotProduct(-e01, e12)),
	  acos(Vec3Df::dotProduct(-e12, -e02)),
	};
 
	for (unsigned int j = 0; j < 3; j++) {
	  n[j].normalize();
	  V[T[i].v[j]].n += angles[j]* n[j];
	}
  }
  for (unsigned int i = 0; i < V.size (); i++)
	V[i].n.normalize ();
}

void Mesh::centerAndScaleToUnit () {
  Vec3Df c;
  for  (unsigned int i = 0; i < V.size (); i++)
	c += V[i].p;
  c /= V.size ();
  float maxD = Vec3Df::distance (V[0].p, c);
  for (unsigned int i = 0; i < V.size (); i++){
	float m = Vec3Df::distance (V[i].p, c);
	if (m > maxD)
	  maxD = m;
  }
  for  (unsigned int i = 0; i < V.size (); i++)
	V[i].p = (V[i].p - c) / maxD;
}

// -------------------------------------------
// OpenGL/GLUT application code.
// -------------------------------------------

static GLint window;
static unsigned int SCREENWIDTH = 640;
static unsigned int SCREENHEIGHT = 480;
static Camera camera;
static bool mouseRotatePressed = false;
static bool mouseMovePressed = false;
static bool mouseZoomPressed = false;
static int lastX=0, lastY=0, lastZoom=0;
static unsigned int FPS = 0;
static bool fullScreen = false;

typedef enum {Wireframe, Flat, Gouraud} PolygonMode;
static PolygonMode polygonMode = Flat;

static Mesh mesh;

void printUsage () {
  cerr << endl 
	   << "gMini: a minimal OpenGL/GLUT application" << endl
	   << "for 3D graphics." << endl 
	   << "Author : Tamy Boubekeur (http://www.labri.fr/~boubek)" << endl << endl
	   << "Usage : ./gmini [<file.off>]" << endl
	   << "Keyboard commands" << endl 
	   << "------------------" << endl
	   << " ?: Print help" << endl 
	   << " w: Toggle Wireframe/Flat/Gouraud Rendering Mode" << endl 
	   << " f: Toggle full screen mode" << endl 
	   << " <drag>+<left button>: rotate model" << endl 
	   << " <drag>+<right button>: move model" << endl
	   << " <drag>+<middle button>: zoom" << endl
	   << " q, <esc>: Quit" << endl << endl; 
}

void usage () {
  printUsage ();
  exit (EXIT_FAILURE);
}



// ------------------------------------

void initLight () {
  GLfloat light_position1[4] = {22.0f, 16.0f, 50.0f, 0.0f};
  GLfloat direction1[3] = {-52.0f,-16.0f,-50.0f};
  GLfloat color1[4] = {0.5f, 1.0f, 0.5f, 1.0f};
  GLfloat ambient[4] = {0.3f, 0.3f, 0.3f, 0.5f};
  
  glLightfv (GL_LIGHT1, GL_POSITION, light_position1);
  glLightfv (GL_LIGHT1, GL_SPOT_DIRECTION, direction1);
  glLightfv (GL_LIGHT1, GL_DIFFUSE, color1);
  glLightfv (GL_LIGHT1, GL_SPECULAR, color1);
  glLightModelfv (GL_LIGHT_MODEL_AMBIENT, ambient);
  glEnable (GL_LIGHT1);
  glEnable (GL_LIGHTING);
}

void init (const char * modelFilename) {
  camera.resize (SCREENWIDTH, SCREENHEIGHT);
  mesh.loadOFF (modelFilename);
  //mesh.makeSphere(10, 10);
  initLight ();
  glCullFace (GL_BACK);
  glEnable (GL_CULL_FACE);
  glDepthFunc (GL_LESS);
  glEnable (GL_DEPTH_TEST);
  glClearColor (0.2f, 0.2f, 0.3f, 1.0f);
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
  const vector<Vertex> & V = mesh.V;
  const vector<Triangle> & T = mesh.T;
  glBegin (GL_TRIANGLES);
  for (unsigned int i = 0; i < T.size (); i++) {
	if (polygonMode != Gouraud) {
	  Vec3Df e01 = V[T[i].v[1]].p -  V[T[i].v[0]].p;
	  Vec3Df e02 = V[T[i].v[2]].p -  V[T[i].v[0]].p;
	  Vec3Df n = Vec3Df::crossProduct (e01, e02);
	  n.normalize ();
	  glNormal3f (n[0], n[1], n[2]);
	}
	for (unsigned int j = 0; j < 3; j++) {
	  const Vertex & v = V[T[i].v[j]];
	  if (polygonMode == Gouraud)
		glNormal3f (v.n[0], v.n[1], v.n[2]);
	  glVertex3f (v.p[0], v.p[1], v.p[2]);
	}
  }
  glEnd ();
}

void display () {
  glLoadIdentity ();
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  camera.apply ();
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
	unsigned int numOfTriangles = mesh.T.size ();
	sprintf (winTitle, "gmini - Num. Of Tri.: %d - FPS: %d", numOfTriangles, FPS);
	glutSetWindowTitle (winTitle);
	lastTime = currentTime;
  }
  glutPostRedisplay ();
}

void key (unsigned char keyPressed, int x, int y) {
  switch (keyPressed) {
  case 'f':
	if (fullScreen == true) {
	  glutReshapeWindow (SCREENWIDTH, SCREENHEIGHT);
	  fullScreen = false;
	} else {
	  glutFullScreen ();
	  fullScreen = true;
	}      
	break;
  case 'q':
  case 27:
	clear ();
	exit (0);
	break;
  case 'w':
	if (polygonMode == Wireframe) {
	  polygonMode = Flat;
	  glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	} else if (polygonMode == Flat) {
	  polygonMode = Gouraud;
	} else {
	  polygonMode = Wireframe;
	  glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
	}
	break;
  case '?':
  default:
	printUsage ();
	break;
  }
  idle ();
}

void mouse (int button, int state, int x, int y) {
  if (state == GLUT_UP) {
	mouseMovePressed = false;
	mouseRotatePressed = false;
	mouseZoomPressed = false;
  } else {
	if (button == GLUT_LEFT_BUTTON) {
	  camera.beginRotate (x, y);
	  mouseMovePressed = false;
	  mouseRotatePressed = true;
	  mouseZoomPressed = false;
	} else if (button == GLUT_RIGHT_BUTTON) {
	  lastX = x;
	  lastY = y;
	  mouseMovePressed = true;
	  mouseRotatePressed = false;
	  mouseZoomPressed = false;
	} else if (button == GLUT_MIDDLE_BUTTON) {
	  if (mouseZoomPressed == false) {
		lastZoom = y;
		mouseMovePressed = false;
		mouseRotatePressed = false;
		mouseZoomPressed = true;
	  }
	}
  }
  idle ();
}

void motion (int x, int y) {
  if (mouseRotatePressed == true) {
	camera.rotate (x, y);
  }
  else if (mouseMovePressed == true) {
	camera.move ((x-lastX)/static_cast<float>(SCREENWIDTH), (lastY-y)/static_cast<float>(SCREENHEIGHT), 0.0);
	lastX = x;
	lastY = y;
  }
  else if (mouseZoomPressed == true) {
	camera.zoom (float (y-lastZoom)/SCREENHEIGHT);
	lastZoom = y;
  }
}


void reshape(int w, int h) {
  camera.resize (w, h);
}


int main (int argc, char ** argv) {
  if (argc > 2) {
	printUsage ();
	exit (EXIT_FAILURE);
  }
  glutInit (&argc, argv);
  glutInitDisplayMode (GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
  glutInitWindowSize (SCREENWIDTH, SCREENHEIGHT);
  window = glutCreateWindow ("gMini");
  
  init (argc == 2 ? argv[1] : "sphere.off");
  glutIdleFunc (idle);
  glutDisplayFunc (display);
  glutKeyboardFunc (key);
  glutReshapeFunc (reshape);
  glutMotionFunc (motion);
  glutMouseFunc (mouse);
  key ('?', 0, 0);   
  glutMainLoop ();
  return EXIT_SUCCESS;
}

