#include <GL/glut.h>
#include <fstream>
#include <string>
#include <cmath>
#include <unordered_map>
#include <sstream>
#include <tuple>

#include <iostream>

#include "Mesh.h"

using namespace std;

string Mesh::getHashkey(unsigned i, unsigned j) const{
  unsigned min =i, max=j;
  if(j<i) { min = j; max=i;}
  stringstream ret(stringstream::in|stringstream::out);
  ret << min << "~" << max;
  return ret.str();
}

void Mesh::subdivideLoop(){
  //key (v1~v2), (new ind in V, nb of triangle that use that edge-1)
  unordered_map<string, tuple<unsigned, unsigned> > newVertex;
  vector<Triangle> newTriangle;

  const unsigned int sizeV = V.size();

  for(const Triangle &t : T) {
	/*	generate new vertex */
	auto aux = [&](unsigned a, unsigned b, unsigned c) {
	  string key = getHashkey(a, b);
	  auto v = newVertex.find(key);

	  if(v == newVertex.end()) { // don't exist
		V.push_back({(V[a].p+V[b].p)*3/8+V[c].p*1/8,
			  (V[a].n+V[b].n)*3+V[c].n});
		newVertex[key] = make_tuple(V.size()-1, 0);
	  }
	  else {
		unsigned ind, count;
		tie (ind, count) = v->second;

		V[ind].p+=(V[c].p*1/8);
		V[ind].n+=V[c].n;

		if(count)
		  V[ind].p*=(count+7)/(count+8);

		newVertex[key] = make_tuple(ind, count+1);
	  }
	};

	aux(t[0], t[1], t[2]);
	aux(t[0], t[2], t[1]);
	aux(t[1], t[2], t[0]);

	/* generate new triangles */
	for(unsigned int i = 0 ; i < 3 ; i++) {
	  newTriangle.push_back({t[i],
			get<0>(newVertex[getHashkey(t[i], t[(i+1)%3])]),
			get<0>(newVertex[getHashkey(t[i], t[(i+2)%3])]) });
	}

	newTriangle.push_back({	get<0>(newVertex[getHashkey(t[0],t[1])]),
		  get<0>(newVertex[getHashkey(t[1],t[2])]),
		  get<0>(newVertex[getHashkey(t[0],t[2])])});
  }

  voisins.clear();
  compute1voisinages();
  /* move even (old) vertices */
  for(unsigned int i = 0 ; i < sizeV ; i++) {
  	unsigned int n = voisins[i].size();
  	float alpha = 1.f/64.f*(40.f-pow(3.f+2.f*cos(2*M_PI/float(n)),2));

  	V[i].p*=(1-alpha);
  	V[i].n*=(1-alpha);

  	for(unsigned int vois : voisins[i]) {
	  V[i].p+=alpha/float(n)*V[vois].p;
	  V[i].n+=alpha/float(n)*V[vois].n;
  	}
  }

  for(Vertex &v : V)
	v.n.normalize();

  T = newTriangle;
  centerAndScaleToUnit();
  recomputeNormals();
}

array<Vec3Df, 2> Mesh::boundingBox() const {
  array<Vec3Df, 2> cube;
  cube[0]=cube[1]=V[0].p;

  for(const Vertex &v : V) {
	for(unsigned int i = 0 ; i<3 ; i++) {
	  cube[0][i] = min(cube[0][i], v.p[i]);
	  cube[1][i] = max(cube[1][i], v.p[i]);
	}
  }
  return cube;
}

Vec3D<int> Mesh::getIndice(Vec3Df p, Vec3Df o, Vec3Df pas) {
  Vec3Df res = p-o;
  res/=pas;
  return Vec3D<int>(floor(res[0]), floor(res[1]), floor(res[2]));
}

int Mesh::getIndice(Vec3D<int> ind,	unsigned int r) {
  return ind[2]*r*r +ind[1]*r +ind[0];
}

int Mesh::getIndice(Vec3Df p, Vec3Df o, Vec3Df pas,	unsigned int r) {
  return getIndice(getIndice(p, o, pas), r);
}

void Mesh::simplifyMesh(unsigned int r){
  array<Vec3Df, 2> cube = boundingBox();
  Vec3Df delta = (cube[1]-cube[0])/(r-1);
  vector<Vertex> grid;
  grid.resize(pow(r,3));

  // Generate grid
  vector<int> nb;
  nb.resize(grid.size());

  for(const Vertex &v : V) {
  	int i= getIndice(v.p, cube[0], delta, r);
  	grid[i].p += v.p;
  	grid[i].n += v.n;
	nb[i]++;
  }

  // Generate new vertexes vectors and map
  vector<int> indMap;
  indMap.resize(grid.size());
  vector<Vertex> newV;
  unsigned int newI =0;
  for(unsigned int i =0 ; i < grid.size() ; i++) {
	if(nb[i] != 0) {
	  grid[i].p/=nb[i];
	  grid[i].n.normalize();
	  newV.push_back(grid[i]);
	  indMap[i] = newI++;
	}
  }
  grid.clear();

  // reindex triangle;
  for(auto t=T.begin() ; t < T.end(); t++) {
	Vec3D<int> indice[3];

	for(int j=0 ; j<3 ; j++)
	  indice[j] = getIndice(V[t->v[j]].p, cube[0], delta);

	if(indice[0] == indice[1] ||
	   indice[0] == indice[2] ||
	   indice[2] == indice[1]) {
	  //removed triangle
	  T.erase(t);
	  t--;
	}
	else {
	  //reindex triangle on the grid
	  for(int j=0 ; j<3 ; j++)
	  	t->v[j] = indMap[getIndice(indice[j], r)];
	}
  }

  V=newV;
  voisins.clear(); // Neightborhood had changed
  centerAndScaleToUnit();
  recomputeNormals();
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

void Mesh::smooth(float alpha) {
  vector<Vec3Df> barycentre;
  barycentre.resize(V.size());
  if(voisins.size()==0)
	compute1voisinages();

  for(unsigned int i = 0 ; i < barycentre.size() ; i++) {
	barycentre[i] = Vec3Df(0,0,0);
	for(auto it : voisins[i]) {
	  barycentre[i] += V[it].p;
	}
	if(voisins[i].size() != 0) //check for when vertex in no triangle
	  barycentre[i] /= voisins[i].size();
  }

  for(unsigned int i = 0; i < V.size() ; i++) {
	V[i].p += alpha*(barycentre[i]-V[i].p);
  }

  centerAndScaleToUnit();
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
  initPostLoad();
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
  initPostLoad();
}

void Mesh::recomputeNormals () {
  for (unsigned int i = 0; i < V.size (); i++)
	V[i].n = Vec3Df (0.0, 0.0, 0.0);
  for (unsigned int i = 0; i < T.size (); i++) {
	Vec3Df e01 = V[T[i].v[1]].p -  V[T[i].v[0]].p;
	Vec3Df e02 = V[T[i].v[2]].p -  V[T[i].v[0]].p;

	if(ponderate_normal) {
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
	else {
	  Vec3Df n = Vec3Df::crossProduct (e01, e02);
	  n.normalize ();
	  for (unsigned int j = 0; j < 3; j++)
		V[T[i].v[j]].n += n;
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
