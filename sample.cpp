#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string>

#define _USE_MATH_DEFINES
#include <math.h>

#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#endif

#include "glew.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include "glut.h"
#include "Header.h"
#include "osusphere.cpp"
#include "osutorus.cpp"


//	This is a sample OpenGL / GLUT program
//
//	The objective is to draw a 3d object and change the color of the axes
//		with a glut menu
//
//	The left mouse button does rotation
//	The middle mouse button does scaling
//	The user interface allows:
//		1. The axes to be turned on and off
//		2. The color of the axes to be changed
//		3. Debugging to be turned on and off
//		4. Depth cueing to be turned on and off
//		5. The projection to be changed
//		6. The transformations to be reset
//		7. The program to quit
//
//	Author:			David Cornett

// title of these windows:

const char *WINDOWTITLE = "OpenGL / FTL Solar System Tour -- David Cornett";
const char *GLUITITLE   = "User Interface Window";

// what the glui package defines as true and false:

const int GLUITRUE  = true;
const int GLUIFALSE = false;

// the escape key:

const int ESCAPE = 0x1b;

// initial window size:

const int INIT_WINDOW_SIZE = 800;

// size of the 3d box to be drawn:

const float BOXSIZE = 2.f;

// multiplication factors for input interaction:
//  (these are known from previous experience)

const float ANGFACT = 1.f;
const float SCLFACT = 0.005f;

// minimum allowable scale factor:

const float MINSCALE = 0.05f;

// scroll wheel button values:

const int SCROLL_WHEEL_UP   = 3;
const int SCROLL_WHEEL_DOWN = 4;

// equivalent mouse movement when we click the scroll wheel:

const float SCROLL_WHEEL_CLICK_FACTOR = 5.f;

// active mouse buttons (or them together):

const int LEFT   = 4;
const int MIDDLE = 2;
const int RIGHT  = 1;

// which projection:

enum Projections
{
	ORTHO,
	PERSP
};

// which button:

enum ButtonVals
{
	RESET,
	QUIT,
	LIGHT
};

enum TextureChoices
{
	NORMAL,
	HIGH
};

// window background color (rgba):

const GLfloat BACKCOLOR[ ] = { 0., 0., 0., 1. };

// line width for the axes:

const GLfloat AXES_WIDTH   = 3.;

// the color numbers:
// this order must match the radio button order, which must match the order of the color names,
// 	which must match the order of the color RGB values

enum Colors
{
	RED,
	YELLOW,
	GREEN,
	CYAN,
	BLUE,
	MAGENTA,
	WHITE,
	BLACK
};

char * ColorNames[ ] =
{
	(char *)"Red",
	(char*)"Yellow",
	(char*)"Green",
	(char*)"Cyan",
	(char*)"Blue",
	(char*)"Magenta",
	(char*)"White",
	(char*)"Black"
};

// the color definitions:
// this order must match the menu order

const GLfloat Colors[ ][3] = 
{
	{ 1., 0., 0. },		// red
	{ 1., 1., 0. },		// yellow
	{ 0., 1., 0. },		// green
	{ 0., 1., 1. },		// cyan
	{ 0., 0., 1. },		// blue
	{ 1., 0., 1. },		// magenta
	{ 1., 1., 1. },		// white
	{ 0., 0., 0. },		// black
};



// fog parameters:

const GLfloat FOGCOLOR[4] = { .0f, .0f, .0f, 1.f };
const GLenum  FOGMODE     = GL_LINEAR;
const GLfloat FOGDENSITY  = 0.30f;
const GLfloat FOGSTART    = 1.5f;
const GLfloat FOGEND      = 4.f;


// what options should we compile-in?
// in general, you don't need to worry about these
// i compile these in to show class examples of things going wrong

//#define DEMO_Z_FIGHTING
//#define DEMO_DEPTH_BUFFER

#define NUM_STARS 100

#define SPEED_INCR_STEP .005
#define SPEED_DECR_STEP .005
#define SPEED_MAX .05
#define SPEED_MIN 0.
#define NUM_STARS 1000

#define RADIUS_SCALE_FACTOR 10000
#define DISTANCE_SCALE_FACTOR 500000
struct Solar_System_Obj {
	char* name;
	float x;
	float y;
	float z;
	float radius; // in miles
	float radius_scaled;
	float off_axis_tilt;
	float rotate_angle = 0;
	float solar_distance;  // distance from sun in miles
	float distance_scaled;
	GLuint texture_name;
};


// non-constant global variables:

int		ActiveButton;			// current button that is down
GLuint	AxesList;				// list to hold the axes
int		AxesOn;					// != 0 means to draw the axes
int		DebugOn;				// != 0 means to print debugging info
int		DepthCueOn;				// != 0 means to use intensity depth cueing
int		DepthBufferOn;			// != 0 means to use the z-buffer
int		DepthFightingOn;		// != 0 means to force the creation of z-fighting
GLuint	SolarSystemList;		// object display list
int		MainWindow;				// window id for main graphics window
float	Scale;					// scaling factor
int		ShadowsOn;				// != 0 means to turn shadows on
int		WhichColor;				// index into Colors[ ]
int		WhichProjection;		// ORTHO or PERSP
int		WhichTexture = HIGH;
int		Xmouse, Ymouse;			// mouse values
float	Xrot, Yrot;				// rotation angles in degrees
unsigned char *sunTexture, *texture, *mercuryTexture, *venusTexture, *marsTexture, *earthTexture, *jupiterTexture, *saturnTexture, *uranusTexture, *neptuneTexture, *spaceshipTexture;
GLuint	SunTex, MercuryTex, VenusTex, EarthTex, MarsTex, Tex3, JupiterTex, SaturnTex, UranusTex, NeptuneTex, SpaceshipTex;
float	travel;
float	PreviousMS = 0.;
double	velocity = 0.;
float	White[3] = { 1., 1., 1. };
float	EngineAmbient = 0.;
float	EngineDiffuse = 0.;
float	EngineSpecular = 0.;
float	SunMinDiffuse = .5;
float	LightSpeedMultiple = 0.;
bool	ForwardDirection = true;
bool	FlipSpaceship = false;
GLfloat RedShift[] = {1.0, 1.0, 1.0};
GLfloat BlueShift[] = { 1.0, 1.0, 1.0 };
int		StarLocations[NUM_STARS][3]; // gets filled in by getRandomStarLocations()

// create sun, planet objects
struct Solar_System_Obj Sun;
struct Solar_System_Obj Mercury;
struct Solar_System_Obj Venus;
struct Solar_System_Obj Earth;
struct Solar_System_Obj Mars;
struct Solar_System_Obj Jupiter;
struct Solar_System_Obj Saturn;
struct Solar_System_Obj Uranus;
struct Solar_System_Obj Neptune;

