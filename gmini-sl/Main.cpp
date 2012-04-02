// --------------------------------------------------------------------------
// gMini,
// a minimal Glut/OpenGL app to extend
//
// Copyright(C) 2007-2009
// Tamy Boubekeur
//
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License (http://www.gnu.org/licenses/gpl.txt)
// for more details.
//
// --------------------------------------------------------------------------

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>

#define GLEW_STATIC 1
#include <GL/glew.h>
#include <GL/glut.h>

#include "Shader.h"
#include "Vec3D.h"
#include "Vertex.h"
#include "Triangle.h"
#include "Mesh.h"
#include "Camera.h"

using namespace std;

class PhongShader : public Shader {
public:
  PhongShader () { init ("shader.vert", "shader.frag"); }
  inline virtual ~PhongShader () {}

  void setDiffuseRef (float s) {
	glUniform1fARB (diffuseRefLocation, s);
  }

  void setDepthEdgeThreshold (float s) {
	glUniform1fARB (depthEdgeThresholdLocation, s);
  }

  void setSpecRef (float s) {
	glUniform1fARB (specRefLocation, s);
  }

  void setShininess (float s) {
	glUniform1fARB (shininessLocation, s);
  }

private:
  void init (const std::string & vertexShaderFilename,
			 const std::string & fragmentShaderFilename) {
	loadFromFile (vertexShaderFilename, fragmentShaderFilename);
	bind ();
	//	diffuseRefLocation = getUniLoc ("diffuseRef");
	depthEdgeThresholdLocation = getUniLoc ("depthEdgeThreshold");
	specRefLocation = getUniLoc ("specRef");
	shininessLocation = getUniLoc ("shininess");
  }
  GLint diffuseRefLocation;
  GLint depthEdgeThresholdLocation;
  GLint specRefLocation;
  GLint shininessLocation;
};

static GLint window;
static unsigned int SCREENWIDTH = 1024;
static unsigned int SCREENHEIGHT = 768;
static Camera camera;
static bool mouseRotatePressed = false;
static bool mouseMovePressed = false;
static bool mouseZoomPressed = false;
static int lastX=0, lastY=0, lastZoom=0;
static unsigned int FPS = 0;
static bool fullScreen = false;

static PhongShader * phongShader;

static Mesh mesh;
static GLuint glID;

static float diffuseRef = 0.8f;
static float depthEdgeThreshold = 0.2f;
static float specRef = 0.5f;
static float shininess = 16.0f;

typedef enum {Solid, Phong} RenderingMode;
static RenderingMode mode = Phong;

Mesh openOFF (const std::string filename, unsigned int normWeight) {
  vector<Vertex> V;
  vector<Triangle> T;

  ifstream in (filename.c_str ());
  if (!in)
	exit (EXIT_FAILURE);
  string offString;
  unsigned int sizeV, sizeT, tmp;
  in >> offString >> sizeV >> sizeT >> tmp;
  for (unsigned int i = 0; i < sizeV; i++) {
	Vec3Df p;
	in >> p;
	V.push_back (Vertex (p));
  }
  int s;
  for (unsigned int i = 0; i < sizeT; i++) {
	in >> s;
	unsigned int v[3];
	for (unsigned int j = 0; j < 3; j++)
	  in >> v[j];
	T.push_back (Triangle (v[0], v[1], v[2]));
  }
  in.close ();

  Vec3Df center;
  float radius;
  Vertex::scaleToUnitBox (V, center, radius);
  Mesh mesh (V, T);
  mesh.recomputeSmoothVertexNormals (normWeight);
  return mesh;
}

inline void glVertexVec3Df (const Vec3Df & v) {
  glVertex3f (v[0], v[1], v[2]);
}

inline void glNormalVec3Df (const Vec3Df & n) {
  glNormal3f (n[0], n[1], n[2]);
}

inline void glDrawPoint (const Vec3Df & pos, const Vec3Df & normal) {
  glNormalVec3Df (normal);
  glVertexVec3Df (pos);
}

inline void glDrawPoint (const Vertex & v) {
  glDrawPoint (v.getPos (), v.getNormal ());
}

void setShaderValues () {
  phongShader->setDiffuseRef (diffuseRef);
  phongShader->setDepthEdgeThreshold (depthEdgeThreshold);
  phongShader->setSpecRef (specRef);
  phongShader->setShininess (shininess);
}

void drawMesh (bool flat) {
  const vector<Vertex> & V = mesh.getVertices ();
  const vector<Triangle> & T = mesh.getTriangles ();
  glBegin (GL_TRIANGLES);
  for (unsigned int i = 0; i < T.size (); i++) {
	const Triangle & t = T[i];
	if (flat) {
	  Vec3Df normal = Vec3Df::crossProduct (V[t.getVertex (1)].getPos ()
											- V[t.getVertex (0)].getPos (),
											V[t.getVertex (2)].getPos ()
											- V[t.getVertex (0)].getPos ());
	  normal.normalize ();
	  glNormalVec3Df (normal);
	}
	for (unsigned int j = 0; j < 3; j++)
	  if (!flat) {
		glNormalVec3Df (V[t.getVertex (j)].getNormal ());
		glVertexVec3Df (V[t.getVertex (j)].getPos ());
	  } else
		glVertexVec3Df (V[t.getVertex (j)].getPos ());
  }
  glEnd ();
}

void drawSolidModel () {
  glEnable (GL_LIGHTING);
  glEnable (GL_COLOR_MATERIAL);
  glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
  glPolygonOffset (1.0, 1.0);
  glEnable (GL_POLYGON_OFFSET_FILL);
  glShadeModel (GL_FLAT);
  phongShader->bind ();
  drawMesh (true);
  glPolygonMode (GL_FRONT, GL_LINE);
  glPolygonMode (GL_BACK, GL_FILL);
  glColor3f (0.0, 0.0, 0.0);
  drawMesh (true);
  glDisable (GL_POLYGON_OFFSET_FILL);
  glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
  glDisable (GL_COLOR_MATERIAL);
  glDisable (GL_LIGHTING);
  glShadeModel (GL_SMOOTH);
}

void drawPhongModel () {
  //    phongShader->bind ();
  //setShaderValues ();
  glCallList (glID);
}

void initLights () {
  GLfloat light_position0[4] = {-50, 50, -10, 0};
  GLfloat light_position1[4] = {42, 374, 161, 0};
  GLfloat light_position2[4] = {473, -351, -259, 0};
  GLfloat light_position3[4] = {-438, 167, -48, 0};

  GLfloat direction1[3] = {-42, -374, -161,};
  GLfloat direction2[3] = {-473, 351, 259};
  GLfloat direction3[3] = {438, -167, 48};

  GLfloat color1[4] = {1.0, 1.0, 1.0, 1};
  GLfloat color2[4] = {0.28, 0.39, 1.0, 1};
  GLfloat color3[4] = {1.0, 0.69, 0.23, 1};

  GLfloat specularColor1[4] = {0.8, 0.8, 0.8, 1};
  GLfloat specularColor2[4] = {0.8, 0.8, 0.8, 1};
  GLfloat specularColor3[4] = {0.8, 0.8, 0.8, 1};

  GLfloat ambient[4] = {0.3f, 0.3f, 0.3f, 0.5f};

  glLightfv (GL_LIGHT0, GL_POSITION, light_position0);

  glLightfv (GL_LIGHT1, GL_POSITION, light_position1);
  glLightfv (GL_LIGHT1, GL_SPOT_DIRECTION, direction1);
  glLightfv (GL_LIGHT1, GL_DIFFUSE, color1);
  glLightfv (GL_LIGHT1, GL_SPECULAR, specularColor1);

  glLightfv (GL_LIGHT2, GL_POSITION, light_position2);
  glLightfv (GL_LIGHT2, GL_SPOT_DIRECTION, direction2);
  glLightfv (GL_LIGHT2, GL_DIFFUSE, color2);
  glLightfv (GL_LIGHT2, GL_SPECULAR, specularColor2);

  glLightfv (GL_LIGHT3, GL_POSITION, light_position3);
  glLightfv (GL_LIGHT3, GL_SPOT_DIRECTION, direction3);
  glLightfv (GL_LIGHT3, GL_DIFFUSE, color3);
  glLightfv (GL_LIGHT3, GL_SPECULAR, specularColor3);

  glLightModelfv (GL_LIGHT_MODEL_AMBIENT, ambient);

  glEnable (GL_LIGHTING);
}

void setSunriseLight () {
  glDisable (GL_LIGHT0);
  glDisable (GL_LIGHT1);
  glDisable (GL_LIGHT2);
  glDisable (GL_LIGHT3);
}

void setSingleSpotLight () {
  glEnable (GL_LIGHT0);
  glDisable (GL_LIGHT1);
  glDisable (GL_LIGHT2);
  glDisable (GL_LIGHT3);
}

void setDefaultMaterial () {
  GLfloat material_color[4] = {1,1,1,1.0f};
  GLfloat material_specular[4] = {0.5,0.5,0.5,1.0};
  GLfloat material_ambient[4] = {0.0,0.0,0.0,1.0};

  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material_specular);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material_color);
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material_ambient);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 128);

  glDisable (GL_COLOR_MATERIAL);
}

