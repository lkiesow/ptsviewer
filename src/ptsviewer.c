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


#define min(A,B) ((A)<(B) ? (A) : (B)) 
#define max(A,B) ((A)>(B) ? (A) : (B)) 


/*******************************************************************************
 *         Name:  ply_vertex_cb
 *  Description:  
 ******************************************************************************/
int plyVertexCb( p_ply_argument argument ) {

	struct { float* v; boundingbox_t* b; } * d;
	long int eol;
	ply_get_argument_user_data( argument, (void *) &d, &eol );
	if ( eol == 0 ) {
		*(d->v) = ply_get_argument_value( argument );
		if ( *(d->v) > d->b->max.x ) { d->b->max.x = *(d->v); }
		if ( *(d->v) < d->b->min.x ) { d->b->min.x = *(d->v); }
	} else if ( eol == 1 ) {
		*(d->v) = ply_get_argument_value( argument );
		if ( *(d->v) > d->b->max.y ) { d->b->max.y = *(d->v); }
		if ( *(d->v) < d->b->min.y ) { d->b->min.y = *(d->v); }
	} else if ( eol == 2 ) {
		*(d->v) = -ply_get_argument_value( argument );
		if ( *(d->v) > d->b->max.z ) { d->b->max.z = *(d->v); }
		if ( *(d->v) < d->b->min.z ) { d->b->min.z = *(d->v); }
	}
	d->v++;
	return 1;

}


/*******************************************************************************
 *         Name:  ply_color_cb
 *  Description:  
 ******************************************************************************/
int plyColorCb( p_ply_argument argument ) {

	uint8_t ** color;
	ply_get_argument_user_data( argument, (void *) &color, NULL );
	**color = (unsigned int) ply_get_argument_value( argument );
	(*color)++;
	return 1;

}


/*******************************************************************************
 *         Name:  loadPly
 *  Description:  Load a ply file into memory.
 ******************************************************************************/
