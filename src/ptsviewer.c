/*******************************************************************************
 *
 *       Filename:  ptsviewer.c
 *
 *    Description:  OpenGL viewer for pts point cloud files
 *
 *        Created:  05/11/2011 20:42:39 PM
 *       Compiler:  gcc
 *
 *         Author:  Lars Kiesow (lkiesow), lkiesow@uos.de
 *        Company:  Universität Osnabrück
 *
 ******************************************************************************/

#include "ptsviewer.h"


/*******************************************************************************
 *         Name:  load_pts
 *  Description:  Load a pts file into memory.
 ******************************************************************************/
void loadPts( char * ptsfile, size_t idx ) {

	printf( "Loading »%s«…\n", ptsfile );

	/* Open file */
	FILE * f = fopen( ptsfile, "r" );
	if ( !f ) {
		fprintf( stderr, "error: Could not open »%s«.\n", ptsfile );
		exit( EXIT_FAILURE );
	}

	/* Determine amount of values per line */
	char line[1024];
	fgets( line, 1023, f );
	int valcount = 0;
	char * pch = strtok( line, "\t " );
	while ( pch ) {
		if ( strcmp( pch, "" ) && strcmp( pch, "\n" ) ) {
			valcount++;
		}
		pch = strtok( NULL, "\t " );
	}

	/* Do we have color information in the pts file? */
	int read_color = valcount >= 6;
	/* Are there additional columns we dont want to have? */
	int dummy_count = valcount - ( read_color ? 6 : 3 );
	float dummy;

	g_clouds[ idx ].vertices = ( float * ) malloc( 3000000 * sizeof(float) );

	float * vert_pos = g_clouds[ idx ].vertices;
	if ( read_color ) {
		g_clouds[ idx ].colors = realloc( g_clouds[ idx ].colors, 3000000 * sizeof(float) );
	}
	float * color_pos = g_clouds[ idx ].colors;
	int i;

	/* Start from the beginning */
	fseek( f, 0, SEEK_SET );

	unsigned int maxval = 0;
	while ( !feof( f ) ) {
		fscanf( f, "%f %f %f", vert_pos, vert_pos+1, vert_pos+2 );
		vert_pos[2] *= -1; /* z */
		if ( abs( vert_pos[0] ) > maxval ) { maxval = abs( vert_pos[0] ); }
		if ( abs( vert_pos[1] ) > maxval ) { maxval = abs( vert_pos[1] ); }
		if ( abs( vert_pos[2] ) > maxval ) { maxval = abs( vert_pos[2] ); }
		vert_pos += 3;
		for ( i = 0; i < dummy_count; i++ ) {
			fscanf( f, "%f", &dummy );
		}
		if ( read_color ) {
			unsigned int r, g, b;
			fscanf( f, "%u %u %u", &r, &g, &b );
			*(color_pos++) = r / 255.0f;
			*(color_pos++) = g / 255.0f;
			*(color_pos++) = b / 255.0f;
		}
		g_clouds[ idx ].pointcount++;
		if ( g_clouds[ idx ].pointcount % 100000 == 0 ) {
			printf( "  %u values read.\r", g_clouds[ idx ].pointcount );
			fflush( stdout );
		}
		if ( g_clouds[ idx ].pointcount % 1000000 == 0 ) {
			/* Resize array (double the size). */
			g_clouds[ idx ].vertices = realloc( g_clouds[ idx ].vertices, 
					g_clouds[ idx ].pointcount * 6 * sizeof(float) );
			vert_pos = g_clouds[ idx ].vertices + 3 * g_clouds[ idx ].pointcount;
			if ( g_clouds[ idx ].colors ) {
				g_clouds[ idx ].colors = realloc( g_clouds[ idx ].colors, 
						g_clouds[ idx ].pointcount * 6 * sizeof(float) );
				color_pos = g_clouds[ idx ].colors + 3 * g_clouds[ idx ].pointcount;
			}
		}
	}
	g_clouds[ idx ].pointcount--;
	printf( "  %u values read.\nPointcloud loaded.\n", g_clouds[ idx ].pointcount );

	/* Fill color array if we did not get any color information from a file */
	if ( !g_clouds[ idx ].colors ) {
		g_clouds[ idx ].colors = realloc( g_clouds[ idx ].colors, 
				g_clouds[ idx ].pointcount * 3 * sizeof(float) );
		float * c = g_clouds[ idx ].colors;
		for ( ; c < g_clouds[ idx ].colors + ( g_clouds[ idx ].pointcount * 3 ); c++ ) {
			*c = 1.0f;
		}
	}

	if ( f ) {
		fclose( f );
	}

	/* Normalize size of pointclouds. */
	unsigned int factor = 1;
	while ( factor * 100 < maxval ) {
		factor *= 10;
	}
	g_maxdim = g_maxdim > maxval ? g_maxdim : maxval;
	/*
	if ( factor > 1 ) {
		printf( "Maximum value is %u => scale factor is 1/%u.\n", maxval, factor );
		printf( "Scaling points...\n" );
		for ( i = 0; i < g_clouds[ idx ].pointcount * 3; i++ ) {
			g_clouds[ idx ].vertices[i] /= factor;
		}
	}
	*/

}