// function prototypes:
void	Animate( );
void	Display( );
void	DoAxesMenu( int );
void	DoColorMenu( int );
void	DoDebugMenu( int );
void	DoMainMenu( int );
void	DoProjectMenu( int );
void	DoRasterString( float, float, float, char * );
void	DoStrokeString( float, float, float, float, char * );
float	ElapsedSeconds( );
void	InitGraphics( );
void	InitLists( );
void	InitMenus( );
void	Keyboard( unsigned char, int, int );
void	MouseButton( int, int, int, int );
void	MouseMotion( int, int );
void	Reset( );
void	Resize( int, int );
void	Visibility( int );
void	DrawPlanet(struct Solar_System_Obj, float, float);
void	DrawSun(struct Solar_System_Obj);
void	IncreaseVelocity(void);
void	DecreaseVelocity(void);
void	CreateSolarSystem(void);
float*	MulArray3(float factor, float array0[3]);
void	SetMaterial(float, float, float, float);
float*	Array3(float, float, float);
void	SetSunLight(int, float, float, float, float, float, float);
float	getLightSpeedMultiple(int);
void	setVelocityText(void);
void	DrawStars(int);
void	getRandomStarLocations(int);
void	GoLightSpeed(void);
void	ChangeLightShift(int);

void			Axes( float );

unsigned char *	BmpToTexture( char *, int *, int * );
int				ReadInt( FILE * );
short			ReadShort( FILE * );

void			HsvRgb( float[3], float [3] );

void			Cross(float[3], float[3], float[3]);
float			Dot(float [3], float [3]);
float			Unit(float [3], float [3]);


// main program:

int
main( int argc, char *argv[ ] )
{
	// turn on the glut package:
	// (do this before checking argc and argv since it might
	// pull some command line arguments out)

	glutInit( &argc, argv );

	// setup all the graphics stuff:

	InitGraphics( );

	// create the display structures that will not change:

	InitLists( );

	// init all the global variables used by Display( ):
	// this will also post a redisplay

	Reset( );

	// setup all the user interface stuff:

	InitMenus( );

	// draw the scene once and wait for some interaction:
	// (this will never return)

	glutSetWindow( MainWindow );
	glutMainLoop( );

	// glutMainLoop( ) never actually returns
	// the following line is here to make the compiler happy:

	return 0;
}


// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display( ) from here -- let glutPostRedisplay( ) do it

void
Animate( )
{
	// put animation stuff in here -- change some global variables
	// for Display( ) to find:

	/*
	float Time;
	#define MS_PER_CYCLE	20000

	int ms = glutGet(GLUT_ELAPSED_TIME);
	ms %= MS_PER_CYCLE;
	Time = (float)ms / (float)MS_PER_CYCLE;		// [0.,1.)

	//RotateAngle = 360. * Time;
	*/
	float ms2 = glutGet(GLUT_ELAPSED_TIME);
	travel = travel + velocity * (ms2 - PreviousMS);
	PreviousMS = ms2;


	// force a call to Display( ) next time it is convenient:

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// draw the complete scene:

void
Display()
{
	// set which window we want to do the graphics into:

	glutSetWindow(MainWindow);


	// erase the background:

	glDrawBuffer(GL_BACK);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
#ifdef DEMO_DEPTH_BUFFER
	if (DepthBufferOn == 0)
		glDisable(GL_DEPTH_TEST);
#endif


	// specify shading to be smooth:

	glShadeModel(GL_SMOOTH);


	// set the viewport to a square centered in the window:

	GLsizei vx = glutGet(GLUT_WINDOW_WIDTH);
	GLsizei vy = glutGet(GLUT_WINDOW_HEIGHT);
	GLsizei v = vx < vy ? vx : vy;			// minimum dimension
	GLint xl = (vx - v) / 2;
	GLint yb = (vy - v) / 2;
	glViewport(xl, yb, v, v);


	// set the viewing volume:
	// remember that the Z clipping  values are actually
	// given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D( ) IF YOU ARE DOING 2D !

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (WhichProjection == ORTHO)
		glOrtho(-2.f, 2.f, -2.f, 2.f, 0.1f, 1000.f);
	else
		gluPerspective(70.f, 1.f, 0.1f, 1000.f);


	// place the objects into the scene:

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


	// set the eye position, look-at position, and up-vector:

	gluLookAt(-5.f, .3f, .3f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f);


	// rotate the scene:

	glRotatef((GLfloat)Yrot, 0.f, 1.f, 0.f);
	glRotatef((GLfloat)Xrot, 1.f, 0.f, 0.f);


	// uniformly scale the scene:

	if (Scale < MINSCALE)
		Scale = MINSCALE;
	glScalef((GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale);


	// set the fog parameters:

	if (DepthCueOn != 0)
	{
		glFogi(GL_FOG_MODE, FOGMODE);
		glFogfv(GL_FOG_COLOR, FOGCOLOR);
		glFogf(GL_FOG_DENSITY, FOGDENSITY);
		glFogf(GL_FOG_START, FOGSTART);
		glFogf(GL_FOG_END, FOGEND);
		glEnable(GL_FOG);
	}
	else
	{
		glDisable(GL_FOG);
	}


	// possibly draw the axes:

	if (AxesOn != 0)
	{
		glColor3fv(&Colors[WhichColor][0]);
		glCallList(AxesList);
	}

	// since we are using glScalef( ), be sure the normals get unitized:
	glEnable(GL_NORMALIZE);

	
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, MulArray3(.3f, White));
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	//SetMaterial(.44, .5, .56, 100.);

	// DRAW SPACESHIP -------------------------------------------------------------------------
	
	glPushMatrix();
	glRotatef(96, 0., 1., 0.); // align spaceship direction with planets

	// flip spaceship to forward facing if changing from deceleration to acceleration; set as back-facing if the reverse
	if (FlipSpaceship) {
		if (ForwardDirection) {
			glRotatef(-360, 0., 1., 0.);
		}
		else {
			glRotatef(180., 0., 1., 0.);
		}
	}

	// spaceship main body
	gluCylinder(gluNewQuadric(), .25, .25, 1., 30., 30.);

	// engine section between body and vent
	glPushMatrix();
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, SpaceshipTex);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		//SetMaterial(.44, .5, .56, 100.);
		glTranslatef(0., 0., -.05);
		//glColor3f(.44, .5, .56);
		OsuTorus(.14, .25, 30., 30.);
		glDisable(GL_TEXTURE_2D);
	glPopMatrix();
	glEnable(GL_LIGHTING);
	
	// spaceship engine vent
	glPushMatrix();
		glEnable(GL_LIGHT1);
		glPushMatrix();
			glTranslatef(0., 0., -.1);
			// draw lighting - changes based on engine output
			float lightPos[] = { 0., 0., -.5 };
			glLightfv(GL_LIGHT1, GL_POSITION, lightPos);
			float ambient[] = { EngineAmbient, 0, 0, 1. };
			float diffuse[] = { EngineDiffuse, 0, 0, 1. };
			float specular[] = { EngineSpecular, 0., 0., 1. };
			glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
			glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
			glLightfv(GL_LIGHT1, GL_SPECULAR, specular);
			glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 0.);
			glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.);
			glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 1.);
			OsuSphere(.01, 10., 10.);
		glPopMatrix();
		glTranslatef(0., 0., -.25);
		gluCylinder(gluNewQuadric(), .15, 0., .25, 30., 30.);
	glPopMatrix();

	// spaceship nose
	glTranslatef(0., 0., 1.);
	gluCylinder(gluNewQuadric(), .25, 0., 1., 30., 30.);
	glPopMatrix();

	



	// DRAW STARS -----------------------------------------------------------------------------------------

	DrawStars(NUM_STARS);


	// DRAW SUN AND PLANETS -------------------------------------------------------------------------------
	glTranslatef(-travel, 0, 0); // use animation() to move objects
	
	glEnable(GL_TEXTURE_2D);

	// SOL
	glPushMatrix();
	
	glEnable(GL_LIGHTING);
	DrawSun(Sun);
	
	// MERCURY
	DrawPlanet(Mercury, 0, 1);

	// VENUS
	DrawPlanet(Venus, 0, -1.2);

	// EARTH
	DrawPlanet(Earth, 0, -1.5);
	
	// MARS
	DrawPlanet(Mars, .2, -1);
	
	// JUPITER
	DrawPlanet(Jupiter, -.4, 5);

	// SATURN
	DrawPlanet(Saturn, 0, -6);

	// URANUS
	DrawPlanet(Uranus, .3, -3);

	// NEPTUNE
	DrawPlanet(Neptune, 0, 2);
	
	
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	glDisable(GL_NORMALIZE);
	