void initGLList () {
  glID = glGenLists (1);
  glNewList (glID, GL_COMPILE);
  drawMesh (false);
  glEndList ();
}

void init (const std::string & filename) {
  glewInit();
  if (glewGetExtension ("GL_ARB_vertex_shader")        != GL_TRUE ||
	  glewGetExtension ("GL_ARB_shader_objects")       != GL_TRUE ||
	  glewGetExtension ("GL_ARB_shading_language_100") != GL_TRUE) {
	cerr << "Driver does not support OpenGL Shading Language" << endl;
	exit (EXIT_FAILURE);
  }
  if (glewGetExtension ("GL_ARB_vertex_buffer_object") != GL_TRUE) {
	cerr << "Driver does not support Vertex Buffer Objects" << endl;
	exit (EXIT_FAILURE);
  }

  camera.resize (SCREENWIDTH, SCREENHEIGHT);
  glClearColor (0.2f, 0.2f, 0.3f, 1.0f);

  initLights ();
  setSingleSpotLight ();
  setDefaultMaterial ();
  mesh = openOFF (filename, 0);
  initGLList ();

  try {
	phongShader = new PhongShader;
	phongShader->bind ();
	setShaderValues ();
  } catch (ShaderException e) {
	cerr << e.getMessage () << endl;
	exit (EXIT_FAILURE);
  }
}