/*******************************************************************************
 *         Name:  mouseMoved
 *  Description:  Handles mouse drag'n'drop for rotation.
 ******************************************************************************/
void mouseMoved( int x, int y ) {

	if ( g_last_mousebtn == GLUT_LEFT_BUTTON ) {
		if ( g_mx >= 0 && g_my >= 0 ) {
			g_rot.tilt += ( y - g_my ) * g_invertroty / 4.0f;
			g_rot.pan  += ( x - g_mx ) * g_invertrotx / 4.0f;
			glutPostRedisplay();
		}
	} else if ( g_last_mousebtn == GLUT_RIGHT_BUTTON ) {
		if ( g_maxdim > 100 ) {
			g_translate.y -= ( y - g_my );
			g_translate.x += ( x - g_mx );
		} else if ( g_maxdim > 10 ) {
			g_translate.y -= ( y - g_my ) / 10.0f;
			g_translate.x += ( x - g_mx ) / 10.0f;
		} else {
			g_translate.y -= ( y - g_my ) / 100.0f;
			g_translate.x += ( x - g_mx ) / 100.0f;
		}
		glutPostRedisplay();
	}
	g_mx = x;
	g_my = y;

}


/*******************************************************************************
 *         Name:  mousePress
 *  Description:  Start drag'n'drop and handle zooming per mouse wheel.
 ******************************************************************************/
void mousePress( int button, int state, int x, int y ) {

	if ( state == GLUT_DOWN ) {
		switch ( button ) {
			case GLUT_LEFT_BUTTON:
			case GLUT_RIGHT_BUTTON:
				g_last_mousebtn = button;
				g_mx = x;
				g_my = y;
				break;
			case 3: /* Mouse wheel up */
				g_translate.z += g_movespeed * g_maxdim / 100.0f;
				glutPostRedisplay();
				break;
			case 4: /* Mouse wheel down */
				g_translate.z -= g_movespeed * g_maxdim / 100.0f;
				glutPostRedisplay();
				break;
		}
	}

}


/*******************************************************************************
 *         Name:  drawScene
 *  Description:  Display point cloud.
 ******************************************************************************/
