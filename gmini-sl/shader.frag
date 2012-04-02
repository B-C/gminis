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

uniform float specRef;
uniform float shininess;
uniform float depthEdgeThreshold;

varying vec4 P;
varying vec3 N;
varying float depth;

void main (void) {
  gl_FragColor = vec4 (0.0, 0.0, 0.0, 1);

  int i = 1;

  vec3 p = vec3 (gl_ModelViewMatrix * P);
  vec3 n = normalize (gl_NormalMatrix * N);
  vec3 l = normalize (gl_LightSource[i].position.xyz - p);

  vec3 r = reflect (-l, n);
  vec3 v = normalize (-p);

  float diffuse = max(dot(l, n), 0.0);
  float spec =pow(max(dot(r, v), 0.0),shininess);

  if(abs(dot(n, v)) > depthEdgeThreshold) {
	if(spec >= specRef)
	  gl_FragColor += vec4 (1.0,1.0,1.0, 1);
	else
	  gl_FragColor += vec4 (1.0,0.0,0.0, 1);
  }

}
