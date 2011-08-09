/*******************************************************************************
 *
 *       Filename:  glpclview.c
 *
 *    Description:  OpenGL viewer for pts point cloud files
 *
 *        Version:  0.3
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
		if ( strcmp( pch, "" ) ) {
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
			printf( "%u values read.\n", g_clouds[ idx ].pointcount );
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
	printf( "%u values read.\nPointcloud loaded.\n", g_clouds[ idx ].pointcount );

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
	maxdim = maxdim > maxval ? maxdim : maxval;
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

	if ( last_mousebtn == GLUT_LEFT_BUTTON ) {
		if ( mx >= 0 && my >= 0 ) {
			rot.tilt += ( y - my ) * invertroty / 4.0f;
			rot.pan  += ( x - mx ) * invertrotx / 4.0f;
			glutPostRedisplay();
		}
	} else if ( last_mousebtn == GLUT_RIGHT_BUTTON ) {
		if ( maxdim > 100 ) {
			translate.y -= ( y - my );
			translate.x += ( x - mx );
		} else if ( maxdim > 10 ) {
			translate.y -= ( y - my ) / 10.0f;
			translate.x += ( x - mx ) / 10.0f;
		} else {
			translate.y -= ( y - my ) / 100.0f;
			translate.x += ( x - mx ) / 100.0f;
		}
		glutPostRedisplay();
	}
	mx = x;
	my = y;

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
				last_mousebtn = button;
				mx = x;
				my = y;
				break;
			case 3: /* Mouse wheel up */
//				zoom *= 1.1f;
				translate.z += maxdim / 100.0f;
				glutPostRedisplay();
				break;
			case 4: /* Mouse wheel down */
//				zoom /= 1.1f;
				translate.z -= maxdim / 100.0f;
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

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnableClientState( GL_COLOR_ARRAY );
	glEnableClientState( GL_VERTEX_ARRAY );
	glLoadIdentity();

	/* Apply scale and translation. */
	glScalef( zoom, zoom, 1 );
	glTranslatef( translate.x, translate.y, translate.z );

	/* Apply rotation. */
	glRotatef( (int) rot.tilt, 1, 0, 0 );
	glRotatef( (int) rot.pan, 0, 1, 0 );

	/* Set point size */
	glPointSize( pointsize );

	int i;
	for ( i = 0; i < g_cloudcount; i++ ) {
		if ( g_clouds[i].enabled ) {
			/* Set vertex and color pointer. */
			glVertexPointer( 3, GL_FLOAT, 0, g_clouds[i].vertices );
			glColorPointer( 3, GL_FLOAT, 0, g_clouds[i].colors );
		
			/* Draw pointcloud */
			glDrawArrays( GL_POINTS, 0, g_clouds[i].pointcount );
		}
	}

	/* Reset ClientState */
	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_COLOR_ARRAY );

	glutSwapBuffers();

}


/*******************************************************************************
 *         Name:  keyPressed
 *  Description:  Handle keyboard control events.
 ******************************************************************************/
void keyPressed( unsigned char key, int x, int y ) {

	switch ( key ) {
		case 27:
			glutDestroyWindow( window );
			exit( EXIT_SUCCESS );
		case 'j': 
			translate.x = 0;
			translate.y = 0;
			translate.z = 0;
			rot.pan     = 0;
			rot.tilt    = 0;
			zoom        = 1;
			break;
		case '+': zoom        *= 1.1; break;
		case '-': zoom        /= 1.1; break;
		/* movement */
		case 'a': translate.x -= 0.1; break;
		case 'd': translate.x += 0.1; break;
		case 'w': translate.z += 0.1; break;
		case 's': translate.z -= 0.1; break;
		case 'q': translate.y += 0.1; break;
		case 'e': translate.y -= 0.1; break;
		/* Uppercase: fast movement */
		case 'A': translate.x -= 1; break;
		case 'D': translate.x += 1; break;
		case 'W': translate.z += 1; break;
		case 'S': translate.z -= 1; break;
		case 'Q': translate.y += 1; break;
		case 'E': translate.y -= 1; break;
		/* Other stuff. */
		case 'i': pointsize    = pointsize < 2 ? 1 : pointsize - 1; break;
		case 'o': pointsize    = 1.0; break;
		case 'p': pointsize   += 1.0; break;
		case 'x': invertrotx  *= -1;  break;
		case 'y': invertroty  *= -1;  break;
		case 'f': rot.tilt    += 180; break;
		/* Control pointclouds */
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
					 printf( "%d - %d\n", key, 0x30 );
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

   glMatrixMode( GL_MODELVIEW );
	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LEQUAL );

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

	window = glutCreateWindow( "ptsViewer" );

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
	g_clouds = (cloud *) malloc( (argc - 1) * sizeof( cloud ) );
	if ( !g_clouds ) {
		fprintf( stderr, "Could not allocate memory for pointclouds!\n" );
		exit( EXIT_FAILURE );
	}
	g_cloudcount = argc - 1;

	/* Load pts file */
	int i;
	for ( i = 0; i < g_cloudcount; i++ ) {
		g_clouds[i].vertices = NULL;
		g_clouds[i].colors   = NULL;
		loadPts( argv[ i + 1 ], i );
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
			"-- Keyboard: ---\n"
			" i,o,p       Increase, reset, decrease pointsize\n"
			" a,d         Move left, right\n"
			" w,s         Move forward, backward\n"
			" q,e         Move up, down\n"
			" A,D         Move left, right (fast)\n"
			" W,S         Move forward, backward (fast)\n"
			" Q,E         Move up, down (fast)\n"
			" j           Jump to start position\n"
			" f           Flip pointcloud\n"
			" y,x         Invert rotation\n"
			" +,-         Zoom in, out\n"
			" <esc>       Quit\n" );

}