void drawScene() {

//	glColor4f(1.0, 1.0, 1.0, 1.0);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glEnableClientState( GL_COLOR_ARRAY );
	glEnableClientState( GL_VERTEX_ARRAY );
	/* Set point size */
	glPointSize( g_pointsize );

	int i;
	for ( i = 0; i < g_cloudcount; i++ ) {
		if ( g_clouds[i].enabled ) {
			glLoadIdentity();

			/* Apply scale, rotation and translation. */
			/* Global (all points) */
			glScalef( g_zoom, g_zoom, 1 );
			glTranslatef( g_translate.x, g_translate.y, g_translate.z );

			glRotatef( (int) g_rot.tilt, 1, 0, 0 );
			glRotatef( (int) g_rot.pan, 0, 1, 0 );

			/* local (this cloud only) */
			glTranslatef( g_clouds[i].trans.x, g_clouds[i].trans.y,
					g_clouds[i].trans.z );

			glRotatef( (int) g_clouds[i].rot.x, 1, 0, 0 );
			glRotatef( (int) g_clouds[i].rot.y, 0, 1, 0 );
			glRotatef( (int) g_clouds[i].rot.z, 0, 0, 1 );

			/* Set vertex and color pointer. */
			glVertexPointer( 3, GL_FLOAT, 0, g_clouds[i].vertices );
			glColorPointer(  3, GL_FLOAT, 0, g_clouds[i].colors );
		
			/* Draw pointcloud */
			glDrawArrays( GL_POINTS, 0, g_clouds[i].pointcount );
		}
	}

	/* Reset ClientState */
	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_COLOR_ARRAY );




	/* Print status of clouds at the top of the window. */
	glLoadIdentity();
	glTranslatef( 0, 0, -100 );

	char buf[64];
	int xpos = g_left;
	for ( i = 0; i < g_cloudcount; i++ ) {
		if ( g_clouds[i].selected ) {
			glColor3f( g_clouds[i].enabled ? 1.0 : 0.6, 0.0, 0.0 );
		} else {
			if ( g_clouds[i].enabled ) {
				glColor3f( 1.0, 1.0, 1.0 );
			} else {
				glColor3f( 0.6, 0.6, 0.6 );
			}
		}
		glRasterPos2i( xpos, 54 );
		sprintf( buf, "%d", i );
		int j;
		for ( j = 0; j < strlen( buf ); j++ ) {
			glutBitmapCharacter( GLUT_BITMAP_8_BY_13, buf[j] );
			xpos += 2;
		}
		xpos += 2;
	}

	/* Print selection at the bottom of the window. */
	if ( g_mode == VIEWER_MODE_SELECT ) {
		glLoadIdentity();
		glTranslatef( 0, 0, -100 );
		glColor4f( 1.0, 1.0, 1.0, 0.0 );
		glRasterPos2i( g_left, -54 );
		strcpy( buf, "SELECT: " );
		for ( i = 0; i < strlen( buf ); i++ ) {
			glutBitmapCharacter( GLUT_BITMAP_8_BY_13, buf[i] );
		}
		for ( i = 0; i < strlen( g_selection ); i++ ) {
			glutBitmapCharacter( GLUT_BITMAP_8_BY_13, g_selection[i] );
		}
	
	}

	/* Print mode sign at the bottom of the window. */
	if ( g_mode == VIEWER_MODE_MOVESEL ) {
		glLoadIdentity();
		glTranslatef( 0, 0, -100 );
		glColor4f( 1.0, 1.0, 1.0, 0.0 );
		glRasterPos2i( g_left, -54 );
		strcpy( buf, "MOVE (Press 'm' to leave this mode)" );
		for ( i = 0; i < strlen( buf ); i++ ) {
			glutBitmapCharacter( GLUT_BITMAP_8_BY_13, buf[i] );
		}
	}

	/* Draw coordinate axis */
	if ( g_showcoord ) {
		glLoadIdentity();
		glColor4f( 0.0, 1.0, 0.0, 0.0 );
		glScalef( g_zoom, g_zoom, 1 );
		glTranslatef( g_translate.x, g_translate.y, g_translate.z );
		glRotatef( (int) g_rot.tilt, 1, 0, 0 );
		glRotatef( (int) g_rot.pan,  0, 1, 0 );

		glRasterPos3f( g_maxdim,       0.0f,     0.0f );
		glutBitmapCharacter( GLUT_BITMAP_8_BY_13, 'X' );
		glRasterPos3f(     0.0f,   g_maxdim,     0.0f );
		glutBitmapCharacter( GLUT_BITMAP_8_BY_13, 'Y' );
		glRasterPos3f(     0.0f,       0.0f, - (float) g_maxdim );
		glutBitmapCharacter( GLUT_BITMAP_8_BY_13, 'Z' );

		glBegin( GL_LINES );
		glVertex3i(        0,        0,         0 );
		glVertex3i( g_maxdim,        0,         0 );
		glVertex3i(        0,        0,         0 );
		glVertex3i(        0, g_maxdim,         0 );
		glVertex3i(        0,        0,         0 );
		glVertex3i(        0,        0, -g_maxdim );
		glEnd();
	}

	glutSwapBuffers();

}


