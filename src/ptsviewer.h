/*******************************************************************************
 *
 *       Filename:  glpclview.h
 *
 *    Description:  OpenGL viewer for pts point cloud files
 *
 *        Version:  0.2
 *        Created:  06/11/2011 08:42:39 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lars Kiesow (lkiesow), lkiesow@uos.de
 *        Company:  Universität Osnabrück
 *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <stdlib.h>
#include <math.h>


void load_pts( char * ptsfile );
void mouseMoved( int x, int y );
void mousePress( int button, int state, int x, int y );
void drawScene();
void keyPressed( unsigned char key, int x, int y );
void resizeScene( int w, int h );
void init();
int main( int argc, char ** argv );
void printHelp();

struct {
	GLdouble x;
	GLdouble y;
	GLdouble z;
} translate = { 
	0.0, /* x */
	0.0, /* y */
	0.0 /* z */
};

int window;
int mx = -1;
int my = -1;
// int rotangles[2] = {0};
struct {
	float pan;
	float tilt;
} rot = {
	0.0f,
	0.0f
};
int invertrotx = -1;
int invertroty = -1;
float zoom = 1;
int color = 1;
float pointsize = 1.0f;
float * vertices = NULL;
float * colors = NULL;
uint32_t count = 0;
