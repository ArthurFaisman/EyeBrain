/*

the backbone of this code was taken from http://nehe.gamedev.net/data/lessons/lesson.asp?lesson=07
(that's why the project file is called "lesson 7")

The following is the note that was included in that code:

*		This Code Was Created By Jeff Molofee 2000
*		A HUGE Thanks To Fredric Echols For Cleaning Up
*		And Optimizing The Base Code, Making It More Flexible!
*		If You've Found This Code Useful, Please Let Me Know.
*		Visit My Site At nehe.gamedev.net
*/



#define RESEARCH_SUBJECT_TEST_MODE TRUE

#define OUTPUT_FILE_NAME "data\\data.txt"


// the following constants are only used when the conditions are not randomized aka when RESEARCH_SUBJECT_TEST_MODE = FALSE
#define USE_MIRROR true
#define DIFFUSE_ONLY false

#define WOBBLE false
#define AMPLITUDE true // false denotes lower amplitude, true denotes higher amplitude
#define SLANT true // true denotes a slant with the top further away from the viewer, whereas false denotes the opposite (both of angle SLANT_ANGLE)
#define SPECULAR_ONLY true
#define SHININESS false // true is wide, false is narrow
#define TEXTURED false
#define LIGHT_POSITION true
// true is above the horizontal plane, false is below


#define FIND_SD_HEIGHT_FIELD FALSE


#define HIGH_AMPLITUDE 0.105              //  These are the possible standard deviations of the height field.
#define LOW_AMPLITUDE 0.05



// the deviation of the light source from the horizontal in degrees, positive corresponds to vertical
#define LIGHT_POSITION_ANGLE 40.0
// the deviation of the wavy field from vertical in degrees, the sign is determined by the current value of the boolean slant (so SLANT_ANGLE is positive)
#define SLANT_ANGLE 30.0

// the field dimensions are in GL units
#define FIELD_WIDTH 8.5
#define FIELD_HEIGHT 8.5
#define PI 3.14159265358979323846

#define CURVATURE_DIAGNOSTIC_MODE FALSE
#define ROTATE FALSE // model rotates around for debugging purposes
#define ALLOW_SPECULAR_TOGGLE FALSE

#define SUBJECT_TIME_LIMIT 3.5
#define TIMES_TO_TEST_EACH_CONDITION 5

// wobble params
bool wobble;
#define WOBBLE_AMPLITUDE  10 // amplitude of the wobble in GL units
#define WOBBLE_PERIOD 1.7 // period of the wobble in seconds
double startTime;
double getSecondsSinceProgramStart();
void startWobble();

#define MESH_WIDTH   350//((int)(200*(mirror?1:1.5))) // make sure to update the variable below accordingly!!!
#define MAX_MESH_WIDTH 350 //((int)(300*1.5)) // for use in initialization of arrays	
#define MESH_HEIGHT  ((int)(MESH_WIDTH*(((double)FIELD_HEIGHT)/FIELD_WIDTH))) // mesh density auto scales to the chosen field dimensions
#define MAX_MESH_HEIGHT  ((int)(MAX_MESH_WIDTH*(((double)FIELD_HEIGHT)/FIELD_WIDTH)))

//texture params
// note: the texture is composed of quads floating above an ambient surface
// IMPORTANT NOTE: proper texture generation assumes that the surface underlying the mesh is PLANAR
bool textured;
bool **textureExists; // holds whether or not a particular quad should be drawn
GLfloat ****textureCoordinates;
void setTexture();
void drawTexture();
GLfloat meshPlaneNormal[3]; 
#define TEXTURE_DRAW_BORDER_PROPORTION 0.01 // used to optimize texture rendering: only the borders are drawn as actual textures
#define OPTIMIZE_TEXTURE_DRAWING FALSE // dont remember what happens if this is turned on. Probably something horrible. 
#define TEXTURE_POINT_OFFSET 0.0000001 // we want the randomly generated texture points to float above the wavy field. This is how high they float
#define TEXTURE_POINT_PROB 0.13