/*******************************************************************************
 *         Name:  selectionKey
 *  Description:  
 ******************************************************************************/
void selectionKey( unsigned char key ) {
	
	if ( key == 8 && strlen( g_selection ) ) { /* Enter selection */
		g_selection[ strlen( g_selection ) - 1 ] = 0;
	} else if ( ( key >= '0' && key <= '9' ) || key == ',' ) { /* Enter selection */
		sprintf( g_selection, "%s%c", g_selection, key );

	} else if ( key == 13 ) { /* Apply selection, go to normal mode */

		g_mode = VIEWER_MODE_NORMAL;
		char * s = g_selection;
		
		int sel;
		while ( strlen( s ) ) {
			/* Jump over comma. */
			if ( *s == ',' ) {
				s++;
				continue;
			}
			sel = strtol( s, &s, 0 );
			/* int sel = atoi( g_selection ); */
			/* Check if cloud exists */
			if ( sel < g_cloudcount ) {
				g_clouds[ sel ].selected = !g_clouds[ sel ].selected;
				printf( "Cloud %d %sselected\n", sel, 
						g_clouds[ sel ].selected ? "" : "un" );
			}
		}
		g_selection[0] = 0;

	} else if ( key == 27 ) { /* Just switch back to normal mode. */
		g_mode = VIEWER_MODE_NORMAL;
		g_selection[0] = 0;
	}
	glutPostRedisplay();

}


/*******************************************************************************
 *         Name:  moveKeyPressed
 *  Description:  
 ******************************************************************************/
