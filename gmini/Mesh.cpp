#include <GL/glut.h>
#include <fstream>
#include <string>
#include <cmath>
#include <vector>
#include <list>

#include <iostream>

#include "Vec3D.h"
#include "Mesh.h"

using namespace std;

vector<Vec3Df> Mesh::getCube() const {
  vector<Vec3Df> cube;
  cube.resize(2);
  cube[0]=cube[1]=V[0].p;

  for(vector<Vertex>::const_iterator it=V.begin() ; it < V.end(); it++) {
	for(unsigned int i = 0 ; i<3 ; i++) {
	  cube[0][i] = min(cube[0][i], it->p[i]);
	  cube[1][i] = max(cube[1][i], it->p[i]);
	}
  }
  // for(unsigned int j = 0 ; j<2 ; j++)
  // 	for(unsigned int i = 0 ; i<3 ; i++)
  // 	  cout << cube[j][i] <<endl;

  return cube;
}

void Mesh::simplifyMesh(unsigned int r){
  r=16;
  vector<Vec3Df> cube = getCube();
  float delta[3];

  for(unsigned int i=0 ; i<3 ; i++) {
	delta[i] = (cube[1][i]-cube[0][i])/r;
  }

  cout << delta[0] << " " << delta[1] << " "<< delta[2] << endl;
  // Grid

  vector<Vertex> grid;
  grid.resize(pow(r,3));

  for(vector<Vertex>::const_iterator v=V.begin() ; v < V.end(); v++) {
	int ind[3];
	for(unsigned int i=0 ; i<3 ; i++)
	  ind[i]=(int)(v->p[i]-cube[0][i])/delta[i];

	int i= ind[2]*r*r +ind[1]*r +ind[0];

	grid[i].p += v->p;
	grid[i].n += v->n;
  }

  for(vector<Vertex>::iterator v=grid.begin() ; v < grid.end(); v++) {
	v->p.normalize();
	v->n.normalize();
  }

  // reindex triangle;
  for(vector<Triangle>::iterator t=T.begin() ; t < T.end(); t++) {
	int indice[3];

	for(int j=0 ; j<3 ; j++) {
	  int ind[3];
	  for(unsigned int i=0 ; i<3 ; i++)
		ind[i]=(int)((V[t->v[j]].p[i]-cube[0][i])/delta[i]);

	  indice[j]= ind[2] +ind[1]*r +ind[0]*r*r;
	}

	if(indice[0] == indice[1] ||
	   indice[0] == indice[2] ||
	   indice[2] == indice[1]) {

	  // cout << "vertex " << endl;
	  // for(unsigned int j=0 ; j<3 ; j++){
	  // 	for(unsigned int i = 0 ; i< 3 ; i++)
	  // 	  cout << V[t->v[j]].p[i] << " ";
	  // 	cout << endl;
	  // }

	  // cout << "\nvertex-cubemin " << endl;
	  // for(unsigned int j=0 ; j<3 ; j++){
	  // 	for(unsigned int i = 0 ; i< 3 ; i++)
	  // 	  cout << V[t->v[j]].p[i] - cube[0][i]<< " ";
	  // 	cout << endl;
	  // }

	  // cout << "vertex-cubemin / delta " << endl;
	  // for(unsigned int j=0 ; j<3 ; j++){
	  // 	cout "delta " << delta[j] << " - ";
	  // 	for(unsigned int i = 0 ; i< 3 ; i++)
	  // 	  cout << V[t->v[j]].p[i] << " ";
	  // 	cout << endl;
	  // }


	  cout << indice[0] << " " << indice[1] << " "<< indice[2] << endl;
	  T.erase(t);
	}
	else {
	}
	cout << endl << endl;
	int toto;
	cin >> toto;
  }


}

void Mesh::compute1voisinages(){
  voisins.resize(V.size());

  for(unsigned int i = 0 ; i < T.size() ; i++) {
	for(unsigned int j = 0 ; j < 3 ; j++) {
	  voisins[T[i].v[j]].push_front(T[i].v[(j+1)%3]);
	  voisins[T[i].v[j]].push_front(T[i].v[(j+2)%3]);
	}
  }

  for(unsigned int i = 0; i < voisins.size(); i++) {
	voisins[i].sort();
	voisins[i].unique();
  }
}

void Mesh::reset() {
  V=V_orig;
}

void Mesh::smooth(float alpha) {
  vector<Vec3Df> barycentre;
  barycentre.resize(V.size());

  for(unsigned int i = 0; i < voisins.size(); i++) {
	barycentre[i] = Vec3Df(0,0,0);
	for(list<int>::iterator it=voisins[i].begin() ;
		it != voisins[i].end(); it++ ) {
	  barycentre[i] += V[*it].p;
	}
	barycentre[i] /= voisins[i].size();
  }

  for(unsigned int i = 0; i < V.size() ; i++) {
	V[i].p += alpha*(barycentre[i]-V[i].p);
  }

  centerAndScaleToUnit ();
  recomputeNormals();
}

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
  V_orig = V;
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