void clear () {
  delete phongShader;
  glDeleteLists (glID, 1);
}

void reshape(int w, int h) {
  camera.resize (w, h);
}

void display () {
  glLoadIdentity ();
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  camera.apply ();
  if (mode == Solid)
	drawSolidModel ();
  else if (mode == Phong)
	drawPhongModel ();
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
	static char FPSstr [128];
	unsigned int numOfTriangles = mesh.getTriangles ().size ();
	if (mode == Solid)
	  sprintf (FPSstr, "gMini: %d tri. - solid shading - %d FPS.",
			   numOfTriangles, FPS);
	else if (mode == Phong)
	  sprintf (FPSstr, "gMini: %d tri. - Phong shading - %d FPS.",
			   numOfTriangles, FPS);
	glutSetWindowTitle (FPSstr);
	lastTime = currentTime;

  }
  glutPostRedisplay ();
}

void printUsage () {
  cerr << endl
	   << "--------------------------------------" << endl
	   << "gMini" << endl
	   << "--------------------------------------" << endl
	   << "Author : Tamy Boubekeur (http://www.telecom-paristech.fr/~boubek)" << endl
	   << "--------------------------------------" << endl
	   << "USAGE: ./Main <file>.off" << endl
	   << "--------------------------------------" << endl
	   << "Keyboard commands" << endl
	   << "--------------------------------------" << endl
	   << " ?: Print help" << endl
	   << " w: Toggle wireframe Mode" << endl
	   << " g: Toggle Gouraud shading Mode" << endl
	   << " f: Toggle full screen mode" << endl
	   << " e: toggle solid mode." << endl
	   << " r: toggle Phong shading mode." << endl
	   << " D/d: Increase/Decrease diffuse reflection" << endl
	   << " S/s: Increase/Decrease specular reflection" << endl
	   << " T/t: Increase/Decrease edge threshold" << endl
	   << " +/-: Increase/Decrease shininess" << endl
	   << " <drag>+<left button>: rotate model" << endl
	   << " <drag>+<right button>: move model" << endl
	   << " <drag>+<middle button>: zoom" << endl
	   << " q, <esc>: Quit" << endl << endl
	   << "--------------------------------------" << endl;
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
	glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
	phongShader->bind ();
	break;
  case 'g':
	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	phongShader->bind ();
	break;
  case 'e':
	mode = Solid;
	phongShader->unbind ();
	break;
  case 'r':
	mode = Phong;
	phongShader->bind ();
	break;
  case 'D':
	diffuseRef += 0.1f;
	phongShader->setDiffuseRef (diffuseRef);
	break;
  case 'd':
	if (diffuseRef > 0.1f) {
	  diffuseRef -= 0.1f;
	  phongShader->setDiffuseRef (diffuseRef);
	}
	break;
  case 'T':
	depthEdgeThreshold += 0.1f;
	phongShader->setDepthEdgeThreshold (depthEdgeThreshold);
	break;
  case 't':
	if (depthEdgeThreshold > 0.1f) {
	  depthEdgeThreshold -= 0.1f;
	  phongShader->setDepthEdgeThreshold (depthEdgeThreshold);
	}
	break;
  case 'S':
	specRef += 0.1f;
	phongShader->setSpecRef (specRef);
	break;
  case 's':
	if (specRef > 0.1f) {
	  specRef -= 0.1f;
	  phongShader->setSpecRef (specRef);
	}
	break;
  case '+':
	shininess += 1.0f;
	phongShader->setShininess (shininess);
	break;
  case '-':
	if (shininess > 1.0f) {
	  shininess -= 1.0f;
	  phongShader->setShininess (shininess);
	}
	break;
  case '?':
  default:
	printUsage ();
	break;
  }
  setShaderValues ();
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
  if (mouseRotatePressed == true)
	camera.rotate (x, y);
  else if (mouseMovePressed == true) {
	camera.move ((x-lastX)/static_cast<float>(SCREENWIDTH),
				 (lastY-y)/static_cast<float>(SCREENHEIGHT),
				 0.0);
	lastX = x;
	lastY = y;
  }
  else if (mouseZoomPressed == true) {
	camera.zoom (float (y-lastZoom)/SCREENHEIGHT);
	lastZoom = y;
  }
}

void usage () {
  printUsage ();
  exit (EXIT_FAILURE);
}



int main (int argc, char ** argv) {
  glutInit (&argc, argv);
  glutInitDisplayMode (GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
  glutInitWindowSize (SCREENWIDTH, SCREENHEIGHT);
  window = glutCreateWindow ( "gMini");

  if (argc != 2)
	usage ();

  init (string (argv[1]));

  glCullFace (GL_BACK);
  glEnable (GL_CULL_FACE);
  glutIdleFunc (idle);
  glutDisplayFunc (display);
  glutKeyboardFunc (key);
  glutReshapeFunc (reshape);
  glutMotionFunc (motion);
  glutMouseFunc (mouse);

  key ('?', 0, 0);

  glDepthFunc (GL_LESS);
  glEnable (GL_DEPTH_TEST);

  phongShader->bind ();
  glutMainLoop ();
  return EXIT_SUCCESS;
}