void loadPly( char * filename, size_t idx ) {

	printf( "Loading »%s«…\n", filename );

	p_ply ply = ply_open( filename, NULL, 0, NULL );

	if ( !ply ) {
		fprintf( stderr, "error: Could not open »%s«.\n", filename );
		exit( EXIT_FAILURE );
	}
	if ( !ply_read_header( ply ) ) {
		fprintf( stderr, "error: Could not read header.\n" );
		exit( EXIT_FAILURE );
	}

	/* Check if there are vertices and get the amount of vertices. */
	char buf[256] = "";
	char elemname[256] = "point";
	const char * name = buf;
	long int nvertices = 0;
	long int count = 0;
	p_ply_element elem = NULL;
	while ( ( elem = ply_get_next_element( ply, elem ) ) ) {
		ply_get_element_info( elem, &name, &count );
		if ( !strcmp( name, "vertex" ) ) {
			nvertices = count;
			strcpy( elemname, "vertex" );
			p_ply_property prop = NULL;
			if ( g_clouds[ idx ].colors ) {
				free( g_clouds[ idx ].colors );
			}
			while ( ( prop = ply_get_next_property( elem, prop ) ) ) {
				ply_get_property_info( prop, &name, NULL, NULL, NULL );
				if ( !strcmp( name, "red" ) ) {
					/* We have color information */
					g_clouds[ idx ].colors = ( uint8_t * ) 
						realloc( g_clouds[ idx ].colors, nvertices * 3 * sizeof(uint8_t) );
				}
			}
		} else if ( !strcmp( name, "point" ) ) {
			nvertices = count;
			strcpy( elemname, "point" );
			p_ply_property prop = NULL;
			if ( g_clouds[ idx ].colors ) {
				free( g_clouds[ idx ].colors );
			}
			while ( ( prop = ply_get_next_property( elem, prop ) ) ) {
				ply_get_property_info( prop, &name, NULL, NULL, NULL );
				if ( !strcmp( name, "red" ) ) {
					/* We have color information */
					g_clouds[ idx ].colors = ( uint8_t * ) 
						realloc( g_clouds[ idx ].colors, nvertices * 3 * sizeof(uint8_t) );
				}
			}
			/* Point is more important than vertex. Thus we can stop immediately if
			 * we got this element. */
			break;
		}
	}
	if ( !nvertices ) {
		fprintf( stderr, "warning: No vertices in ply.\n" );
		return;
	}

	/* Allocate memory. */
	g_clouds[ idx ].pointcount = nvertices;
	nvertices++;
	g_clouds[ idx ].vertices = (float*) malloc( nvertices * 3 * sizeof(float) );
	
	uint8_t* color  = g_clouds[ idx ].colors;
	g_clouds[ idx ].boundingbox.min.x = DBL_MAX;
	g_clouds[ idx ].boundingbox.min.y = DBL_MAX;
	g_clouds[ idx ].boundingbox.min.z = DBL_MAX;
	g_clouds[ idx ].boundingbox.max.x = DBL_MIN;
	g_clouds[ idx ].boundingbox.max.y = DBL_MIN;
	g_clouds[ idx ].boundingbox.max.z = DBL_MIN;
	struct { float* v; boundingbox_t* b; } d = { 
		g_clouds[ idx ].vertices, &g_clouds[ idx ].boundingbox };

	/* Set callbacks. */
	nvertices = ply_set_read_cb( ply, elemname, "x", plyVertexCb, &d, 0 );
	            ply_set_read_cb( ply, elemname, "y", plyVertexCb, &d, 1 );
	            ply_set_read_cb( ply, elemname, "z", plyVertexCb, &d, 2 );

	if ( color ) {
		ply_set_read_cb( ply, elemname, "red",   plyColorCb, &color, 0 );
		ply_set_read_cb( ply, elemname, "green", plyColorCb, &color, 1 );
		ply_set_read_cb( ply, elemname, "blue",  plyColorCb, &color, 2 );
	}

	/* Read ply file. */
	if ( !ply_read( ply ) ) {
		fprintf( stderr, "error: could not read »%s«.\n", filename );
		exit( EXIT_FAILURE );
	}
	ply_close( ply );

	printf( "%ld values read.\nPoint cloud loaded.", nvertices );

	g_maxdim = max( max( max( g_maxdim, d.b->max.x - d.b->min.x ), 
				d.b->max.y - d.b->min.y ), d.b->max.z - d.b->min.z ); 
	g_bb.max.x = max( g_bb.max.x, d.b->max.x );
	g_bb.max.y = max( g_bb.max.y, d.b->max.y );
	g_bb.max.z = max( g_bb.max.z, d.b->max.z );
	g_bb.min.x = min( g_bb.min.x, d.b->min.x );
	g_bb.min.y = min( g_bb.min.y, d.b->min.y );
	g_bb.min.z = min( g_bb.min.z, d.b->min.z );

}


/*******************************************************************************
 *         Name:  countValuesPerLine
 *  Description:  Count the values in the current line of the given file.
 ******************************************************************************/