void moveKeyPressed( unsigned char key ) {

#define FORALL  for ( i = 0; i < g_cloudcount; i++ ) {
#define FORSEL  for ( i = 0; i < g_cloudcount; i++ ) { if ( g_clouds[i].selected )
#define FORSELC for ( i = 0; i < g_cloudcount; i++ ) { if ( g_clouds[i].selected ) g_clouds[i]
#define FSEND } break

	int i;
	switch ( key ) {
		case 27:
		case 'm' : g_mode = VIEWER_MODE_NORMAL; break;
		/* movement */
		case 'a': FORSELC.trans.x -= 1 * g_movespeed; FSEND;
		case 'd': FORSELC.trans.x += 1 * g_movespeed; FSEND;
		case 'w': FORSELC.trans.z -= 1 * g_movespeed; FSEND;
		case 's': FORSELC.trans.z += 1 * g_movespeed; FSEND;
		case 'q': FORSELC.trans.y += 1 * g_movespeed; FSEND;
		case 'e': FORSELC.trans.y -= 1 * g_movespeed; FSEND;
		/* Uppercase: fast movement */
		case 'A': FORSELC.trans.x -= 0.1 * g_movespeed; FSEND;
		case 'D': FORSELC.trans.x += 0.1 * g_movespeed; FSEND;
		case 'W': FORSELC.trans.z -= 0.1 * g_movespeed; FSEND;
		case 'S': FORSELC.trans.z += 0.1 * g_movespeed; FSEND;
		case 'Q': FORSELC.trans.y += 0.1 * g_movespeed; FSEND;
		case 'E': FORSELC.trans.y -= 0.1 * g_movespeed; FSEND;
		/* Rotation */
		case 'r': FORSELC.rot.x -= 1; FSEND;
		case 'f': FORSELC.rot.x += 1; FSEND;
		case 't': FORSELC.rot.y -= 1; FSEND;
		case 'g': FORSELC.rot.y += 1; FSEND;
		case 'z': FORSELC.rot.z -= 1; FSEND;
		case 'h': FORSELC.rot.z += 1; FSEND;
		/* Precise rotations */
		case 'R': FORSELC.rot.x -= 0.1; FSEND;
		case 'F': FORSELC.rot.x += 0.1; FSEND;
		case 'T': FORSELC.rot.y -= 0.1; FSEND;
		case 'G': FORSELC.rot.y += 0.1; FSEND;
		case 'Z': FORSELC.rot.z -= 0.1; FSEND;
		case 'H': FORSELC.rot.z += 0.1; FSEND;
		/* Other stuff */
		case ' ': FORSELC.enabled = !g_clouds[i].enabled; FSEND;
		case 'p': FORALL printf( "%s: %f %f %f  %f %f %f\n", g_clouds[i].name,
							 g_clouds[i].trans.x, g_clouds[i].trans.y,
							 -g_clouds[i].trans.z, -g_clouds[i].rot.x,
							 -g_clouds[i].rot.y, g_clouds[i].rot.z); FSEND;
	}
	/* Generate and save pose files */
	if ( key == 'P' ) {
		char buf[1024];
		char buf2[1024];
		char * s;
		for ( i = 0; i < g_cloudcount; i++ ) {
			strcpy( buf, g_clouds[i].name );
			/* remove extension */
			if ( ( s = strrchr( buf, '.' ) ) ) {
				*s = 0;
			}
			sprintf( buf2, "./%s.pose", basename( buf ) );
			printf( "Saving pose file to %s\n", buf2 );
			FILE * f = fopen( buf2, "w" );
			if ( f ) {
				fprintf( f, "%f %f %f\n%f %f %f\n",
							 g_clouds[i].trans.x, g_clouds[i].trans.y,
							 -g_clouds[i].trans.z, -g_clouds[i].rot.x,
							 -g_clouds[i].rot.y, g_clouds[i].rot.z );
				fclose( f );
			}
		}

	/* Load .pose files */
	} else if ( key == 'l' || key == 'L' ) {
		char buf[1024];
		char buf2[1024];
		char * s;
		for ( i = 0; i < g_cloudcount; i++ ) {
			if ( !g_clouds[i].selected ) {
				continue;
			}
			strcpy( buf, g_clouds[i].name );
			/* remove extension */
			if ( ( s = strrchr( buf, '.' ) ) ) {
				*s = 0;
			}
			if ( key == 'l' ) {
				sprintf( buf2, "%s.pose", buf );
			} else {
				sprintf( buf2, "./%s.pose", basename( buf ) );
			}
			FILE * f = fopen( buf2, "r" );
			if ( f ) {
				printf( "Loading pose file from %s\n", buf2 );
				double tx, ty, tz, rx, ry, rz;
				fscanf( f, "%lf %lf %lf %lf %lf %lf", &tx, &ty, &tz, &rx, &ry, &rz );
				g_clouds[i].trans.x =  tx;
				g_clouds[i].trans.y =  ty;
				g_clouds[i].trans.z = -tz;
				g_clouds[i].rot.x   = -rx;
				g_clouds[i].rot.y   = -ry;
				g_clouds[i].rot.z   =  rz;
				fclose( f );
			}
		}
	}
	glutPostRedisplay();
	

}


/*******************************************************************************
 *         Name:  keyPressed
 *  Description:  Handle keyboard control events.
 ******************************************************************************/