#ifdef DEMO_Z_FIGHTING
	if( DepthFightingOn != 0 )
	{
		glPushMatrix( );
			glRotatef( 90.f,   0.f, 1.f, 0.f );
			glCallList( SphereList );
		glPopMatrix( );
	}
#endif


	// draw some gratuitous text that just rotates on top of the scene:
	// i commented out the actual text-drawing calls -- put them back in if you have a use for them
	// a good use for thefirst one might be to have your name on the screen
	// a good use for the second one might be to have vertex numbers on the screen alongside each vertex

	glDisable( GL_DEPTH_TEST );
	glColor3f( 0.f, 1.f, 1.f );
	//DoRasterString( 0.f, 1.f, 0.f, (char *)"Text That Moves" );


	// draw some gratuitous text that is fixed on the screen:
	//
	// the projection matrix is reset to define a scene whose
	// world coordinate system goes from 0-100 in each axis
	//
	// this is called "percent units", and is just a convenience
	//
	// the modelview matrix is reset to identity as we don't
	// want to transform these coordinates

	glDisable( GL_DEPTH_TEST );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	gluOrtho2D( 0.f, 100.f,     0.f, 100.f );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );
	glColor3f( 1.f, 1.f, 1.f );

	setVelocityText(); // set initial velocity text

	// swap the double-buffered framebuffers:

	glutSwapBuffers( );

	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !

	glFlush( );
}


