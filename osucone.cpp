#include <stdio.h>
#include <math.h>
#include <GL/gl.h>

extern void Unit( float *, float * );

#ifndef POINT_H
#define POINT_H
struct point
{
	float x, y, z;		// coordinates
	float nx, ny, nz;	// surface normal
	float s, t;		// texture coords
};

inline
void
DrawPoint( struct point *p )
{
	glNormal3fv( &p->nx );
	glTexCoord2fv( &p->s );
	glVertex3fv( &p->x );
}
#endif

int		ConeNumLngs, ConeNumLats;
struct point *	ConePts;

inline
struct point *
ConePtsPointer( int lat, int lng )
{
	if( lat < 0 )			lat += (ConeNumLats-1);
	if( lng < 0 )			lng += (ConeNumLngs-0);
	if( lat > ConeNumLats-1 )	lat -= (ConeNumLats-1);
	if( lng > ConeNumLngs-1 )	lng -= (ConeNumLngs-0);
	return &ConePts[ ConeNumLngs*lat + lng ];
}


void
OsuCone( float radBot, float radTop, float height, int slices, int stacks )
{
	// gracefully handle degenerate case:

	if( radBot == 0.  &&  radTop == 0. )
	{
		glBegin( GL_LINES );
			glNormal3f( 0., -1., 0. );
			glTexCoord2f( 0., 0. );
			glVertex3f( 0., 0., 0. );

			glNormal3f( 0., 1., 0. );
			glTexCoord2f( 0., 1. );
			glVertex3f( 0., height, 0. );
		glEnd( );
		return;
	}


	radBot = fabs( radBot );
	radTop = fabs( radTop );
	slices = fabs( slices );
	stacks = fabs( stacks );
	//fprintf( stderr, "%8.3f, %8.3f, %8.3f,  %3d, %3d\n", radBot, radTop, height, slices, stacks );

	ConeNumLngs = slices;
	if( ConeNumLngs < 3 )
		ConeNumLngs = 3;

	ConeNumLats = stacks;
	if( ConeNumLats < 3 )
		ConeNumLats = 3;

	// allocate the point data structure:

	ConePts = new struct point[ ConeNumLngs * ConeNumLats ];

	// fill the ConePts structure:

	for( int ilat = 0; ilat < ConeNumLats; ilat++ )
	{
		float t = (float)ilat / (float)(ConeNumLats-1);
		float y = t * height;
		float rad = t * radTop + ( 1. - t ) * radBot;
		for( int ilng = 0; ilng < ConeNumLngs; ilng++ )
		{
			float lng = -M_PI  +  2. * M_PI * (float)ilng / (float)(ConeNumLngs-1);
			float x =  cos( lng );
			float z = -sin( lng );
			struct point *p = ConePtsPointer( ilat, ilng );
			p->x  = rad * x;
			p->y  = y;
			p->z  = rad * z;
			p->nx = height*x;
			p->ny = radBot - radTop;
			p->nz = height*z;
			Unit( &p->nx, &p->nx );
			p->s = (float)ilng / (float)(ConeNumLngs-1);
			p->t = (float)ilat / (float)(ConeNumLats-1);
		}
	}


	// draw the sides:

	for( int ilat = 0; ilat < ConeNumLats-1; ilat++ )
	{
		glBegin( GL_TRIANGLE_STRIP );

		struct point *p;
		p = ConePtsPointer( ilat,   0 );
		DrawPoint( p );

		p = ConePtsPointer( ilat+1, 0 );
		DrawPoint( p );

		for( int ilng = 1; ilng < ConeNumLngs; ilng++ )
		{
			p = ConePtsPointer( ilat,   ilng );
			DrawPoint( p );

			p = ConePtsPointer( ilat+1, ilng );
			DrawPoint( p );
		}

		glEnd( );
	}

	// draw the bottom circle:

	if( radBot != 0. )
	{
		struct point *bot = new struct point [ConeNumLngs];
		for( int ilng = 0; ilng < ConeNumLngs; ilng++ )
		{
			bot[ilng].x  = 0.;
			bot[ilng].y  = 0.;
			bot[ilng].z  = 0.;
			bot[ilng].nx =  0.;
			bot[ilng].ny = -1.;
			bot[ilng].nz =  0.;
			bot[ilng].s = (float)ilng / (float)(ConeNumLngs-1);
			bot[ilng].t = 0.;
		}

		glBegin( GL_TRIANGLES );
		for( int ilng = ConeNumLngs-1; ilng >= 0; ilng-- )
		{
			struct point *p;
			p = ConePtsPointer( 0, ilng+1 );
			DrawPoint( p );

			p = ConePtsPointer( 0, ilng );
			DrawPoint( p );

			DrawPoint( &bot[ilng] );
		}
		glEnd( );
		delete[ ] bot;
	}


	// draw the top circle:

	if( radTop != 0. )
	{
		struct point *top = new struct point [ConeNumLngs];
		for( int ilng = 0; ilng < ConeNumLngs; ilng++ )
		{
			top[ilng].x  = 0.;
			top[ilng].y  = height;
			top[ilng].z  = 0.;
			top[ilng].nx = 0.;
			top[ilng].ny = 1.;
			top[ilng].nz = 0.;
			top[ilng].s = (float)ilng / (float)(ConeNumLngs-1);
			top[ilng].t = 1.;
		}

		glBegin( GL_TRIANGLES );
		for( int ilng = 0; ilng < ConeNumLngs-1; ilng++ )
		{
			struct point *p;
			p = ConePtsPointer( ConeNumLats-1, ilng );
			DrawPoint( p );

			p = ConePtsPointer( ConeNumLats-1, ilng+1 );
			DrawPoint( p );

			DrawPoint( &top[ilng] );
		}
		glEnd( );
		delete [ ] top;
	}

	delete [ ] ConePts;
}