void keyPressed( unsigned char key, int x, int y ) {

	if ( g_mode == VIEWER_MODE_SELECT ) {
		selectionKey( key );
		return;
	} else if ( g_mode == VIEWER_MODE_MOVESEL ) {
		moveKeyPressed( key );
		return;
	}

	float rgb[3];
	int i;
	switch ( key ) {
		case 27:
			glutDestroyWindow( g_window );
			exit( EXIT_SUCCESS );
		case 'j': 
			g_translate.x = 0;
			g_translate.y = 0;
			g_translate.z = 0;
			g_rot.pan     = 0;
			g_rot.tilt    = 0;
			g_zoom        = 1;
			break;
		case '+': g_zoom      *= 1.1; break;
		case '-': g_zoom      /= 1.1; break;
		/* movement */
		case 'a': g_translate.x += 1 * g_movespeed; break;
		case 'd': g_translate.x -= 1 * g_movespeed; break;
		case 'w': g_translate.z += 1 * g_movespeed; break;
		case 's': g_translate.z -= 1 * g_movespeed; break;
		case 'q': g_translate.y += 1 * g_movespeed; break;
		case 'e': g_translate.y -= 1 * g_movespeed; break;
		/* Uppercase: fast movement */
		case 'A': g_translate.x -= 0.1 * g_movespeed; break;
		case 'D': g_translate.x += 0.1 * g_movespeed; break;
		case 'W': g_translate.z += 0.1 * g_movespeed; break;
		case 'S': g_translate.z -= 0.1 * g_movespeed; break;
		case 'Q': g_translate.y += 0.1 * g_movespeed; break;
		case 'E': g_translate.y -= 0.1 * g_movespeed; break;
		/* Mode changes */
		case 13 : g_mode = VIEWER_MODE_SELECT; break;
		case 'm': g_mode = VIEWER_MODE_MOVESEL; break;
		/* Other stuff. */
		case 'i': g_pointsize   = g_pointsize < 2 ? 1 : g_pointsize - 1; break;
		case 'o': g_pointsize   = 1.0; break;
		case 'p': g_pointsize  += 1.0; break;
		case '*': g_movespeed  *= 10;  break;
		case '/': g_movespeed  /= 10;  break;
		case 'x': g_invertrotx *= -1;  break;
		case 'y': g_invertroty *= -1;  break;
		case 'f': g_rot.tilt   += 180; break;
		case 'z': g_clouds[0].rot.y += 1; break;
		case 'C': g_showcoord = !g_showcoord; break;
		case 'c': glGetFloatv( GL_COLOR_CLEAR_VALUE, rgb );
					/* Invert background color */
					if ( *rgb < 0.9 ) {
						glClearColor( 1.0f, 1.0f, 1.0f, 0.0f );
					} else {
						glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
					}
				 break;
	case 'u':
					 for ( i = 0; i < g_cloudcount; i++ ) {
						 g_clouds[i].selected = 0;
					 }
					 break;
	case 't':
					 for ( i = 0; i < g_cloudcount; i++ ) {
						 g_clouds[i].enabled = !g_clouds[i].enabled;
					 }
	}
	/* Control pointclouds */
	if ( key >= '0' && key <= '9' ) {
		if ( g_cloudcount > key - 0x30 ) {
			g_clouds[ key - 0x30 ].enabled = !g_clouds[ key - 0x30 ].enabled;
		}
	
	}
	glutPostRedisplay();

}


/*******************************************************************************
 *         Name:  resizeScene
 *  Description:  Handle resize of window.
 ******************************************************************************/
void resizeScene( int w, int h ) {

	glViewport( 0, 0, w, h );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 60, w / (float) h, 0, 200 );
	g_left = (int) ( -tan( 0.39 * w / h ) * 100 ) - 13;


   glMatrixMode( GL_MODELVIEW );
	glEnable(     GL_DEPTH_TEST );
	glDepthFunc(  GL_LEQUAL );

}


/*******************************************************************************
 *         Name:  init
 *  Description:  Do some initialization.
 ******************************************************************************/
void init() {

	/**
	 * Set mode for GLUT windows:
	 * GLUT_RGBA       Red, green, blue, alpha framebuffer.
	 * GLUT_DOUBLE     Double-buffered mode.
	 * GLUT_DEPTH      Depth buffering.
	 * GLUT_LUMINANCE  Greyscale color mode.
	 **/

	glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );

   glutInitWindowSize( 640, 480 );
	g_window = glutCreateWindow( "ptsViewer" );

	glutDisplayFunc(  &drawScene );
	glutReshapeFunc(  &resizeScene );
	glutKeyboardFunc( &keyPressed );
	glutMotionFunc(   &mouseMoved );
	glutMouseFunc(    &mousePress );

	/* Set black as background color */
	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );

}


/*******************************************************************************
 *         Name:  cleanup
 *  Description:  Free all allocated memory and do additional stuff.
 ******************************************************************************/
