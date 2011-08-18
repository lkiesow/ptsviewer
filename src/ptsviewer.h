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
#include <libgen.h>
#include <math.h>


/* Functions */
void loadPts( char * ptsfile, size_t idx );
void mouseMoved( int x, int y );
void mousePress( int button, int state, int x, int y );
void drawScene();
void keyPressed( unsigned char key, int x, int y );
void resizeScene( int w, int h );
void init();
int main( int argc, char ** argv );
void printHelp();

/* Type definitions */
typedef struct {
	GLdouble x;
	GLdouble y;
	GLdouble z;
} coord3d;

typedef struct {
	float *  vertices;
	float *  colors;
	uint32_t pointcount;
	int      enabled;
	coord3d  trans;
	coord3d  rot;
	int      selected;
	char *   name;
} cloud;

/* Global variables */
coord3d translate = { 
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
int last_mousebtn;
int invertrotx = -1;
int invertroty = -1;
float zoom = 1;
int color = 1;
float pointsize  = 1.0f;
cloud * g_clouds = NULL;
uint32_t g_cloudcount;
uint32_t maxdim  = 0;
char g_selection[1024] = "";

/* Define viewer modes */

#define VIEWER_MODE_NORMAL  0
#define VIEWER_MODE_SELECT  1
#define VIEWER_MODE_MOVESEL 2

int g_mode = VIEWER_MODE_NORMAL;