//screen dimensions
#define MONITOR_SIZE_IN_INCHES 24.0//17.3
#define MONITOR_RESOLUTION_WIDTH 1920.0//1600.0
#define MONITOR_RESOLUTION_HEIGHT 1200.0//900.0

#define INIT_MESH_SIZE max( MESH_WIDTH, MESH_HEIGHT) // the dimensions of the square array used in FFT, UPDATE THE VARIABLE BELOW IF YOU CHANGE THIS
#define MAX_INIT_MESH_SIZE max( MAX_MESH_WIDTH, MAX_MESH_HEIGHT) // for use in array initialization
#define KMIN      5                 //  KMIN cycles per MESH_WIDTH.
#define KMIN2     (KMIN*KMIN)      //  Frequency squared.  i.e.  sqrt(KMAX2) cycles per MESH_WIDTH.
#define KMAX      9.5                //  KMAX cycles per MESH_WIDTH.
#define KMAX2     (KMAX*KMAX)      //  Frequency squared.  i.e.  sqrt(KMAX2) cycles per MESH_WIDTH

#define DEFAULT_FULLSCREEN_CHOICE false
#define PRESENT_FULLSCREEN_CHOICE FALSE
#define SHOW_CURSOR_ON_FULLSCREEN true
#define VIEWPOINT_REAL_DISTANCE_TO_SCREEN 21 //30 // the face is set to be this many inches away from the computer screen with respect to viewpoint
#define VIEWPOINT_OLD_DISTANCE_TO_SCREEN 21 //30 // used to set up the scaling for this experiment as proportional to the previous one
#define OPENGL_UNITS_PER_INCH 1.0 * (((float)VIEWPOINT_OLD_DISTANCE_TO_SCREEN) / VIEWPOINT_REAL_DISTANCE_TO_SCREEN)
#define PIXELS_PER_INCH (sqrt(MONITOR_RESOLUTION_WIDTH*MONITOR_RESOLUTION_WIDTH+MONITOR_RESOLUTION_HEIGHT*MONITOR_RESOLUTION_HEIGHT)/MONITOR_SIZE_IN_INCHES)
#define MODEL_REAL_DEPTH_BEHIND_SCREEN 0.0
#define CURVATURE_THRESHOLD  2.5
#define WIDE_SHININESS  (.09*128) // .1*128
#define TIGHT_SHININESS  (0.4*128)

#define PERSPECTIVE_RIGHT_EYE 1
#define PERSPECTIVE_LEFT_EYE 2
#define PERSPECTIVE_RIGHT_FLAT 3
#define PERSPECTIVE_LEFT_FLAT 4

#define GIVE_FEEDBACK TRUE

#if GIVE_FEEDBACK
bool lastAnswerCorrect;
bool lastAnswerIncorrect;
#endif
bool timeOut;

int currentTestConditionCombinationIndex;

#define INIT_WINDOW_WIDTH 1630
#define INIT_WINDOW_HEIGHT 870

//dots
#define DRAWN_DOT_SCALE_FACTOR (((double)1)/30)
#define DOT_PREIMAGE_SCALE_FACTOR (((double)1)/1.7)
// dot placement constraint constants
#define MIN_DISTANCE_FROM_DOT_TO_EDGE (MESH_HEIGHT/3.5)
#define COLOR_MIN_BLUE FALSE
#define PLACE_DOT_ON_EXTREMA TRUE
#define DOT_PREIMAGE_TIME 0.34
bool showDotPreimage;
double timeAtWhichDotPreimageAppeared;
bool clickAllowed;

int dotMeshCoordinates[2]; // coordinates with respect to the mesh


HDC			hDC=NULL;		// Private GDI Device Context
HGLRC		hRC=NULL;		// Permanent Rendering Context
HWND		hWnd=NULL;		// Holds Our Window Handle
HINSTANCE	hInstance;		// Holds The Instance Of The Application

