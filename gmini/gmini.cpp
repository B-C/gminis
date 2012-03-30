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
#include <list>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cmath>

#include <GL/glut.h>
#include "Camera.h"
#include "Mesh.h"

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
  mesh.compute1voisinages();
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

static float smooth_coeff = 0;

void key (unsigned char keyPressed, int x, int y) {
  switch (keyPressed) {
  case 'a':
	if (smooth_coeff <0.99)
	  smooth_coeff+=0.1;
	cout << " Alpha (smooth) "<< smooth_coeff << endl;
	break;
  case 'z':
	if (smooth_coeff >0.01)
	  smooth_coeff-=0.1;
	cout << " Alpha (smooth) "<< smooth_coeff << endl;
	break;
  case 's':
	mesh.smooth(smooth_coeff);
	break;
  case '4':
	mesh.reset();
	mesh.simplifyMesh(64);
	cout << "simplified 64x64" << endl;
	break;
  case '5':
	mesh.reset();
	mesh.simplifyMesh(32);
	cout << "simplified 32x32" << endl;
	break;
  case '6':
	mesh.reset();
	mesh.simplifyMesh(16);
	cout << "simplified 16x16" << endl;
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

