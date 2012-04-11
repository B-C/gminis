#include <cstdio>
#include <GL/glut.h>

#include "Texture.h"

unsigned char* Texture::ppmRead(const  char* filename, unsigned int* width, unsigned int* height ) {

  FILE* fp;
  unsigned int i, w, h, d;
  unsigned char* image;
  char head[70];		// max line <= 70 in PPM (per spec).

  fp = fopen( filename, "rb" );
  if ( !fp ) {
	perror(filename);
	return nullptr;
  }

  // Grab first two chars of the file and make sure that it has the
  // correct magic cookie for a raw PPM file.
  fgets(head, 70, fp);
  // if (strncmp(head, "P6", 2)) {
  //    fprintf(stderr, "%s: Not a raw PPM file\n", filename);
  //    return nullptr;
  // }

  // Grab the three elements in the header (width, height, maxval).
  i = 0;
  while( i < 3 ) {
	fgets( head, 70, fp );
	if ( head[0] == '#' )		// skip comments.
	  continue;
	if ( i == 0 )
	  i += sscanf( head, "%d %d %d", &w, &h, &d );
	else if ( i == 1 )
	  i += sscanf( head, "%d %d", &h, &d );
	else if ( i == 2 )
	  i += sscanf( head, "%d", &d );
  }

  // Grab all the image data in one fell swoop.
  image = (unsigned char*) malloc( sizeof( unsigned char ) * w * h * 3 );

  for( unsigned int i = 0 ; i < w*h*3 ; i++) {
	unsigned int tmp;
	fscanf( fp, "%u", &tmp);
	image[i] = tmp;
  }

  fclose( fp );

  if(width)
	*width = w;
  if(height)
	*height = h;
  return image;

}

Texture::Texture(const char * textureFilename):on(false) {
  unsigned int texMapWidth;
  unsigned int texMapHeight;

  unsigned int texMapComponents = GL_RGB;
  unsigned int texMapFormat = GL_RGB;

  // unsigned int texMapLevel;
  // bool texMapGenerated;

  glGenTextures (1, &texMap);
  glBindTexture (GL_TEXTURE_2D, texMap);

  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  unsigned char* texMapData = ppmRead(textureFilename, &texMapWidth, &texMapHeight);

  gluBuild2DMipmaps (GL_TEXTURE_2D, texMapComponents, texMapWidth, texMapHeight,
					 texMapFormat, GL_UNSIGNED_BYTE, texMapData);
}