void
DoAxesMenu( int id )
{
	AxesOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoColorMenu( int id )
{
	WhichColor = id - RED;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDebugMenu( int id )
{
	DebugOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}

// main menu callback:

void
DoMainMenu( int id )
{
	switch( id )
	{
		case LIGHT:
			GoLightSpeed();
			break;
	
		case RESET:
			Reset( );
			break;

		case QUIT:
			// gracefully close out the graphics:
			// gracefully close the graphics window:
			// gracefully exit the program:
			glutSetWindow( MainWindow );
			glFinish( );
			glutDestroyWindow( MainWindow );
			exit( 0 );
			break;

		default:
			fprintf( stderr, "Don't know what to do with Main Menu ID %d\n", id );
	}

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoProjectMenu( int id )
{
	WhichProjection = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// use glut to display a string of characters using a raster font:

void
DoRasterString( float x, float y, float z, char *s )
{
	glRasterPos3f( (GLfloat)x, (GLfloat)y, (GLfloat)z );

	char c;			// one character to print
	for( ; ( c = *s ) != '\0'; s++ )
	{
		glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, c );
	}
}


// use glut to display a string of characters using a stroke font:

void
DoStrokeString( float x, float y, float z, float ht, char *s )
{
	glPushMatrix( );
		glTranslatef( (GLfloat)x, (GLfloat)y, (GLfloat)z );
		float sf = ht / ( 119.05f + 33.33f );
		glScalef( (GLfloat)sf, (GLfloat)sf, (GLfloat)sf );
		char c;			// one character to print
		for( ; ( c = *s ) != '\0'; s++ )
		{
			glutStrokeCharacter( GLUT_STROKE_ROMAN, c );
		}
	glPopMatrix( );
}


// return the number of seconds since the start of the program:

float
ElapsedSeconds( )
{
	// get # of milliseconds since the start of the program:

	int ms = glutGet( GLUT_ELAPSED_TIME );

	// convert it to seconds:

	return (float)ms / 1000.f;
}


// initialize the glui window:

void
InitMenus( )
{
	glutSetWindow( MainWindow );

	int numColors = sizeof( Colors ) / ( 3*sizeof(int) );
	int colormenu = glutCreateMenu( DoColorMenu );
	for( int i = 0; i < numColors; i++ )
	{
		glutAddMenuEntry( ColorNames[i], i );
	}

	int axesmenu = glutCreateMenu( DoAxesMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int debugmenu = glutCreateMenu( DoDebugMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	
	int mainmenu = glutCreateMenu( DoMainMenu );
	glutAddSubMenu(   "Axes",          axesmenu);
	glutAddSubMenu(   "Axis Colors",   colormenu);

#ifdef DEMO_DEPTH_BUFFER
	glutAddSubMenu(   "Depth Buffer",  depthbuffermenu);
#endif

#ifdef DEMO_Z_FIGHTING
	glutAddSubMenu(   "Depth Fighting",depthfightingmenu);
#endif
	glutAddMenuEntry("Go Lightspeed",  LIGHT);
	glutAddMenuEntry( "Reset",         RESET );
	glutAddSubMenu(   "Debug",         debugmenu);
	glutAddMenuEntry( "Quit",          QUIT );

// attach the pop-up menu to the right mouse button:

	glutAttachMenu( GLUT_RIGHT_BUTTON );
}



// initialize the glut and OpenGL libraries:
//	also setup callback functions

void
InitGraphics( )
{
	// request the display modes:
	// ask for red-green-blue-alpha color, double-buffering, and z-buffering:

	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );

	// set the initial window configuration:

	glutInitWindowPosition( 0, 0 );
	glutInitWindowSize( INIT_WINDOW_SIZE, INIT_WINDOW_SIZE );

	// open the window and set its title:

	MainWindow = glutCreateWindow( WINDOWTITLE );
	glutSetWindowTitle( WINDOWTITLE );

	// set the framebuffer clear values:

	glClearColor( BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3] );

	// setup the callback functions:
	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user resizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// PassiveMotionFunc -- handle the mouse moving with a button up
	// VisibilityFunc -- handle a change in window visibility
	// EntryFunc	-- handle the cursor entering or leaving the window
	// SpecialFunc -- handle special keys on the keyboard
	// SpaceballMotionFunc -- handle spaceball translation
	// SpaceballRotateFunc -- handle spaceball rotation
	// SpaceballButtonFunc -- handle spaceball button hits
	// ButtonBoxFunc -- handle button box hits
	// DialsFunc -- handle dial rotations
	// TabletMotionFunc -- handle digitizing tablet motion
	// TabletButtonFunc -- handle digitizing tablet button hits
	// MenuStateFunc -- declare when a pop-up menu is in use
	// TimerFunc -- trigger something to happen a certain time from now
	// IdleFunc -- what to do when nothing else is going on

	glutSetWindow( MainWindow );
	glutDisplayFunc( Display );
	glutReshapeFunc( Resize );
	glutKeyboardFunc( Keyboard );
	glutMouseFunc( MouseButton );
	glutMotionFunc( MouseMotion );
	glutPassiveMotionFunc(MouseMotion);
	//glutPassiveMotionFunc( NULL );
	glutVisibilityFunc( Visibility );
	glutEntryFunc( NULL );
	glutSpecialFunc( NULL );
	glutSpaceballMotionFunc( NULL );
	glutSpaceballRotateFunc( NULL );
	glutSpaceballButtonFunc( NULL );
	glutButtonBoxFunc( NULL );
	glutDialsFunc( NULL );
	glutTabletMotionFunc( NULL );
	glutTabletButtonFunc( NULL );
	glutMenuStateFunc( NULL );
	glutTimerFunc( -1, NULL, 0 );

	// setup glut to call Animate( ) every time it has
	// 	nothing it needs to respond to (which is most of the time)
	// we don't need to do this for this program, and really should set the argument to NULL
	// but, this sets us up nicely for doing animation

	glutIdleFunc( Animate );

	int	WidthSun, HeightSun, WidthMercury, HeightMercury, WidthVenus, HeightVenus, WidthEarth, HeightEarth, WidthMars, HeightMars, WidthJupiter, HeightJupiter, WidthSaturn, HeightSaturn, WidthUranus, HeightUranus, WidthNeptune, HeightNeptune, WidthShip, HeightShip;

	// read in textures
	spaceshipTexture = BmpToTexture("Solar_system/spaceship2.bmp", &WidthShip, &HeightShip);
	sunTexture = BmpToTexture("Solar_system/2k_sun.bmp", &WidthSun, &HeightSun);
	venusTexture = BmpToTexture("Solar_system/4k_venus_atmosphere.bmp", &WidthVenus, &HeightVenus);
	texture = BmpToTexture("Solar_system/worldtex.bmp", &WidthEarth, &HeightEarth);
	uranusTexture = BmpToTexture("Solar_system/2k_uranus.bmp", &WidthUranus, &HeightUranus);
	neptuneTexture = BmpToTexture("Solar_system/2k_neptune.bmp", &WidthNeptune, &HeightNeptune);
	

	if (WhichTexture == NORMAL) {
		// use 2k res
		mercuryTexture = BmpToTexture("Solar_system/2k_mercury.bmp", &WidthMercury, &HeightMercury);
		marsTexture = BmpToTexture("Solar_system/2k_mars.bmp", &WidthMars, &HeightMars);
		jupiterTexture = BmpToTexture("Solar_system/2k_jupiter.bmp", &WidthJupiter, &HeightJupiter);
		saturnTexture = BmpToTexture("Solar_system/2k_saturn.bmp", &WidthSaturn, &HeightSaturn);
	}
	else {
		// use 8k if possible
		mercuryTexture = BmpToTexture("Solar_system/8k_mercury.bmp", &WidthMercury, &HeightMercury);
		marsTexture = BmpToTexture("Solar_system/8k_mars.bmp", &WidthMars, &HeightMars);
		jupiterTexture = BmpToTexture("Solar_system/8k_jupiter.bmp", &WidthJupiter, &HeightJupiter);
		saturnTexture = BmpToTexture("Solar_system/8k_saturn.bmp", &WidthSaturn, &HeightSaturn);
	}

	int level = 0, ncomps = 3, border = 0;
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// assign binding handles
	glGenTextures(1, &SpaceshipTex);
	glGenTextures(1, &SunTex);
	glGenTextures(1, &MercuryTex);
	glGenTextures(1, &VenusTex);
	glGenTextures(1, &EarthTex);
	glGenTextures(1, &MarsTex); 
	glGenTextures(1, &JupiterTex);
	glGenTextures(1, &SaturnTex);
	glGenTextures(1, &UranusTex);
	glGenTextures(1, &NeptuneTex);
	

	// texture spaceship
	glBindTexture(GL_TEXTURE_2D, SpaceshipTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, level, ncomps, WidthShip, HeightShip, border, GL_RGB, GL_UNSIGNED_BYTE, spaceshipTexture);

	// make Sol texture current and set its parameters
	glBindTexture(GL_TEXTURE_2D, SunTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, level, ncomps, WidthSun, HeightSun, border, GL_RGB, GL_UNSIGNED_BYTE, sunTexture);

	// make Mercury texture current and set its parameters
	glBindTexture(GL_TEXTURE_2D, MercuryTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, level, ncomps, WidthMercury, HeightMercury, border, GL_RGB, GL_UNSIGNED_BYTE, mercuryTexture);
	
	// make Venus texture current and set its parameters
	glBindTexture(GL_TEXTURE_2D, VenusTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, level, ncomps, WidthVenus, HeightVenus, border, GL_RGB, GL_UNSIGNED_BYTE, venusTexture);

	// make Earth texture current and set its parameters
	glBindTexture(GL_TEXTURE_2D, EarthTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, level, ncomps, WidthEarth, HeightEarth, border, GL_RGB, GL_UNSIGNED_BYTE, texture);

	// make Mars texture current and set its parameters
	glBindTexture(GL_TEXTURE_2D, MarsTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, level, ncomps, WidthMars, HeightMars, border, GL_RGB, GL_UNSIGNED_BYTE, marsTexture);

	// make Jupiter texture current and set its parameters
	glBindTexture(GL_TEXTURE_2D, JupiterTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, level, ncomps, WidthJupiter, HeightJupiter, border, GL_RGB, GL_UNSIGNED_BYTE, jupiterTexture);

	// make Saturn texture current and set its parameters
	glBindTexture(GL_TEXTURE_2D, SaturnTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, level, ncomps, WidthSaturn, HeightSaturn, border, GL_RGB, GL_UNSIGNED_BYTE, saturnTexture);

	// make Uranus texture current and set its parameters
	glBindTexture(GL_TEXTURE_2D, UranusTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, level, ncomps, WidthUranus, HeightUranus, border, GL_RGB, GL_UNSIGNED_BYTE, uranusTexture);

	// make Neptune texture current and set its parameters
	glBindTexture(GL_TEXTURE_2D, NeptuneTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, level, ncomps, WidthNeptune, HeightNeptune, border, GL_RGB, GL_UNSIGNED_BYTE, neptuneTexture);

	Sun.solar_distance = -25000000.; // modified for convenience
	Sun.radius = 432690;
	Sun.texture_name = SunTex;

	Mercury.solar_distance = 35000000.;
	Mercury.radius = 1516;
	Mercury.texture_name = MercuryTex;

	Venus.solar_distance = 67000000.;
	Venus.radius = 3760;
	Venus.texture_name = VenusTex;

	Earth.solar_distance = 93000000.;
	Earth.radius = 3959;
	Earth.texture_name = EarthTex;

	Mars.solar_distance = 142000000.;
	Mars.radius = 2106;
	Mars.texture_name = MarsTex;

	Jupiter.solar_distance = 484000000.;
	Jupiter.radius = 43441;
	Jupiter.rotate_angle = 180;
	Jupiter.texture_name = JupiterTex;

	Saturn.solar_distance = 889000000.;
	Saturn.radius = 36184;
	Saturn.texture_name = SaturnTex;

	Uranus.solar_distance = 1790000000.;
	Uranus.radius = 15759;
	Uranus.texture_name = UranusTex;

	Neptune.solar_distance = 2880000000.;
	Neptune.radius = 15299;
	Neptune.rotate_angle = 180;
	Neptune.texture_name = NeptuneTex;

	getRandomStarLocations(NUM_STARS);


	// init the glew package (a window must be open to do this):

#ifdef WIN32
	GLenum err = glewInit( );
	if( err != GLEW_OK )
	{
		fprintf( stderr, "glewInit Error\n" );
	}
	else
		fprintf( stderr, "GLEW initialized OK\n" );
	fprintf( stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

}


// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )

void
InitLists( )
{
	
	glutSetWindow( MainWindow );
	//SolarSystemList = glGenLists(1);
	//glNewList(SolarSystemList, GL_COMPILE);
	
	
	
	
	
	
	//OsuSphere(1, 30, 30);
	glEndList();
	





}


// the keyboard callback:

void
Keyboard( unsigned char c, int x, int y )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Keyboard: '%c' (0x%0x)\n", c, c );

	switch( c )
	{
		
		case 'v':
			printf("velocity: %f\n", velocity);
			break;
	
		case 'w':
		case 'W':
			IncreaseVelocity();
			break;

		case 'S':
		case 's':
			DecreaseVelocity();
			break;

		case 'p':
		case 'P':
			WhichProjection = PERSP;
			break;

		case 'q':
		case 'Q':
		case ESCAPE:
			DoMainMenu( QUIT );	// will not return here
			break;				// happy compiler

		default:
			fprintf( stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c );
	}

	// force a call to Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// called when the mouse button transitions down or up:

void
MouseButton( int button, int state, int x, int y )
{
	int b = 0;			// LEFT, MIDDLE, or RIGHT

	if( DebugOn != 0 )
		fprintf( stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y );

	
	// get the proper button bit mask:

	switch( button )
	{
		case GLUT_LEFT_BUTTON:
			b = LEFT;		break;

		case GLUT_MIDDLE_BUTTON:
			b = MIDDLE;		break;

		case GLUT_RIGHT_BUTTON:
			b = RIGHT;		break;

		case SCROLL_WHEEL_UP:
			Scale += SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
			// keep object from turning inside-out or disappearing:
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;

		case SCROLL_WHEEL_DOWN:
			Scale -= SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
			// keep object from turning inside-out or disappearing:
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;

		default:
			b = 0;
			fprintf( stderr, "Unknown mouse button: %d\n", button );
	}

	// button down sets the bit, up clears the bit:

	if( state == GLUT_DOWN )
	{
		Xmouse = x;
		Ymouse = y;
		ActiveButton |= b;		// set the proper bit
	}
	else
	{
		ActiveButton &= ~b;		// clear the proper bit
	}

	glutSetWindow(MainWindow);
	glutPostRedisplay();

}


// called when the mouse moves while a button is down:

void
MouseMotion( int x, int y )
{
	int dx = x - Xmouse;		// change in mouse coords
	int dy = y - Ymouse;

	if( ( ActiveButton & LEFT ) != 0 )
	{
		Xrot += ( ANGFACT*dy );
		Yrot += ( ANGFACT*dx );
	}

	if( ( ActiveButton & MIDDLE ) != 0 )
	{
		Scale += SCLFACT * (float) ( dx - dy );

		// keep object from turning inside-out or disappearing:

		if( Scale < MINSCALE )
			Scale = MINSCALE;
	}

	Xmouse = x;			// new current position
	Ymouse = y;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// reset the transformations and the colors:
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene

void
Reset( )
{
	ActiveButton = 0;
	AxesOn = 1;
	DebugOn = 0;
	DepthBufferOn = 1;
	DepthFightingOn = 0;
	DepthCueOn = 0;
	Scale  = 1.0;
	ShadowsOn = 0;
	WhichColor = WHITE;
	WhichProjection = PERSP;
	Xrot = Yrot = 0.;
	travel = 0;
	velocity = 0;
	RedShift[1] = 1.;
	RedShift[2] = 1.;
	BlueShift[0] = 1.;
	BlueShift[1] = 1.;
}


// called when user resizes the window:

void
Resize( int width, int height )
{
	// don't really need to do anything since window size is
	// checked each time in Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// handle a change to the window's visibility:

void
Visibility ( int state )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Visibility: %d\n", state );

	if( state == GLUT_VISIBLE )
	{
		glutSetWindow( MainWindow );
		glutPostRedisplay( );
	}
	else
	{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}



///////////////////////////////////////   HANDY UTILITIES:  //////////////////////////


// the stroke characters 'X' 'Y' 'Z' :

static float xx[ ] = { 0.f, 1.f, 0.f, 1.f };

static float xy[ ] = { -.5f, .5f, .5f, -.5f };

static int xorder[ ] = { 1, 2, -3, 4 };

static float yx[ ] = { 0.f, 0.f, -.5f, .5f };

static float yy[ ] = { 0.f, .6f, 1.f, 1.f };

static int yorder[ ] = { 1, 2, 3, -2, 4 };

static float zx[ ] = { 1.f, 0.f, 1.f, 0.f, .25f, .75f };

static float zy[ ] = { .5f, .5f, -.5f, -.5f, 0.f, 0.f };

static int zorder[ ] = { 1, 2, 3, 4, -5, 6 };

// fraction of the length to use as height of the characters:
const float LENFRAC = 0.10f;

// fraction of length to use as start location of the characters:
const float BASEFRAC = 1.10f;

//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)

void
Axes( float length )
{
	glBegin( GL_LINE_STRIP );
		glVertex3f( length, 0., 0. );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., length, 0. );
	glEnd( );
	glBegin( GL_LINE_STRIP );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., 0., length );
	glEnd( );

	float fact = LENFRAC * length;
	float base = BASEFRAC * length;

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 4; i++ )
		{
			int j = xorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( base + fact*xx[j], fact*xy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 5; i++ )
		{
			int j = yorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( fact*yx[j], base + fact*yy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 6; i++ )
		{
			int j = zorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( 0.0, fact*zy[j], base + fact*zx[j] );
		}
	glEnd( );

}

// read a BMP file into a Texture:

#define VERBOSE				false
#define BMP_MAGIC_NUMBER	0x4d42
#ifndef BI_RGB
#define BI_RGB				0
#define BI_RLE8				1
#define BI_RLE4				2
#endif


// bmp file header:
struct bmfh
{
	short bfType;		// BMP_MAGIC_NUMBER = "BM"
	int bfSize;		// size of this file in bytes
	short bfReserved1;
	short bfReserved2;
	int bfOffBytes;		// # bytes to get to the start of the per-pixel data
} FileHeader;

// bmp info header:
struct bmih
{
	int biSize;		// info header size, should be 40
	int biWidth;		// image width
	int biHeight;		// image height
	short biPlanes;		// #color planes, should be 1
	short biBitCount;	// #bits/pixel, should be 1, 4, 8, 16, 24, 32
	int biCompression;	// BI_RGB, BI_RLE4, BI_RLE8
	int biSizeImage;
	int biXPixelsPerMeter;
	int biYPixelsPerMeter;
	int biClrUsed;		// # colors in the palette
	int biClrImportant;
} InfoHeader;



// read a BMP file into a Texture:

unsigned char *
BmpToTexture( char *filename, int *width, int *height )
{
	FILE *fp;
#ifdef _WIN32
        errno_t err = fopen_s( &fp, filename, "rb" );
        if( err != 0 )
        {
		fprintf( stderr, "Cannot open Bmp file '%s'\n", filename );
		return NULL;
        }
#else
		fp = fopen( filename, "rb" );
		if( fp == NULL )
		{
			fprintf( stderr, "Cannot open Bmp file '%s'\n", filename );
			return NULL;
		}
#endif

	FileHeader.bfType = ReadShort( fp );


	// if bfType is not BMP_MAGIC_NUMBER, the file is not a bmp:

	if( VERBOSE ) fprintf( stderr, "FileHeader.bfType = 0x%0x = \"%c%c\"\n",
			FileHeader.bfType, FileHeader.bfType&0xff, (FileHeader.bfType>>8)&0xff );
	if( FileHeader.bfType != BMP_MAGIC_NUMBER )
	{
		fprintf( stderr, "Wrong type of file: 0x%0x\n", FileHeader.bfType );
		fclose( fp );
		return NULL;
	}


	FileHeader.bfSize = ReadInt( fp );
	if( VERBOSE )	fprintf( stderr, "FileHeader.bfSize = %d\n", FileHeader.bfSize );

	FileHeader.bfReserved1 = ReadShort( fp );
	FileHeader.bfReserved2 = ReadShort( fp );

	FileHeader.bfOffBytes = ReadInt( fp );


	InfoHeader.biSize = ReadInt( fp );
	InfoHeader.biWidth = ReadInt( fp );
	InfoHeader.biHeight = ReadInt( fp );

	const int nums = InfoHeader.biWidth;
	const int numt = InfoHeader.biHeight;

	InfoHeader.biPlanes = ReadShort( fp );

	InfoHeader.biBitCount = ReadShort( fp );
	if( VERBOSE )	fprintf( stderr, "InfoHeader.biBitCount = %d\n", InfoHeader.biBitCount );

	InfoHeader.biCompression = ReadInt( fp );
	if( VERBOSE )	fprintf( stderr, "InfoHeader.biCompression = %d\n", InfoHeader.biCompression );

	InfoHeader.biSizeImage = ReadInt( fp );
	if( VERBOSE )	fprintf( stderr, "InfoHeader.biSizeImage = %d\n", InfoHeader.biSizeImage );

	InfoHeader.biXPixelsPerMeter = ReadInt( fp );
	InfoHeader.biYPixelsPerMeter = ReadInt( fp );

	InfoHeader.biClrUsed = ReadInt( fp );
	if( VERBOSE )	fprintf( stderr, "InfoHeader.biClrUsed = %d\n", InfoHeader.biClrUsed );

	InfoHeader.biClrImportant = ReadInt( fp );

	// fprintf( stderr, "Image size found: %d x %d\n", ImageWidth, ImageHeight );

	// pixels will be stored bottom-to-top, left-to-right:
	unsigned char *texture = new unsigned char[ 3 * nums * numt ];
	if( texture == NULL )
	{
		fprintf( stderr, "Cannot allocate the texture array!\n" );
		return NULL;
	}

	// extra padding bytes:

	int requiredRowSizeInBytes = 4 * ( ( InfoHeader.biBitCount*InfoHeader.biWidth + 31 ) / 32 );
	if( VERBOSE )	fprintf( stderr, "requiredRowSizeInBytes = %d\n", requiredRowSizeInBytes );

	int myRowSizeInBytes = ( InfoHeader.biBitCount*InfoHeader.biWidth + 7 ) / 8;
	if( VERBOSE )	fprintf( stderr, "myRowSizeInBytes = %d\n", myRowSizeInBytes );

	int numExtra = requiredRowSizeInBytes - myRowSizeInBytes;
	if( VERBOSE )	fprintf( stderr, "New NumExtra padding = %d\n", numExtra );


	// this function does not support compression:

	if( InfoHeader.biCompression != 0 )
	{
		fprintf( stderr, "Wrong type of image compression: %d\n", InfoHeader.biCompression );
		fclose( fp );
		return NULL;
	}
	
	// we can handle 24 bits of direct color:
	if( InfoHeader.biBitCount == 24 )
	{
		rewind( fp );
		fseek( fp, FileHeader.bfOffBytes, SEEK_SET );
		int t;
		unsigned char *tp;
		for( t = 0, tp = texture; t < numt; t++ )
		{
			for( int s = 0; s < nums; s++, tp += 3 )
			{
				*(tp+2) = fgetc( fp );		// b
				*(tp+1) = fgetc( fp );		// g
				*(tp+0) = fgetc( fp );		// r
			}

			for( int e = 0; e < numExtra; e++ )
			{
				fgetc( fp );
			}
		}
	}

	// we can also handle 8 bits of indirect color:
	if( InfoHeader.biBitCount == 8 && InfoHeader.biClrUsed == 256 )
	{
		struct rgba32
		{
			unsigned char r, g, b, a;
		};
		struct rgba32 *colorTable = new struct rgba32[ InfoHeader.biClrUsed ];

		rewind( fp );
		fseek( fp, sizeof(struct bmfh) + InfoHeader.biSize - 2, SEEK_SET );
		for( int c = 0; c < InfoHeader.biClrUsed; c++ )
		{
			colorTable[c].r = fgetc( fp );
			colorTable[c].g = fgetc( fp );
			colorTable[c].b = fgetc( fp );
			colorTable[c].a = fgetc( fp );
			if( VERBOSE )	fprintf( stderr, "%4d:\t0x%02x\t0x%02x\t0x%02x\t0x%02x\n",
				c, colorTable[c].r, colorTable[c].g, colorTable[c].b, colorTable[c].a );
		}

		rewind( fp );
		fseek( fp, FileHeader.bfOffBytes, SEEK_SET );
		int t;
		unsigned char *tp;
		for( t = 0, tp = texture; t < numt; t++ )
		{
			for( int s = 0; s < nums; s++, tp += 3 )
			{
				int index = fgetc( fp );
				*(tp+0) = colorTable[index].r;	// r
				*(tp+1) = colorTable[index].g;	// g
				*(tp+2) = colorTable[index].b;	// b
			}

			for( int e = 0; e < numExtra; e++ )
			{
				fgetc( fp );
			}
		}

		delete[ ] colorTable;
	}

	fclose( fp );

	*width = nums;
	*height = numt;
	return texture;
}

int
ReadInt( FILE *fp )
{
	const unsigned char b0 = fgetc( fp );
	const unsigned char b1 = fgetc( fp );
	const unsigned char b2 = fgetc( fp );
	const unsigned char b3 = fgetc( fp );
	return ( b3 << 24 )  |  ( b2 << 16 )  |  ( b1 << 8 )  |  b0;
}

short
ReadShort( FILE *fp )
{
	const unsigned char b0 = fgetc( fp );
	const unsigned char b1 = fgetc( fp );
	return ( b1 << 8 )  |  b0;
}


// function to convert HSV to RGB
// 0.  <=  s, v, r, g, b  <=  1.
// 0.  <= h  <=  360.
// when this returns, call:
//		glColor3fv( rgb );

void
HsvRgb( float hsv[3], float rgb[3] )
{
	// guarantee valid input:

	float h = hsv[0] / 60.f;
	while( h >= 6. )	h -= 6.;
	while( h <  0. ) 	h += 6.;

	float s = hsv[1];
	if( s < 0. )
		s = 0.;
	if( s > 1. )
		s = 1.;

	float v = hsv[2];
	if( v < 0. )
		v = 0.;
	if( v > 1. )
		v = 1.;

	// if sat==0, then is a gray:

	if( s == 0.0 )
	{
		rgb[0] = rgb[1] = rgb[2] = v;
		return;
	}

	// get an rgb from the hue itself:
	
	float i = (float)floor( h );
	float f = h - i;
	float p = v * ( 1.f - s );
	float q = v * ( 1.f - s*f );
	float t = v * ( 1.f - ( s * (1.f-f) ) );

	float r=0., g=0., b=0.;			// red, green, blue
	switch( (int) i )
	{
		case 0:
			r = v;	g = t;	b = p;
			break;
	
		case 1:
			r = q;	g = v;	b = p;
			break;
	
		case 2:
			r = p;	g = v;	b = t;
			break;
	
		case 3:
			r = p;	g = q;	b = v;
			break;
	
		case 4:
			r = t;	g = p;	b = v;
			break;
	
		case 5:
			r = v;	g = p;	b = q;
			break;
	}


	rgb[0] = r;
	rgb[1] = g;
	rgb[2] = b;
}

void
Cross(float v1[3], float v2[3], float vout[3])
{
	float tmp[3];
	tmp[0] = v1[1] * v2[2] - v2[1] * v1[2];
	tmp[1] = v2[0] * v1[2] - v1[0] * v2[2];
	tmp[2] = v1[0] * v2[1] - v2[0] * v1[1];
	vout[0] = tmp[0];
	vout[1] = tmp[1];
	vout[2] = tmp[2];
}

float
Dot(float v1[3], float v2[3])
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

float
Unit(float vin[3], float vout[3])
{
	float dist = vin[0] * vin[0] + vin[1] * vin[1] + vin[2] * vin[2];
	if (dist > 0.0)
	{
		dist = sqrtf(dist);
		vout[0] = vin[0] / dist;
		vout[1] = vin[1] / dist;
		vout[2] = vin[2] / dist;
	}
	else
	{
		vout[0] = vin[0];
		vout[1] = vin[1];
		vout[2] = vin[2];
	}
	return dist;
}

void
DrawPlanet(struct Solar_System_Obj planet, float y_dist, float z_dist)
{
	glBindTexture(GL_TEXTURE_2D, planet.texture_name);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	
	planet.distance_scaled = (float)planet.solar_distance / DISTANCE_SCALE_FACTOR;
	planet.radius_scaled = (float)planet.radius / RADIUS_SCALE_FACTOR;
	glPushMatrix();
	glTranslatef(planet.distance_scaled, y_dist, z_dist);
	
	//glTranslatef(0, y_dist, z_dist);
	glRotatef(planet.rotate_angle, 0., 1., 0.);
	OsuSphere(planet.radius_scaled, 30, 30);
	glPopMatrix();

}

void
DrawSun(struct Solar_System_Obj planet)
{
	planet.radius_scaled = (float)planet.radius / RADIUS_SCALE_FACTOR;
	planet.distance_scaled = -2 * planet.radius_scaled; // move sun to 1-sun distance behind spaceship starting point

	// CREATE LIGHT SOURCES - 1 center pt and 4 diameter pt lights will represent sun's size
	
	// SUN CENTER and PERIMETER LIGHTS
	glTranslatef(-50, 0., -1.);
	SetSunLight(GL_LIGHT0, 0., 0., 0., 1., 1., 1.);

	glDisable(GL_LIGHTING);
	glColor3f(1., 0., 0.);
	glTranslatef(planet.distance_scaled, 0., 0.);
	
	glPushMatrix();
	glTranslatef(0, 0., -planet.radius_scaled);
	SetSunLight(GL_LIGHT2, 0., 0., -planet.radius_scaled, 1., 1., 1.);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0., planet.radius_scaled);
	SetSunLight(GL_LIGHT3, 0., 0., planet.radius_scaled, 1., 1., 1.);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, -planet.radius_scaled, 0.);
	SetSunLight(GL_LIGHT4, 0., -planet.radius_scaled, 0., 1., 1., 1.);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, planet.radius_scaled, 0.);
	SetSunLight(GL_LIGHT5, 0., planet.radius_scaled, 0., 1., 1., 1.);
	glPopMatrix();

	glPopMatrix();

	// CREATE TEXTURED SUN
	glBindTexture(GL_TEXTURE_2D, planet.texture_name);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glPushMatrix();
	glTranslatef(planet.distance_scaled, 0., 0.);

	OsuSphere(planet.radius_scaled, 30, 30);
	glPopMatrix();
	glEnable(GL_LIGHTING);

}

void
DrawStars(int num)
{

	glPointSize(1.);
	glEnable(GL_POINT_SMOOTH);
	glDisable(GL_LIGHTING);
	glPushMatrix();
		
		glBegin(GL_POINTS);
			
			for (int i = 0; i < NUM_STARS; i++) {
				if (StarLocations[i][0] >= 0) {
					glColor3f(BlueShift[0], BlueShift[1], BlueShift[2]);
				}
				else {
					glColor3f(RedShift[0], RedShift[1], RedShift[2]);
				}

				glVertex3f(StarLocations[i][0], StarLocations[i][1], StarLocations[i][2]);

			}
			//glVertex3f(100., 100., 1.);
			//glVertex3f(300., 70., 1.);
		glEnd();

	glPopMatrix();
	glEnable(GL_LIGHTING);
	glDisable(GL_POINT_SMOOTH);


}

void
ChangeLightShift(int change)
{
	if (change == 0) {
		// speed is low, lower relativistic shift
		RedShift[1] += .1;
		RedShift[2] += .1;
		BlueShift[0] += .1;
		BlueShift[1] += .1;
	}
	else {
		// speed is high, higher relativistic shift
		RedShift[1] -= .1;
		RedShift[2] -= .1;
		BlueShift[0] -= .1;
		BlueShift[1] -= .1;
	}
}

void
GoLightSpeed(void)
// special speed triggered by keyboard that shows how relatively slow light is, given the distances involved
{
	if (ForwardDirection == false) {
		ForwardDirection = true;
		FlipSpaceship = true; // flip spacecraft around
	}



#define max_to_c_ratio 134
	//velocity = .005;
	velocity = .05 / 134;
	LightSpeedMultiple = 1;
	EngineDiffuse = .2;
	
	// blue and red shift
	RedShift[1] = .5;
	RedShift[2] = .5;
	BlueShift[0] = .5;
	BlueShift[1] = .5;


}

void
DecreaseVelocity(void)
{

	if (ForwardDirection) {
		ForwardDirection = false;
		FlipSpaceship = true; // flip spacecraft around
	}


	EngineAmbient = 0.;
	if (velocity > SPEED_MIN) {
		ChangeLightShift(0);
		velocity -= SPEED_DECR_STEP;
		velocity = max(SPEED_MIN, velocity);
		// lower engine lighting
		EngineDiffuse *= .8;
		EngineSpecular -= .05;
		LightSpeedMultiple = getLightSpeedMultiple(115); // get speed of spaceship as multiple of lightspeed
		setVelocityText();
	}
	if (velocity <= SPEED_MIN) {
		EngineDiffuse = 0.;
		EngineSpecular = 0.;
	}
}

void
IncreaseVelocity(void)
{
	if (ForwardDirection == false) {
		ForwardDirection = true;
		FlipSpaceship = true; // flip spacecraft around
	}
	
	if (velocity < SPEED_MAX) {
		ChangeLightShift(1);
		velocity += SPEED_INCR_STEP;
		velocity = min(SPEED_MAX, velocity);
		//higher engine lighting
		EngineDiffuse += .1;
		EngineSpecular += .05;
		//printf("vel: %f\n", velocity);
		LightSpeedMultiple = getLightSpeedMultiple(115); // get speed of spaceship as multiple of lightspeed
		setVelocityText();
	}
	else {
		EngineAmbient = .2;
	}
		
}

float*
MulArray3(float factor, float array0[3])
{
	static float array[4];

	array[0] = factor * array0[0];
	array[1] = factor * array0[1];
	array[2] = factor * array0[2];
	array[3] = 1.;
	return array;
}

float*
Array3(float a, float b, float c)
{
	static float array[4];

	array[0] = a;
	array[1] = b;
	array[2] = c;
	array[3] = 1.;
	return array;
}

void
SetMaterial(float r, float g, float b, float shininess)
{
	glMaterialfv(GL_BACK, GL_EMISSION, Array3(0., 0., 0.));
	glMaterialfv(GL_BACK, GL_AMBIENT, MulArray3(.4f, White));
	glMaterialfv(GL_BACK, GL_DIFFUSE, MulArray3(1., White));
	glMaterialfv(GL_BACK, GL_SPECULAR, Array3(0., 0., 0.));
	glMaterialf(GL_BACK, GL_SHININESS, 5.f);

	glMaterialfv(GL_FRONT, GL_EMISSION, Array3(0., 0., 0.));
	glMaterialfv(GL_FRONT, GL_AMBIENT, Array3(r, g, b));
	glMaterialfv(GL_FRONT, GL_DIFFUSE, Array3(r, g, b));
	glMaterialfv(GL_FRONT, GL_SPECULAR, MulArray3(.8f, White));
	glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}


void
SetSunLight(int ilight, float x, float y, float z, float r, float g, float b)
{
	GLfloat sun_ambient[] = { 0.1, 0.1, 0.1, 1.0 };
	GLfloat sun_specular[] = { 0.2, 0.2, 0.2, 1.0 };
	glLightfv(ilight, GL_POSITION, Array3(x, y, z));
	glLightfv(ilight, GL_AMBIENT, sun_ambient);
	//glLightfv(ilight, GL_DIFFUSE, diffuse);
	glLightfv(ilight, GL_DIFFUSE, Array3(r, g, b));
	glLightfv(ilight, GL_SPECULAR, sun_specular);
	glLightf(ilight, GL_CONSTANT_ATTENUATION, 1.);
	//glLightf(ilight, GL_LINEAR_ATTENUATION, 0.);  // NO ATTENUATION
	glLightf(ilight, GL_LINEAR_ATTENUATION, .001); // very slight attenuation
	glLightf(ilight, GL_QUADRATIC_ATTENUATION, 0.);
	glEnable(ilight);
}


float
getLightSpeedMultiple(int seconds)
// enter seconds elapsed from journey beginning to Neptune at velocity set at .05
// Returns speed as a multiple of c
{
	int mps_c = 186282; // light speed in miles-per-second
	float baseline_velocity = .05;
	float velocity_ratio = velocity / baseline_velocity;

	float baseline_c_multiple = Neptune.solar_distance / seconds / mps_c;
	
	//printf("baseline speed multiple %f\n", baseline_c_multiple);
	//printf("current speed multiple %f\n", baseline_c_multiple * velocity_ratio);
	return baseline_c_multiple * velocity_ratio;

}

void
setVelocityText(void)
{
	char MsgText[256];
	char VelocityString[16];
	sprintf(VelocityString, "%f", round(LightSpeedMultiple));
	//printf("text should show %f\n", LightSpeedMultiple);
	strcpy(MsgText, "Velocity (lightspeed multiple): ");
	strcat(MsgText, VelocityString);
	DoRasterString(5.f, 5.f, 0.f, MsgText);
}

void
getRandomStarLocations(int num) {
	int x, y, z;
	
	for (int i = 0; i < num; i++) {
		
		x = rand() % 2000 - 1000; // rand between -500 and 500 (-500 is the offset)
		y = rand() % 2000 - 1000; // rand between -100 and 100 
		z = rand() % 2000 - 1000; // rand between -100 and 100
		
		StarLocations[i][0] = x;
		StarLocations[i][1] = y;
		StarLocations[i][2] = z;
	}

}