int countValuesPerLine( FILE * f ) {

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
	return valcount;

}


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
	int valcount_first_line = countValuesPerLine( f );
	int valcount = countValuesPerLine( f );

	/* Do we have color information in the pts file? */
	int read_color = valcount >= 6;
	/* Are there additional columns we dont want to have? */
	int dummy_count = valcount - ( read_color ? 6 : 3 );
	float dummy;

	g_clouds[ idx ].vertices = ( float * ) malloc( 3000000 * sizeof(float) );

	float * vert_pos = g_clouds[ idx ].vertices;
	if ( read_color ) {
		g_clouds[ idx ].colors = realloc( g_clouds[ idx ].colors, 3000000 * sizeof(uint8_t) );
	}
	uint8_t * color_pos = g_clouds[ idx ].colors;
	int i;

	/* Start from the beginning */
	fseek( f, 0, SEEK_SET );

	/* If amount of values in first line is different of the second line jump
	 * over the first line. */
	if ( valcount != valcount_first_line ) {
		char line[1024];
		fgets( line, 1023, f );
	}

	boundingbox_t bb = { 
		{ DBL_MAX, DBL_MAX, DBL_MAX }, 
		{ DBL_MIN, DBL_MIN, DBL_MIN } };
	while ( !feof( f ) ) {
		fscanf( f, "%f %f %f", vert_pos, vert_pos+1, vert_pos+2 );
		vert_pos[2] *= -1; /* z */
		if ( vert_pos[0] > bb.max.x ) { bb.max.x = vert_pos[0]; }
		if ( vert_pos[1] > bb.max.y ) { bb.max.y = vert_pos[1]; }
		if ( vert_pos[2] > bb.max.z ) { bb.max.z = vert_pos[2]; }
		if ( vert_pos[0] < bb.min.x ) { bb.min.x = vert_pos[0]; }
		if ( vert_pos[1] < bb.min.y ) { bb.min.y = vert_pos[1]; }
		if ( vert_pos[2] < bb.min.z ) { bb.min.z = vert_pos[2]; }
		vert_pos += 3;
		for ( i = 0; i < dummy_count; i++ ) {
			fscanf( f, "%f", &dummy );
		}
		if ( read_color ) {
			unsigned int r, g, b;
			fscanf( f, "%u %u %u", &r, &g, &b );
			*(color_pos++) = r;
			*(color_pos++) = g;
			*(color_pos++) = b;
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
						g_clouds[ idx ].pointcount * 6 * sizeof(uint8_t) );
				color_pos = g_clouds[ idx ].colors + 3 * g_clouds[ idx ].pointcount;
			}
		}
	}
	g_clouds[ idx ].pointcount--;
	g_clouds[ idx ].boundingbox = bb;
	printf( "  %u values read.\nPoint cloud loaded.\n", 
			g_clouds[ idx ].pointcount );

	if ( f ) {
		fclose( f );
	}

	g_maxdim = max( max( max( g_maxdim, bb.max.x - bb.min.x ), 
				bb.max.y - bb.min.y ), bb.max.z - bb.min.z ); 
	g_bb.max.x = max( g_bb.max.x, bb.max.x );
	g_bb.max.y = max( g_bb.max.y, bb.max.y );
	g_bb.max.z = max( g_bb.max.z, bb.max.z );
	g_bb.min.x = min( g_bb.min.x, bb.min.x );
	g_bb.min.y = min( g_bb.min.y, bb.min.y );
	g_bb.min.z = min( g_bb.min.z, bb.min.z );

}


/*******************************************************************************
 *         Name:  mouseMoved
 *  Description:  Handles mouse drag'n'drop for rotation.
 ******************************************************************************/