bool	keys[256];			// Array Used For The Keyboard Routine
bool	active=TRUE;		// Window Active Flag Set To TRUE By Default
bool fullscreen;
bool	light;				// Lighting ON/OFF ( NEW )
bool	lp;					// L Pressed? ( NEW )
bool	fp;					// F Pressed? ( NEW )
bool mirror;
bool amplitude; // false is low amplitude, true is high amplitude
bool slant;
bool specularOnly;
bool shininess; // true is wide specular, false is tight specular
bool diffuseOnly;
bool lightPosition; // true is above the horizontal plane, false is below


GLfloat	xrot;				// X Rotation
GLfloat	zrot;				// Y Rotation
GLfloat xspeed;				// X Rotation Speed
GLfloat zspeed;				// Y Rotation Speed
GLfloat	z=-5.0f;			// Depth Into The Screen
GLsizei viewportWidth;
GLsizei viewportHeight;
GLuint texName;
double viewpointPixelDistanceToScreen;
GLfloat lightPositionTop[] = {0.0, tan((float)LIGHT_POSITION_ANGLE*PI/180), 1.0, 0.0};
GLfloat lightPositionBottom[] = {0.0, -tan((float)LIGHT_POSITION_ANGLE*PI/180), 1.0, 0.0};



GLuint	filter;				// Which Filter To Use

bool hill; // current value of hill: true is hill, false is valley
bool * pickedDots; // false corresponds to subject choosing valley, true is hill
int * pickedTestConditionIndices;

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	// Declaration For WndProc

void drawCylinder(int numSides);
void drawSphere(int roughNumSides);
void drawWavyField();
void drawDot(GLfloat dotScaleFactor);
void drawAllObjects();

void processKeyPress(char key);
void setPerspective();
void bindEnvironment();
void updateTestConditions();
void clearPressedKeys();

void pickNewDotLocation();
void generateNewMesh();
void setModelLocation();
bool evaluateCurvatureDiagnosticCriterion(int i, int j);


fftw_complex * fftComplexField;
GLfloat ***wavyField;
GLfloat ***normField;
GLfloat **radiusField;
GLfloat **principalCurvature1Field;
GLfloat **principalCurvature2Field;
bool dotExists;
int mouseX;
int mouseY;
int numSamplesSoFar;
int currentEnvironmentTextureIndex;


typedef struct materialStruct{
	GLfloat ambient[4];
	GLfloat diffuse[4];
	GLfloat specular[4];
	GLfloat shininess[1];
} materialStruct;

static materialStruct redAmbientMaterial = {
	{1.0, 0.0, 0.0, 1.0},
	{0.0, 0.0, 0.0, 1.0},
	{0.0, 0.0, 0.0, 1.0},
	{100.0}
};

static materialStruct blueAmbientMaterial = {
	{0.0, 0.0, 1.0, 1.0},
	{0.0, 0.0, 0.0, 1.0},
	{0.0, 0.0, 0.0, 1.0},
	{100.0}
};

static materialStruct brownAmbientMaterial = {
	{0.54, 0.269, 0.074, 1.0},
	{0.0, 0.0, 0.0, 0.0},
	{0.0, 0.0, 0.0, 0.0},
	{100.0}
};

static materialStruct redAmbientTranslucentMaterial = {
	{1.0, 0.0, 0.0, 0.85},
	{0.0, 0.0, 0.0, 1.0},
	{0.0, 0.0, 0.0, 1.0},
	{100.0}
};

static materialStruct whiteShinyMaterial = {
	{0.0, 0.0, 0.0, 1.0},
//	{0.0, 0.0, 0.0, 1.0},
	{0.55, 0.55, 0.55, 1.0},
//	{1.0, 1.0, 1.0, 1.0},
	{0.70, 0.70, 0.70, 1.0},
	{.25*128}
};

 materialStruct specularWideMaterial = {

	{0.0, 0.0, 0.0, 1.0},
{0.0, 0.0, 0.0, 1.0},
{1.0, 1.0, 1.0, 1.0},
	{WIDE_SHININESS}
};

