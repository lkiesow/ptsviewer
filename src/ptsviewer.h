/*******************************************************************************
 *
 *       Filename:  ptsviewer.h
 *
 *    Description:  OpenGL viewer for pts point cloud files
 *
 *        Created:  06/11/2011 08:42:39 PM
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
#include <rply.h>

#define FILE_FORMAT_NONE 0
#define FILE_FORMAT_UOS  1
#define FILE_FORMAT_PLY  2

/* Functions */
void loadPts( char * ptsfile, size_t idx );
void mouseMoved( int x, int y );
void mousePress( int button, int state, int x, int y );
void drawScene();
void keyPressed( unsigned char key, int x, int y );
void resizeScene( int w, int h );
void init();
int  main( int argc, char ** argv );
void printHelp();
uint8_t determineFileFormat( char * filename );

/* Type definitions */
typedef struct {
	GLdouble x;
	GLdouble y;
	GLdouble z;
} coord3d_t;

typedef struct {
	float *    vertices;
	uint8_t *  colors;
	uint32_t   pointcount;
	int        enabled;
	coord3d_t  trans;
	coord3d_t  rot;
	int        selected;
	char *     name;
} cloud_t;

typedef struct {
	float pan;
	float tilt;
} pantilt_t;



/* Global variables */
coord3d_t g_translate       =  { 0.0, 0.0, 0.0 };
pantilt_t g_rot             =     { 0.0f, 0.0f };
int       g_window          =                  0;
int       g_mx              =                 -1;
int       g_my              =                 -1;
int       g_last_mousebtn   =                 -1;
int       g_invertrotx      =                 -1;
int       g_invertroty      =                 -1;
float     g_zoom            =                  1;
int       g_color           =                  1;
float     g_pointsize       =               1.0f;
cloud_t * g_clouds          =               NULL;
uint32_t  g_cloudcount      =                  0;
float     g_maxdim          =                  0;
int       g_showcoord       =                  0;
char      g_selection[1024] =                 "";
float     g_movespeed       =                  1;
int       g_left            =                -75;

/* Define viewer modes */

#define VIEWER_MODE_NORMAL  0
#define VIEWER_MODE_SELECT  1
#define VIEWER_MODE_MOVESEL 2

int       g_mode            = VIEWER_MODE_NORMAL;