void mouseMoved( int x, int y ) {

	if ( g_last_mousebtn == GLUT_LEFT_BUTTON ) {
		if ( g_mx >= 0 && g_my >= 0 ) {
			g_rot.x += ( y - g_my ) * g_invertroty / 4.0f;
			g_rot.y += ( x - g_mx ) * g_invertrotx / 4.0f;
			glutPostRedisplay();
		}
	} else if ( g_last_mousebtn == GLUT_MIDDLE_BUTTON ) {
		if ( g_mx >= 0 && g_my >= 0 ) {
			g_rot.x += ( y - g_my ) * g_invertroty / 4.0f;
			g_rot.z += ( x - g_mx ) * g_invertrotx / 4.0f;
			glutPostRedisplay();
		}
	} else if ( g_last_mousebtn == GLUT_RIGHT_BUTTON ) {
		g_translate.y -= ( y - g_my ) / 1000.0f * g_maxdim;
		g_translate.x += ( x - g_mx ) / 1000.0f * g_maxdim;
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

	printf( "btn: %d\n", button );

	if ( state == GLUT_DOWN ) {
		switch ( button ) {
			case GLUT_LEFT_BUTTON:
			case GLUT_RIGHT_BUTTON:
			case GLUT_MIDDLE_BUTTON:
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

	glEnableClientState( GL_VERTEX_ARRAY );
	/* Set point size */
	glPointSize( g_pointsize );

	int i;
	for ( i = 0; i < g_cloudcount; i++ ) {
		if ( g_clouds[i].enabled ) {
			glLoadIdentity();

			/* Enable colorArray. */
			if ( g_clouds[i].colors ) {
				glEnableClientState( GL_COLOR_ARRAY );
			} else {
				/* Set cloudcolor to opposite of background color. */
				float rgb[3];
				glGetFloatv( GL_COLOR_CLEAR_VALUE, rgb );
				if ( *rgb < 0.5 ) {
					glColor3f( 1.0f, 1.0f, 1.0f );
				} else {
					glColor3f( 0.0f, 0.0f, 0.0f );
				}
			}

			/* Apply scale, rotation and translation. */
			/* Global (all points) */
			glScalef( g_zoom, g_zoom, 1 );
			glTranslatef( g_translate.x, g_translate.y, g_translate.z );

			glRotatef( (int) g_rot.x, 1, 0, 0 );
			glRotatef( (int) g_rot.y, 0, 1, 0 );
			glRotatef( (int) g_rot.z, 0, 0, 1 );

			glTranslatef( -g_trans_center.x, -g_trans_center.y, -g_trans_center.z );

			/* local (this cloud only) */
			glTranslatef( g_clouds[i].trans.x, g_clouds[i].trans.y,
					g_clouds[i].trans.z );

			glRotatef( (int) g_clouds[i].rot.x, 1, 0, 0 );
			glRotatef( (int) g_clouds[i].rot.y, 0, 1, 0 );
			glRotatef( (int) g_clouds[i].rot.z, 0, 0, 1 );

			/* Set vertex and color pointer. */
			glVertexPointer( 3, GL_FLOAT, 0, g_clouds[i].vertices );
			if ( g_clouds[i].colors ) {
				glColorPointer(  3, GL_UNSIGNED_BYTE, 0, g_clouds[i].colors );
			}
		
			/* Draw point cloud */
			glDrawArrays( GL_POINTS, 0, g_clouds[i].pointcount );

			/* Disable colorArray. */
			if ( g_clouds[i].colors ) {
				glDisableClientState( GL_COLOR_ARRAY );
			}
		}
	}

	/* Reset ClientState */
	glDisableClientState( GL_VERTEX_ARRAY );




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
		glRotatef( (int) g_rot.x, 1, 0, 0 );
		glRotatef( (int) g_rot.y, 0, 1, 0 );
		glRotatef( (int) g_rot.z, 0, 0, 1 );

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
		case '*': g_movespeed  *= 10;  break;
		case '/': g_movespeed  /= 10;  break;
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
			g_rot.x       = 0;
			g_rot.y       = 0;
			g_rot.z       = 0;
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
		case 'f': g_rot.y      += 180; break;
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
	/* Control point clouds */
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
 *         Name:  determineFileFormat
 *  Description:  
 ******************************************************************************/
uint8_t determineFileFormat( char * filename ) {
	
	char * ext = strrchr( filename, '.' );
	if ( !ext ) {
		return FILE_FORMAT_NONE;
	}
	if ( !strcmp( ext, ".pts" ) || !strcmp( ext, ".3d" ) ) {
		return FILE_FORMAT_UOS;
	} else if ( !strcmp( ext, ".ply" ) ) {
		FILE * f = fopen( filename, "r" );
		if ( f ) {
			char magic_number[5] = { 0 };
			fread( magic_number, 1, 4, f );
			if ( !strcmp( magic_number, "ply\n" ) ) {
				return FILE_FORMAT_PLY;
			}
			fclose( f );
		}
	}
	return FILE_FORMAT_NONE;

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
		fprintf( stderr, "Could not allocate memory for point clouds!\n" );
		exit( EXIT_FAILURE );
	}
	g_cloudcount = argc - 1;

	/* Load pts file */
	int i;
	for ( i = 0; i < g_cloudcount; i++ ) {
		memset( g_clouds + i, 0, sizeof( cloud_t ) );
		switch ( determineFileFormat( argv[ i + 1 ] ) ) {
			case FILE_FORMAT_PLY:
				loadPly( argv[ i + 1 ], i );
				break;
			case FILE_FORMAT_UOS:
			default:
				loadPts( argv[ i + 1 ], i );
		}
		g_clouds[i].name = argv[ i + 1 ];
		g_clouds[i].enabled = 1;
	}

	/* Calculate translation to middle of cloud. */
	g_trans_center.x = ( g_bb.max.x + g_bb.min.x ) / 2;
	g_trans_center.y = ( g_bb.max.y + g_bb.min.y ) / 2;
	g_trans_center.z = ( g_bb.max.z + g_bb.min.z ) / 2;
	g_translate.z = -fabs( g_bb.max.z - g_bb.min.z );

	/* Print usage information to stdout. */
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
			" drag left   Rotate point cloud (x/y axis)\n"
			" drag middle Rotate point cloud (x/z axis)\n"
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
/*			" f           Flip point cloud\n"               */
/*			" y,x         Invert rotation\n"               */
			" +,-         Zoom in, out\n"
			" *,/         Increase/Decrease movement speed\n"
			" 0...9       Toggle visibility of point clouds 0 to 9\n"
			" t           Toggle visibility of all point clouds\n"
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