static materialStruct phongWideMaterial = {
	{0.0, 0.0, 0.0, 1.0},
	{0.50,0.50, 0.50, 1.0},
	{0.50,0.50, 0.50, 1.0},
	{WIDE_SHININESS}
};

 materialStruct specularTightMaterial = {

	{0.0, 0.0, 0.0, 1.0},
{0.0, 0.0, 0.0, 1.0},
{1.0, 1.0, 1.0, 1.0},
	{TIGHT_SHININESS}
};

static materialStruct phongTightMaterial = {
	{0.0, 0.0, 0.0, 1.0},
	{0.50,0.50, 0.50, 1.0},
	{0.50,0.50, 0.50, 1.0},
	{TIGHT_SHININESS}
};

static materialStruct diffuseMaterial = {
	{0.0, 0.0, 0.0, 1.0},
	{1.0,1.0, 1.0, 1.0},
	{0.0,0.0, 0.0, 1.0},
	{50}
};

static materialStruct whiteAmbientMaterial = {
	{1.0, 1.0, 1.0, 13.0},
	{0.0, 0.0, 0.0, 0.0},
	{0.0, 0.0, 0.0, 0.0},
	{100.0}
};

static materialStruct whiteDiffuseMaterial = {
	{0.0, 0.0, 0.0, 1.0},
	{1.0, 1.0, 1.0, 1.0},
	{0.0, 0.0, 0.0, 1.0},
	{100.0}
};



void setMaterial(materialStruct *materials);



// holds the total number of times that we've seen each test condition with the current subject
// the array is of length 2^NUMBER_OF_TEST_CONDITIONS
// note that this bookkeeping only works for binary test conditions (ie slant/not slant)
// for each combination of conditions, the position at index (sum over all condition indices l of whether that condition is true (either 0 or 1) times 2^l) 
// corresponds to the total number of times that that combination of condition indices has been tested for the current subject
int *testConditionsTried; 

// we make sure that each test condition has been encountered already before we start seeing the same test conditions twice
// this variable denotes the maximum number of times that we allow each test condition to have been seen up until now
int currentBoundOnTestConditionsTried;

// list of usable image filenames
// also, they have a color depth of 32 bits - as hardcoded into bmp.cpp		
#define NUM_TEXTURES 5
GLuint	texture[NUM_TEXTURES];	
#define TEXTURE_SIZE 1024 // sphere mapped textures in opengl must be square with a width of a power of 2
std::string textureFilenames [NUM_TEXTURES] = {
	"textures\\1076035-Grilltexture-FurSlipons.bmp",
	"textures\\Crack_Texture_by_struckdumb.bmp",
	"textures\\stone_wall_texture_A270663.bmp",
//	"textures\\stump-ring-texture.bmp",
//	"textures\\texture1.bmp",
	"textures\\texture-hz6t5.bmp",
	"textures\\textures-large-155.bmp"};
//	"textures\\adornedwithgold-136849_wallpaper.jpg-art-texture-pattern-print-16.bmp"};
	
GLfloat averageTextureColor[NUM_TEXTURES][3];


//timeout params/functions

// minimum time that the screen is coloured a neutral color in between tests
// minimum because it might take longer to actually compute the mesh
#define BLANK_SCREEN_TIME 2.0 
void setBlankScreen();
void drawBlankScreen();
bool screenBlank;
bool firstRefreshAfterScreenBlank;
double screenBlankStartTime; // this denotes the time at which the first screen blank is called
double screenBlankFadeStartTime; // this denotes the time at which the first refresh happens after the screen blank is called - differs from the previous variable by the amount of time it takes to compute the mesh
double displayImageStartTime;


bool computeMeshAfterNextScreenRefresh;
bool computeMeshAtNextScreenRefresh;



bool computingNextMesh; // if true, the next mesh has not yet been computed (dont render the next mesh yet!)


// used for storing the value of the window coordinates of a dot location
void saveScreenshot(int centerX, int centerY, int width, int height, const char* fileName);
GLdouble extremaWin_x, extremaWin_y, extremaWin_z;
int screenshotBoxWidth = 6;
int screenshotBoxHeight = 15;
const char screenshotBaseDir[] = "C:\\Users\\Arthur\\Dropbox\\psychophysics\\EyeBrain\\Data\\screenshots\\";