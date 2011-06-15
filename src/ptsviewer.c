/*******************************************************************************
 *
 *       Filename:  glpclview.c
 *
 *    Description:  OpenGL viewer for pts point cloud files
 *
 *        Version:  1.0
 *        Created:  05/11/2011 20:42:39 PM
 *       Revision:  none
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
void load_pts( char * ptsfile ) {

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

	vertices = realloc( vertices, 3000000 * sizeof(float) );
	float * vert_pos = vertices;
	if ( read_color ) {
		colors = realloc( colors,   3000000 * sizeof(float) );
	}
	float * color_pos = colors;
	int i;

	/* Start from the beginning */
	fseek( f, 0, SEEK_SET );

	while ( !feof( f ) ) {
		fscanf( f, "%f %f %f", vert_pos, vert_pos+1, vert_pos+2 );
		vert_pos[1] *= -1;
		vert_pos[2] *= -1;
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
		count++;
		if ( count % 100000 == 0 ) {
			printf( "%u values read.\n", count );
		}
		if ( count % 1000000 == 0 ) {
			/* Resize array (double the size). */
			vertices = realloc( vertices, count * 6 * sizeof(float) );
			vert_pos = vertices + 3 * count;
			if ( colors ) {
				colors = realloc( colors, count * 6 * sizeof(float) );
				color_pos = colors + 3 * count;
			}
		}
	}
	count--;
	printf( "%u values read.\nPointcloud loaded.\n", count );

	if ( f ) {
		fclose( f );
	}

}


/*******************************************************************************
 *         Name:  mouseMoved
 *  Description:  Handles mouse drag'n'drop for rotation.
 ******************************************************************************/
void mouseMoved( int x, int y ) {

	if ( mx >= 0 && my >= 0 ) {
		rot.tilt += ( y - my ) * invertroty / 4.0f;
		rot.pan  += ( x - mx ) * invertrotx / 4.0f;
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
				mx = x;
				my = y;
				break;
			case 3: /* Mouse wheel up */
				zoom *= 1.1f;
				glutPostRedisplay();
				break;
			case 4: /* Mouse wheel down */
				zoom /= 1.1f;
				glutPostRedisplay();
				break;
		}
	}
	if ( button == GLUT_LEFT_BUTTON && state == GLUT_UP ) {
		mx = -1;
		my = -1;
	}

}


/*******************************************************************************
 *         Name:  drawScene
 *  Description:  Display point cloud.
 ******************************************************************************/
void drawScene() {

	if ( colors ) {
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		glEnableClientState( GL_COLOR_ARRAY );
	} else {
		glClear( GL_COLOR_BUFFER_BIT );
	}
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

	/* Set vertex and color pointer. */
	glVertexPointer( 3, GL_FLOAT, 0, vertices );
	if ( colors ) {
		glColorPointer( 3, GL_FLOAT, 0, colors );
	}
	
	/* Draw pointcloud */
	glDrawArrays( GL_POINTS, 0, count );

	/* Reset ClientState */
	glDisableClientState( GL_VERTEX_ARRAY );
	if ( colors ) {
		glDisableClientState( GL_COLOR_ARRAY );
	}

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
		case '+': zoom        *= 1.1; break;
		case '-': zoom        /= 1.1; break;
		case 'a': translate.x -= 0.1; break;
		case 'd': translate.x += 0.1; break;
		case 'w': translate.z += 0.1; break;
		case 's': translate.z -= 0.1; break;
		case 'q': translate.y += 0.1; break;
		case 'e': translate.y -= 0.1; break;
		case 'i': pointsize   /= 1.1; break;
		case 'o': pointsize    = 1.0; break;
		case 'p': pointsize   *= 1.1; break;
		case 'x': invertrotx  *= -1;  break;
		case 'y': invertroty  *= -1;  break;
		case 'f': rot.tilt    += 180; break;
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

	if ( colors ) {
		glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
	} else {
		glutInitDisplayMode( GLUT_LUMINANCE | GLUT_DOUBLE | GLUT_DEPTH );
	}

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

	if ( vertices ) {
		free( vertices );
	}
	if ( colors ) {
		free( colors );
	}

}


/*******************************************************************************
 *         Name:  main
 *  Description:  Main function
 ******************************************************************************/
int main( int argc, char ** argv ) {

	if ( argc != 2 ) {
		printf( "Usage: %s ptsfile\n", *argv );
		exit( EXIT_SUCCESS );
	}

	/* Load pts file */
	load_pts( argv[1] );

	printHelp();

	/* Initialize GLUT */
	glutInit( &argc, argv );
	init();

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
			" drag'n'drop  Rotate pointcloud\n"
			" mousewheel   Zoom\n"
			" i,o,p        Increase, reset, decrease Pointsize\n"
			" a,d          Move left, right\n"
			" w,s          Move forward, backward\n"
			" q,e          Move up, down\n"
			" f            Flip pointcloud\n"
			" y,x          Invert rotation\n"
			" +,-          Zoom in, out\n"
			" <esc>        Quit\n" );

}