void cleanup() {

	int i;
	for ( i = 0; i < g_cloudcount; i++ ) {

		if ( g_clouds[i].vertices ) {
			free( g_clouds[i].vertices );
		}
		if ( g_clouds[i].colors ) {
			free( g_clouds[i].colors );
		}
	}
	if ( g_clouds ) {
	}

}


/*******************************************************************************
 *         Name:  main
 *  Description:  Main function
 ******************************************************************************/
int main( int argc, char ** argv ) {

	/* Initialize GLUT */
	glutInit( &argc, argv );
	init();

	/* Check if we have enough parameters */
	if ( argc < 2 ) {
		printf( "Usage: %s ptsfile1 [ptsfile2 ...]\n", *argv );
		exit( EXIT_SUCCESS );
	}

	/* Prepare array */
	g_clouds = (cloud_t *) malloc( (argc - 1) * sizeof( cloud_t ) );
	if ( !g_clouds ) {
		fprintf( stderr, "Could not allocate memory for pointclouds!\n" );
		exit( EXIT_FAILURE );
	}
	g_cloudcount = argc - 1;

	/* Load pts file */
	int i;
	for ( i = 0; i < g_cloudcount; i++ ) {
		memset( g_clouds + i, 0, sizeof( cloud_t ) );
		loadPts( argv[ i + 1 ], i );
		g_clouds[i].name = argv[ i + 1 ];
		g_clouds[i].enabled = 1;
	}

	/* Print usage information to stdout */
	printHelp();

	/* Run program */
	glutMainLoop();

	cleanup();
	return EXIT_SUCCESS;

}


/*******************************************************************************
 *         Name:  printHelp
 *  Description:  Prints control information.
 ******************************************************************************/
void printHelp() {

	printf( "\n=== CONTROLS: ======\n"
			"-- Mouse: ---\n"
			" drag left   Rotate pointcloud\n"
			" drag right  Move up/down, left/right\n"
			" wheel       Move forward, backward (fact)\n"
			"-- Keyboard (normal mode): ---\n"
			" i,o,p       Increase, reset, decrease pointsize\n"
			" a,d         Move left, right\n"
			" w,s         Move forward, backward\n"
			" q,e         Move up, down\n"
/*			" A,D         Move left, right (slow)\n"       */
/*			" W,S         Move forward, backward (slow)\n" */
/*			" Q,E         Move up, down (slow)\n"          */
			" j           Jump to start position\n"
/*			" f           Flip pointcloud\n"               */
/*			" y,x         Invert rotation\n"               */
			" +,-         Zoom in, out\n"
			" *,/         Increase/Decrease movement speed\n"
			" 0...9       Toggle visibility of pointclouds 0 to 9\n"
			" t           Toggle visibility of all pointclouds\n"
			" u           Unselect all clouds\n"
			" c           Invert background color\n"
			" C           Toggle coordinate axis\n"
			" <return>    Enter selection mode\n"
			" m           Enter move mode\n"
			" <esc>       Quit\n"
			"-- Keyboard (selection mode): ---\n"
			" 0..9        Enter cloud number\n"
			" <return>    Apply selection.\n"
			" <esc>       Cancel selection\n"
			"-- Keyboard (move mode): ---\n"
			" a,d         Move left, right (fast)\n"
			" w,s         Move forward, backward (fast)\n"
			" q,e         Move up, down (fast)\n"
/*			" A,D         Move left, right (slow)\n"       */
/*			" W,S         Move forward, backward (slow)\n" */
/*			" Q,E         Move up, down (slow)\n"          */
			" r,f         Rotate around x-axis\n"
			" t,g         Rotate around y-axis\n"
			" z,h         Rotate around z-axis\n"
/*			" R,F         Rotate around x-axis (slow)\n" */
/*			" T,G         Rotate around y-axis (slow)\n" */
/*			" Z,H         Rotate around z-axis (slow)\n" */
			" p           Print pose\n"
			" P           Generate pose files in current directory\n"
			" l           Load pose files for selected clouds from current directory.\n"
			" L           Load pose files for selected clouds from cloud directory.\n"
			" m,<esc>     Leave move mode\n"
			);

}
