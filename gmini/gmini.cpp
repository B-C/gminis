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

#include <GL/glut.h>
#include "Camera.h"
#include "Mesh.h"
#include "AmbientOcclusion.h"
#include "Texture.h"

using namespace std;

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

// Ambient Occlusion
static bool ambientOcclusion = false;
static vector<Vec3Df> aoColor;
static float R = 0.09;
static const float AngleMax = 3.14/2;
static int N = 100;

// Texture
static Texture *texture;

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

void init (const char * modelFilename, const char * textureFilename) {
  camera.resize (SCREENWIDTH, SCREENHEIGHT);
  if(modelFilename)
	mesh.loadOFF (modelFilename);
  else
	mesh.makeSphere(10, 10);
  if(textureFilename)
	texture = new Texture(textureFilename);
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
  delete texture;
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
	  if(texture)
		texture->mapSphere(v.p);
	  if(ambientOcclusion) {
		Vec3Df color = aoColor[T[i].v[j]];
		glColor3f(color[0], color[1], color[2]);
	  }
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

void disableAO() {
  ambientOcclusion = false;
  glEnable (GL_LIGHTING);
  cout << "Ambient occlusion OFF" << endl;
}

void enableAO() {
  ambientOcclusion = true;
  glDisable (GL_LIGHTING);
  if(aoColor.size()==0) {
	cout << "generating ambient occlusion" << endl;
	aoColor = AmbientOcclusion(mesh, R, AngleMax, N).getColors();
  }
  cout << "Ambient occlusion ON" << endl;
}

void key (unsigned char keyPressed, int x, int y) {
#define statMesh()									\
  cout << "nb triangle: " << mesh.T.size()			\
	   << " - nb Vertex: " << mesh.V.size() << endl

#define lisse(n)								\
  mesh.smooth((n));								\
  aoColor.clear();								\
  disableAO();									\
  cout << "smooth - "#n << endl

#define simplify(n)								\
  statMesh();									\
  mesh.simplifyMesh((n));						\
  aoColor.clear();								\
  disableAO();									\
  cout << "simplified "#n"x"#n << endl;			\
  statMesh()

  switch (keyPressed) {
  case 't':
	if(texture)
	  texture->turnOnOff();
	break;
  case 'a':
	if(ambientOcclusion)
	  disableAO();
	else
	  enableAO();
	break;
  case 'n':
	if(polygonMode == Gouraud) {
	  if(mesh.changeNormalComputation())
		cout << "Ponderated normals computation" << endl;
	  else
		cout << "Uniform normals computation" << endl;
	}
	break;
  case '1':
	lisse(0.1);
	break;
  case '2':
	lisse(0.5);
	break;
  case '3':
	lisse(1);
	break;
  case '4':
	simplify(64);
	break;
  case '5':
	simplify(32);
	break;
  case '6':
	simplify(16);
	break;
  case '8':
	statMesh();
	mesh.subdivideLoop();
	cout << "subdivideLoop" << endl;
	statMesh();
	break;
  case 'r':
	mesh.reset();
	break;
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

#undef statMesh
#undef lisse
#undef simplify
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
  if (argc > 3) {
	printUsage ();
	exit (EXIT_FAILURE);
  }
  glutInit (&argc, argv);
  glutInitDisplayMode (GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
  glutInitWindowSize (SCREENWIDTH, SCREENHEIGHT);
  window = glutCreateWindow ("gMini");
  srand(time(NULL));

  init (argc >= 2 ? argv[1] : NULL, argc == 3 ? argv[2] : NULL);
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
