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

#include <windows.h>		// Header File For Windows
#include <stdio.h>			// Header File For Standard Input/Output
#include <stdlib.h>
#include <gl\gl.h>			// Header File For The OpenGL32 Library
#include <gl\glu.h>			// Header File For The GLu32 Library
#include "bmp.h"		// Header File For The Glaux Library
extern "C"
{
#include "fftn.h"
#include "TGA.h"
#include "TGAHelper.h"
}
#include "curvatures.h"
#include <cmath>
#include <time.h>
#include "dataProcess.h"
#include <string>
#include "lesson7.h"

using namespace std;


AUX_RGBImageRec *LoadBMP(char *Filename)				// Loads A Bitmap Image
{
	FILE *File=NULL;									// File Handle

	if (!Filename)										// Make Sure A Filename Was Given
	{
		return NULL;									// If Not Return NULL
	}

	File=fopen(Filename,"r");							// Check To See If The File Exists

	if (File)											// Does The File Exist?
	{
		fclose(File);									// Close The Handle
		return auxDIBImageLoad(Filename);				// Load The Bitmap And Return A Pointer
	}

	return NULL;										// If Load Failed Return NULL
}

void LoadGLTextures()								// Load Bitmaps And Convert To Textures
{

	AUX_RGBImageRec *TextureImage[NUM_TEXTURES];					// Create Storage Space For The Texture

	memset(TextureImage,0,sizeof(void *)*NUM_TEXTURES);  //memset(TextureImage,0,sizeof(void *)*2);  		         	// Set The Pointer To NULL

	for (int i = 0; i < NUM_TEXTURES; i++){
		TextureImage[i]=LoadBMP((char *)textureFilenames[i].c_str());
	}


	glGenTextures(NUM_TEXTURES, &texture[0]); 

	long tempPixel[3];

	for (int i=0; i<NUM_TEXTURES; i++)
	{
		tempPixel[0] = tempPixel[1] = tempPixel[2] = 0;
		// Create Linear Filtered Texture
		glBindTexture(GL_TEXTURE_2D, texture[i]);		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[i]->sizeX, TextureImage[i]->sizeY,
			0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[i]->data);
		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

		//		double value = -1.0;
		// compute the average color of each texture

		for (int j=0; j<TextureImage[i]->sizeX*TextureImage[i]->sizeY*3; j++){
			//			value = max(TextureImage[i]->data[j], value);
			tempPixel[j%3] += TextureImage[i]->data[j];
		}

		for (int j=0; j < 3; j++){
			//			value = (1.0/256)*((double)tempPixel[j]) / ((double)TextureImage[i]->sizeX*TextureImage[i]->sizeY);
			averageTextureColor[i][j] = (1.0/256)*((double)tempPixel[j]) / ((double)TextureImage[i]->sizeX*TextureImage[i]->sizeY);
		}
	}




	//NOTE: the code below to delete the textures produced errors. So, here's a known memory leak.
	// this can be fixed if necessary, but there is only a constant number of textures loaded in so it does
	// not cause any problems for this application
	/*
	for (int i=0; i<NUM_TEXTURES; i++)
	{
	if (TextureImage[i])						// If Texture Exists
	{
	if (TextureImage[i]->data)			// If Texture Image Exists
	{
	delete TextureImage[i]->data;	// Free The Texture Image Memory
	}
	delete TextureImage[i];		// Free The Image Structure
	}
	}
	*/

}

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)		// Resize And Initialize The GL Window
{
	if (height==0)										// Prevent A Divide By Zero By
	{
		height=1;										// Making Height Equal One
	}

	viewportWidth = width;
	viewportHeight = height;

	glViewport(0,0,viewportWidth,viewportHeight);						// Reset The Current Viewport


}

int InitGL(GLvoid)										// All Setup For OpenGL Goes Here
{

#if USE_RANDOMIZED_MESH
		srand( time(NULL));
#endif

	timeOut = false;
	clickAllowed = false;
	screenBlank = false;
	firstRefreshAfterScreenBlank = true;
	showDotPreimage = false;
	computeMeshAfterNextScreenRefresh = false;
	computeMeshAtNextScreenRefresh = false;
	computingNextMesh = false;

#if GIVE_FEEDBACK
	lastAnswerCorrect = false;
	lastAnswerIncorrect = false;
#endif

	meshHeight = MESH_HEIGHT;
	meshWidth = MESH_WIDTH;

	startTime = getSecondsSinceProgramStart();

	numSamplesSoFar = 0;

	//initialize mesh variables:
	realPart = new double[MAX_INIT_MESH_SIZE*MAX_INIT_MESH_SIZE];
	imPart = new double[MAX_INIT_MESH_SIZE*MAX_INIT_MESH_SIZE];
	radiusField = new GLfloat * [MAX_INIT_MESH_SIZE];
	principalCurvature1Field = new GLfloat * [MAX_INIT_MESH_SIZE];
	principalCurvature2Field = new GLfloat * [MAX_INIT_MESH_SIZE];
	wavyField = new GLfloat ** [MAX_MESH_HEIGHT];
	normField = new GLfloat ** [MAX_MESH_HEIGHT];
	for (int i=0; i< MAX_INIT_MESH_SIZE; i++){
		radiusField[i] = new GLfloat[MAX_INIT_MESH_SIZE];
		principalCurvature1Field[i] = new GLfloat[MAX_INIT_MESH_SIZE];
		principalCurvature2Field[i] = new GLfloat[MAX_INIT_MESH_SIZE];
	}
	for (int i = 0; i < MAX_MESH_HEIGHT; i++){
		wavyField[i] = new GLfloat * [MAX_MESH_WIDTH];
		for (int j=0; j < MAX_MESH_WIDTH; j++){
			wavyField[i][j] = new GLfloat[3];
		}
	}
	for (int i = 0; i < MAX_MESH_HEIGHT; i++){
		normField[i] = new GLfloat * [MAX_MESH_WIDTH];
		for (int j=0; j < MAX_MESH_WIDTH; j++){
			normField[i][j] = new GLfloat[3];
		}
	}
	textureExists = new bool*[MAX_MESH_HEIGHT];
	textureCoordinates = new GLfloat***[MAX_MESH_HEIGHT];
	for (int i = 0; i < MAX_MESH_HEIGHT; i++){
		textureExists[i] = new bool [MAX_MESH_WIDTH];
		textureCoordinates[i] = new GLfloat** [MAX_MESH_WIDTH];
		for (int j=0; j < MAX_MESH_WIDTH; j++){
			textureCoordinates[i][j] = new GLfloat* [4]; // 4 points per quad
			for (int k=0; k < 4; k++){
				textureCoordinates[i][j][k] = new GLfloat [3]; // 3 coordinates per point
			}
		}
	}


#if !CURVATURE_DIAGNOSTIC_MODE
	dotExists = TRUE;
#endif

#if RESEARCH_SUBJECT_TEST_MODE
	// use randomized test conditions

	testConditionsTried = new int[(int)pow(2.0, NUMBER_OF_TEST_CONDITIONS)];
	for (int i=0; i< (int)pow(2.0, NUMBER_OF_TEST_CONDITIONS); i++){
		testConditionsTried[i] = 0;
	}


	pickedDots = new bool[((int)pow(2.0, NUMBER_OF_TEST_CONDITIONS))*TIMES_TO_TEST_EACH_CONDITION];
	//	actualDots = new bool[((int)pow(2.0, NUMBER_OF_TEST_CONDITIONS))*TIMES_TO_TEST_EACH_CONDITION];
	pickedTestConditionIndices = new int[((int)pow(2.0, NUMBER_OF_TEST_CONDITIONS))*TIMES_TO_TEST_EACH_CONDITION];

	currentBoundOnTestConditionsTried = 1;
	updateTestConditions();
#else
	// for testing purposes, sets up the set defaults of conditions
	slant = SLANT;
	wobble = WOBBLE;
	mirror = USE_MIRROR;
	amplitude = AMPLITUDE;
	hill =  (bool)(rand()%2);
	specularOnly = SPECULAR_ONLY;
	shininess = SHININESS;
	diffuseOnly = DIFFUSE_ONLY;
	textured = TEXTURED;
	lightPosition = LIGHT_POSITION;
	generateNewMesh();
	pickNewDotLocation();
#endif





	//if (slant) zrot = (GLfloat) 90.0f;
	//else zrot = (GLfloat) 180.0f;
	//xrot = (GLfloat) 90.0f;
	xrot = (GLfloat) 0.0f;
	zrot = (GLfloat) 0.0f;

	glEnable(GL_TEXTURE_2D);							// Enable Texture Mapping
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations

	// set automatic normalization of openGL normal vectors
	//	if (!USE_LIGHTS) {
	glEnable(GL_NORMALIZE);
	//  }
	LoadGLTextures();


	// init light
	glEnable(GL_LIGHTING);

	GLfloat diffuse0[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat specular0[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat ambient0[] = {1.0, 1.0, 1.0, 1.0};


	glEnable(GL_LIGHT0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLightfv(GL_LIGHT0, GL_POSITION, lightPositionTop);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse0);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient0);


	bindEnvironment();




	return TRUE;										// Initialization Went OK
}

int DrawGLScene(GLvoid)									// Here's Where We Do All The Drawing
{

//	double currentTime = getSecondsSinceProgramStart();

	if (clickAllowed && getSecondsSinceProgramStart() - displayImageStartTime > SUBJECT_TIME_LIMIT && RESEARCH_SUBJECT_TEST_MODE){
		clickAllowed = false;
		computeMeshAfterNextScreenRefresh = true;
		lastAnswerCorrect = lastAnswerIncorrect = false;
		timeOut = true;
	}

	// set the clear color. This will only be used if the screen is blank because otherwise the background color
	// will be reset to black before drawing the model
#if GIVE_FEEDBACK
/*	if(lastAnswerCorrect)
		glClearColor(0.0f, 1.0f, 0.0f, 0.0f);
	else if (lastAnswerIncorrect)
		glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
	else
		glClearColor(0.7f, 0.7f, 0.7f, 0.7f);
		*/
	if (timeOut)
		glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
	else
		glClearColor(0.7f, 0.7f, 0.7f, 0.7f);

#else
	glClearColor(0.7f, 0.7f, 0.7f, 0.7f);
#endif


	// this next part has to do with timing the screen refreshes relative to mesh creation
	// the mesh is created after the screen has been refreshed once - to ensure that it's gray while the mesh is being generated

	if (computeMeshAfterNextScreenRefresh){
		computeMeshAfterNextScreenRefresh = false;
		computeMeshAtNextScreenRefresh = true;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		return TRUE;
	}
	if (computeMeshAtNextScreenRefresh){
		computeMeshAtNextScreenRefresh = false;
#if RESEARCH_SUBJECT_TEST_MODE
		updateTestConditions();
#else
		numSamplesSoFar++;
		bindEnvironment();
		setBlankScreen();
		generateNewMesh();
		pickNewDotLocation();
#endif
	}


	if (screenBlank){

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (getSecondsSinceProgramStart() - screenBlankStartTime > BLANK_SCREEN_TIME && !computingNextMesh){

			screenBlank = false;
			timeOut = false;
			showDotPreimage = true;
			timeAtWhichDotPreimageAppeared = getSecondsSinceProgramStart();
		}

	} else{



		if (showDotPreimage && getSecondsSinceProgramStart() - timeAtWhichDotPreimageAppeared > DOT_PREIMAGE_TIME){
			showDotPreimage = false;
			displayImageStartTime = getSecondsSinceProgramStart();
			clickAllowed = true;
		}

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);	

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear The Screen And The Depth Buffer


		glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
		glLoadIdentity();									// Reset The Projection Matrix


		setPerspective();

		drawAllObjects();



		xrot+=xspeed;
		zrot+=zspeed;

	}

	return TRUE;										// Keep Going
}

GLvoid KillGLWindow(GLvoid)								// Properly Kill The Window
{
	if (fullscreen)										// Are We In Fullscreen Mode?
	{
		ChangeDisplaySettings(NULL,0);					// If So Switch Back To The Desktop
		ShowCursor(TRUE);								// Show Mouse Pointer
	}

	if (hRC)											// Do We Have A Rendering Context?
	{
		if (!wglMakeCurrent(NULL,NULL))					// Are We Able To Release The DC And RC Contexts?
		{
			MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC))						// Are We Able To Delete The RC?
		{
			MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		hRC=NULL;										// Set RC To NULL
	}

	if (hDC && !ReleaseDC(hWnd,hDC))					// Are We Able To Release The DC
	{
		MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hDC=NULL;										// Set DC To NULL
	}

	if (hWnd && !DestroyWindow(hWnd))					// Are We Able To Destroy The Window?
	{
		MessageBox(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hWnd=NULL;										// Set hWnd To NULL
	}

	if (!UnregisterClass("OpenGL",hInstance))			// Are We Able To Unregister Class
	{
		MessageBox(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hInstance=NULL;									// Set hInstance To NULL
	}
}

/*	This Code Creates Our OpenGL Window.  Parameters Are:					*
*	title			- Title To Appear At The Top Of The Window				*
*	width			- Width Of The GL Window Or Fullscreen Mode				*
*	height			- Height Of The GL Window Or Fullscreen Mode			*
*	bits			- Number Of Bits To Use For Color (8/16/24/32)			*
*	fullscreenflag	- Use Fullscreen Mode (TRUE) Or Windowed Mode (FALSE)	*/

BOOL CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag)
{
	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	WNDCLASS	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left=(long)0;			// Set Left Value To 0
	WindowRect.right=(long)width;		// Set Right Value To Requested Width
	WindowRect.top=(long)0;				// Set Top Value To 0
	WindowRect.bottom=(long)height;		// Set Bottom Value To Requested Height

	fullscreen=fullscreenflag;			// Set The Global Fullscreen Flag

	hInstance			= GetModuleHandle(NULL);				// Grab An Instance For Our Window
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc		= (WNDPROC) WndProc;					// WndProc Handles Messages
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= hInstance;							// Set The Instance
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground	= NULL;									// No Background Required For GL
	wc.lpszMenuName		= NULL;									// We Don't Want A Menu
	wc.lpszClassName	= "OpenGL";								// Set The Class Name

	if (!RegisterClass(&wc))									// Attempt To Register The Window Class
	{
		MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;											// Return FALSE
	}

	if (fullscreen)												// Attempt Fullscreen Mode?
	{
		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth	= width;				// Selected Screen Width
		dmScreenSettings.dmPelsHeight	= height;				// Selected Screen Height
		dmScreenSettings.dmBitsPerPel	= bits;					// Selected Bits Per Pixel
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
		{
			// If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
			if (MessageBox(NULL,"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?","NeHe GL",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
			{
				fullscreen=FALSE;		// Windowed Mode Selected.  Fullscreen = FALSE
			}
			else
			{
				// Pop Up A Message Box Letting User Know The Program Is Closing.
				MessageBox(NULL,"Program Will Now Close.","ERROR",MB_OK|MB_ICONSTOP);
				return FALSE;									// Return FALSE
			}
		}
	}

	if (fullscreen)												// Are We Still In Fullscreen Mode?
	{
		dwExStyle=WS_EX_APPWINDOW;								// Window Extended Style
		dwStyle=WS_POPUP;										// Windows Style
		ShowCursor(SHOW_CURSOR_ON_FULLSCREEN);										// Hide Mouse Pointer
	}
	else
	{
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
		dwStyle=WS_OVERLAPPEDWINDOW;							// Windows Style
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

	// Create The Window
	if (!(hWnd=CreateWindowEx(	dwExStyle,							// Extended Style For The Window
		"OpenGL",							// Class Name
		title,								// Window Title
		dwStyle |							// Defined Window Style
		WS_CLIPSIBLINGS |					// Required Window Style
		WS_CLIPCHILDREN,					// Required Window Style
		0, 0,								// Window Position
		WindowRect.right-WindowRect.left,	// Calculate Window Width
		WindowRect.bottom-WindowRect.top,	// Calculate Window Height
		NULL,								// No Parent Window
		NULL,								// No Menu
		hInstance,							// Instance
		NULL)))								// Dont Pass Anything To WM_CREATE
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	static	PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		bits,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};

	if (!(hDC=GetDC(hWnd)))							// Did We Get A Device Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))	// Did Windows Find A Matching Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!SetPixelFormat(hDC,PixelFormat,&pfd))		// Are We Able To Set The Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(hRC=wglCreateContext(hDC)))				// Are We Able To Get A Rendering Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!wglMakeCurrent(hDC,hRC))					// Try To Activate The Rendering Context
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	ShowWindow(hWnd,SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);									// Sets Keyboard Focus To The Window
	ReSizeGLScene(width, height);					// Set Up Our Perspective GL Screen

	if (!InitGL())									// Initialize Our Newly Created GL Window
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Initialization Failed.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	return TRUE;									// Success
}

LRESULT CALLBACK WndProc(	HWND	hWnd,			// Handle For This Window
	UINT	uMsg,			// Message For This Window
	WPARAM	wParam,			// Additional Message Information
	LPARAM	lParam)			// Additional Message Information
{
	switch (uMsg)									// Check For Windows Messages
	{
	case WM_ACTIVATE:							// Watch For Window Activate Message
		{
			if (!HIWORD(wParam))					// Check Minimization State
			{
				active=TRUE;						// Program Is Active
			}
			else
			{
				active=FALSE;						// Program Is No Longer Active
			}

			return 0;								// Return To The Message Loop
		}

	case WM_SYSCOMMAND:							// Intercept System Commands
		{
			switch (wParam)							// Check System Calls
			{
			case SC_SCREENSAVE:					// Screensaver Trying To Start?
			case SC_MONITORPOWER:				// Monitor Trying To Enter Powersave?
				return 0;							// Prevent From Happening
			}
			break;									// Exit
		}

	case WM_CLOSE:								// Did We Receive A Close Message?
		{
			PostQuitMessage(0);						// Send A Quit Message
			return 0;								// Jump Back
		}

	case WM_KEYDOWN:							// Is A Key Being Held Down?
		{
			keys[wParam] = TRUE;					// If So, Mark It As TRUE
			return 0;								// Jump Back
		}

	case WM_KEYUP:								// Has A Key Been Released?
		{
			keys[wParam] = FALSE;					// If So, Mark It As FALSE
			return 0;								// Jump Back
		}

	case WM_SIZE:								// Resize The OpenGL Window
		{
			ReSizeGLScene(LOWORD(lParam),HIWORD(lParam));  // LoWord=Width, HiWord=Height
			return 0;								// Jump Back
		}
	case WM_MOUSEMOVE:
		{
			mouseX = LOWORD(lParam);          
			mouseY = HIWORD(lParam);
			return 0;
		}
	case WM_LBUTTONDOWN:
		{
			mouseX = LOWORD(lParam);          
			mouseY = HIWORD(lParam);
			//			processKeyPress();
			return 0;
		}

	}

	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

int WINAPI WinMain(	HINSTANCE	hInstance,			// Instance
	HINSTANCE	hPrevInstance,		// Previous Instance
	LPSTR		lpCmdLine,			// Command Line Parameters
	int			nCmdShow)			// Window Show State
{
	MSG		msg;									// Windows Message Structure
	BOOL	done=FALSE;								// Bool Variable To Exit Loop

#if PRESENT_FULLSCREEN_CHOICE
	// Ask The User Which Screen Mode They Prefer
	if (MessageBox(NULL,"Would You Like To Run In Fullscreen Mode?", "Start FullScreen?",MB_YESNO|MB_ICONQUESTION)==IDNO)
	{
		fullscreen=FALSE;							// Windowed Mode
	}
#else 
	fullscreen = DEFAULT_FULLSCREEN_CHOICE;
#endif
	// Create Our OpenGL Window
	if (!CreateGLWindow("EyeBrain",INIT_WINDOW_WIDTH,INIT_WINDOW_HEIGHT,32,fullscreen))
	{
		return 0;									// Quit If Window Was Not Created
	}

	while(!done)									// Loop That Runs While done=FALSE
	{
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))	// Is There A Message Waiting?
		{
			if (msg.message==WM_QUIT)				// Have We Received A Quit Message?
			{
				done=TRUE;							// If So done=TRUE
			}
			else									// If Not, Deal With Window Messages
			{
				TranslateMessage(&msg);				// Translate The Message
				DispatchMessage(&msg);				// Dispatch The Message
			}
		}
		else										// If There Are No Messages
		{
			// Draw The Scene.  Watch For ESC Key And Quit Messages From DrawGLScene()
			if ((active && !DrawGLScene()) || keys[VK_ESCAPE])	// Active?  Was There A Quit Received?
			{
				done=TRUE;							// ESC or DrawGLScene Signalled A Quit
			}
			else									// Not Time To Quit, Update Screen
			{
				SwapBuffers(hDC);					// Swap Buffers (Double Buffering)

				// there was some weirdness with multiple calls per key press (maybe from holding down the key? thats not what it looked like though)
				// but since were only interested in one keypress per keystroke, I use the following hacky code to ensure that there is only one call per key press
				if (! keys['D'] && ! keys['d'] && !keys['B'] && !keys['b']
#if ALLOW_SPECULAR_TOGGLE
				&& !keys['q'] && !keys['Q']
#endif
				){

					clearPressedKeys();
				} else {

					if (clickAllowed /*!screenBlank && !showDotPreimage*/){ // makes sure that the keypress is not handled when there is nothing going on
						bool v_pressed = false;
						bool h_pressed = false;
						bool q_pressed = false;
						if (keys['D'])
						{
							v_pressed = true;
						} else if (keys['d']){
							v_pressed = true;
						} else if (keys['B']){
							h_pressed = true;
						} else if (keys['b']){
							h_pressed = true;
						} else if (keys['Q']){
							q_pressed = true;
						} else if (keys['q']){
							q_pressed = true;
						}
						clearPressedKeys();
						if (v_pressed) processKeyPress('d');
						else if (h_pressed) processKeyPress('b');
						else if (q_pressed && ALLOW_SPECULAR_TOGGLE) mirror = !mirror;//specularOnly = !specularOnly; 
					}
				}
			}
		}
	}

	// Shutdown
	KillGLWindow();									// Kill The Window
	return (msg.wParam);							// Exit The Program
}



void drawWavyField(){

	if (textured){

		//		setMaterial(&whiteAmbientMaterial);

#if OPTIMIZE_TEXTURE_DRAWING
		int earlyIndexWidth = TEXTURE_DRAW_BORDER_PROPORTION*meshWidth/2;
		int lateIndexWidth = (1.0-TEXTURE_DRAW_BORDER_PROPORTION/2)*meshWidth;
		int earlyIndexHeight = TEXTURE_DRAW_BORDER_PROPORTION*meshHeight/2;
		int lateIndexHeight = (1.0-TEXTURE_DRAW_BORDER_PROPORTION/2)*meshHeight;

		// draw flat underlying quad to optimize rendering
		glBegin(GL_QUADS);
		glNormal3f(0.0f,0.0f,1.0f);
		glVertex3f(wavyField[earlyIndexHeight][earlyIndexWidth][0], wavyField[earlyIndexHeight][earlyIndexWidth][1], -maxRadius);
		glVertex3f(wavyField[earlyIndexHeight][lateIndexWidth][0], wavyField[earlyIndexHeight][lateIndexWidth][1], -maxRadius);
		glVertex3f(wavyField[lateIndexHeight][lateIndexWidth][0], wavyField[lateIndexHeight][lateIndexWidth][1], -maxRadius);
		glVertex3f(wavyField[lateIndexHeight][earlyIndexWidth][0], wavyField[lateIndexHeight][earlyIndexWidth][1], -maxRadius);
		glEnd();


#else 
		double temp1, temp2;
		glShadeModel(GL_SMOOTH);
		for (int i = 0; i < meshHeight - 1; i++){
			glBegin(GL_TRIANGLE_STRIP);

			for (int j = 0; j < meshWidth*2; j++){
				glNormal3f(normField[i + j%2][j/2][0],normField[i + j%2][j/2][1],normField[i + j%2][j/2][2]);
				glVertex3f(wavyField[i + j%2][j/2][0],wavyField[i + j%2][j/2][1],wavyField[i + j%2][j/2][2]);

				temp1 = normField[i + j%2][j/2][2];
				temp2 = wavyField[i + j%2][j/2][2];

			}

			glEnd();
		}
#endif

		setMaterial(&brownAmbientMaterial);
		drawTexture();
	} else { // not textured


		glShadeModel(GL_SMOOTH);
		for (int i = 0; i < meshHeight - 1; i++){
			glBegin(GL_TRIANGLE_STRIP);

			for (int j = 0; j < meshWidth*2; j++){
#if CURVATURE_DIAGNOSTIC_MODE
				if ( evaluateCurvatureDiagnosticCriterion(i + j%2, j/2)){
					setMaterial(&redAmbientMaterial);
				} else setMaterial(&greyAmbientMaterial);
#endif
				glNormal3f(normField[i + j%2][j/2][0],normField[i + j%2][j/2][1],normField[i + j%2][j/2][2]);
				glVertex3f(wavyField[i + j%2][j/2][0],wavyField[i + j%2][j/2][1],wavyField[i + j%2][j/2][2]);

			}

			glEnd();
		}
	}
}

void drawDot(GLfloat dotScaleFactor){

	glPushMatrix();

	glTranslatef(wavyField[dotMeshCoordinates[0]][dotMeshCoordinates[1]][0],
		wavyField[dotMeshCoordinates[0]][dotMeshCoordinates[1]][1],
		wavyField[dotMeshCoordinates[0]][dotMeshCoordinates[1]][2]);

	glScalef(dotScaleFactor, dotScaleFactor, dotScaleFactor);
	//UNCOMMENT DO DRAW DOT
	//drawSphere(200);

	glPopMatrix();

}

void pickNewDotLocation(){

	int heightBoundMin;
	int heightBoundMax;
	int widthBoundMin;
	int widthBoundMax;

	heightBoundMin = MIN_DISTANCE_FROM_DOT_TO_EDGE;
	heightBoundMax = meshHeight - MIN_DISTANCE_FROM_DOT_TO_EDGE;
	widthBoundMin = MIN_DISTANCE_FROM_DOT_TO_EDGE;
	widthBoundMax = meshWidth - MIN_DISTANCE_FROM_DOT_TO_EDGE;


	bool foundGoodDot = false;


	// note: the hills and valleys are inverted with respect to the mesh when the field is concave
	while (!foundGoodDot){

		dotMeshCoordinates[0] = heightBoundMin + (rand() % (heightBoundMax - heightBoundMin));
		dotMeshCoordinates[1] = widthBoundMin + (rand() % (widthBoundMax - widthBoundMin));

#if PLACE_DOT_ON_EXTREMA
		if (hill){
			if (principalCurvature1Field[dotMeshCoordinates[0]][dotMeshCoordinates[1]] < -CURVATURE_THRESHOLD && 
				principalCurvature2Field[dotMeshCoordinates[0]][dotMeshCoordinates[1]] < -CURVATURE_THRESHOLD){
					// found hill in the case of slant or valley in the case of concave
					foundGoodDot = true;
			}
		} else {
			// (slant && !hill) || (!slant && hill)
			if (principalCurvature1Field[dotMeshCoordinates[0]][dotMeshCoordinates[1]] > CURVATURE_THRESHOLD && 
				principalCurvature2Field[dotMeshCoordinates[0]][dotMeshCoordinates[1]] > CURVATURE_THRESHOLD){
					// found valley in the case of slant or hill in the case of concave
					foundGoodDot = true;
			}
		}
#else
		foundGoodDot = true;
#endif 
	}

}

void processKeyPress(char key){

	if (key != 'd' && key != 'b') return;
	bool pickedDot;
	if ( key == 'b')
		pickedDot = true;
	else 
		pickedDot = false;

#if GIVE_FEEDBACK
	if (pickedDot == hill) {
		lastAnswerCorrect = true;
		lastAnswerIncorrect = false;
	} else {
		lastAnswerCorrect = false;
		lastAnswerIncorrect = true;
	}
#endif

#if RESEARCH_SUBJECT_TEST_MODE
	if ( getSecondsSinceProgramStart() - timeAtWhichDotPreimageAppeared < SUBJECT_TIME_LIMIT){
		numSamplesSoFar++;
		pickedDots[numSamplesSoFar -1] = pickedDot;

		pickedTestConditionIndices[numSamplesSoFar - 1] = currentTestConditionCombinationIndex;
		//	actualDots[numSamplesSoFar - 1] = hill;
		testConditionsTried[currentTestConditionCombinationIndex]++;

		writeRecentDataToFile(OUTPUT_FILE_NAME, pickedDots[numSamplesSoFar -1],  pickedTestConditionIndices[numSamplesSoFar - 1], getSecondsSinceProgramStart() - timeAtWhichDotPreimageAppeared - DOT_PREIMAGE_TIME);

	}

	computeMeshAfterNextScreenRefresh = true;
	clickAllowed = false;



#else // not RESEARCH_SUBJECT_TEST_MODE
	numSamplesSoFar++;
	hill =  (bool)(rand()%2);
	bindEnvironment();

	setBlankScreen();
	generateNewMesh();
	pickNewDotLocation();
#endif
}

void drawSphere(int roughNumSides) {
	int lats = sqrt((double)roughNumSides);
	lats ++;
	int longs = lats;
	int i, j;
	for(i = 0; i <= lats; i++) {
		double lat0 = PI * (-0.5 + (double) (i - 1) / lats);
		double z0  = sin(lat0);
		double zr0 =  cos(lat0);

		double lat1 = PI * (-0.5 + (double) i / lats);
		double z1 = sin(lat1);
		double zr1 = cos(lat1);

		glShadeModel(GL_SMOOTH);
		glBegin(GL_QUAD_STRIP);
		for(j = 0; j <= longs; j++) {
			double lng = 2 * PI * (double) (j - 1) / longs;
			double x = cos(lng);
			double y = sin(lng);

			glNormal3f(x * zr0, y * zr0, z0);
			glVertex3f(x * zr0, y * zr0, z0);
			glNormal3f(x * zr1, y * zr1, z1);
			glVertex3f(x * zr1, y * zr1, z1);
		}
		glEnd();
	}
}

void drawCylinder(int numSides){

	double currentAngle = 0.0;
	GLfloat currentX;
	GLfloat currentY;


	glShadeModel(GL_SMOOTH);
	glBegin(GL_QUAD_STRIP);

	double angleDelta = 2*PI / numSides;

	currentX = cos(currentAngle);
	currentY = sin(currentAngle);
	glNormal3f(currentX, currentY,0.0f);
	glVertex3f(currentX, currentY,0.5f);
	glNormal3f(currentX, currentY,0.0f);
	glVertex3f(currentX, currentY,-0.5f);

	for (int i=0; i < numSides; i++){
		currentAngle += angleDelta;
		currentX = cos(currentAngle);
		currentY = sin(currentAngle);

		glNormal3f(currentX, currentY,0.0f);
		glVertex3f(currentX, currentY,0.5f);
		glNormal3f(currentX, currentY,0.0f);
		glVertex3f(currentX, currentY,-0.5f);

	}
	glEnd();

	glShadeModel(GL_FLAT);
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0.0f, 0.0f,1.0f);
	glVertex3f(0.0f, 0.0f,0.5f);
	currentAngle = 0.0;
	for (int i=0; i < numSides+1; i++){
		currentX = cos(currentAngle);
		currentY = sin(currentAngle);
		glVertex3f(currentX, currentY,0.5f);
		currentAngle += angleDelta;
	}
	glVertex3f(0.0f, 0.0f,0.5f);
	glEnd();

	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0.0f, 0.0f, -1.0f);
	glVertex3f(0.0f, 0.0f, -0.5f);
	currentAngle = 0.0;
	for (int i=0; i < numSides+1; i++){
		currentX = cos(currentAngle);
		currentY = sin(currentAngle);
		glVertex3f(currentX, currentY, -0.5f);
		currentAngle += angleDelta;
	}
	glVertex3f(0.0f, 0.0f, -0.5f);
	glEnd();
}


void generateNewMesh(){

	meshWidth = MESH_WIDTH;
	meshHeight = MESH_HEIGHT;

	int dims[] = {INIT_MESH_SIZE, INIT_MESH_SIZE};


	for (int i = 0; i < INIT_MESH_SIZE*INIT_MESH_SIZE; i++){
		realPart[i] = ((double)rand())/RAND_MAX - 0.5;   //  Between -.5 and .5 with mean 0.
		imPart[i] = 0.0;
	}

	fftn(2, dims, realPart, imPart, -1, 1.0 * INIT_MESH_SIZE);

	for (int i = 0; i < INIT_MESH_SIZE; i++){
		for (int j = 0; j < INIT_MESH_SIZE; j++){
			int i1 = i*i;
			int j1 = j*j;
			int i2 = (i-INIT_MESH_SIZE)*(i-INIT_MESH_SIZE);
			int j2 = (j-INIT_MESH_SIZE)*(j-INIT_MESH_SIZE);
			if  (!((i1 + j1 < KMAX2) & (i1 + j1 > KMIN2)) &
				!((i2 + j1 < KMAX2) & (i2 + j1 > KMIN2)) &
				!((i1 + j2 < KMAX2) & (i1 + j2 > KMIN2)) &
				!((i2 + j2 < KMAX2) & (i2 + j2 > KMIN2)))
				realPart[i*INIT_MESH_SIZE+j] = imPart[i*INIT_MESH_SIZE+j] = 0;
			else imPart[i*INIT_MESH_SIZE+j] = 0;
		}
	}
	fftn(2, dims, realPart, imPart, 1, 1.0 * INIT_MESH_SIZE);


	double sumsquare = 0.0;

	for (int i = 0; i < INIT_MESH_SIZE; i++)
		for (int j = 0; j < INIT_MESH_SIZE; j++)
			sumsquare +=  realPart[i*INIT_MESH_SIZE+j] * realPart[i*INIT_MESH_SIZE+j];

	for (int i = 0; i < INIT_MESH_SIZE; i++)
		for (int j = 0; j < INIT_MESH_SIZE; j++)
			radiusField[i][j] = (GLfloat) realPart[i*INIT_MESH_SIZE+j] / 
			( sqrt(sumsquare)/INIT_MESH_SIZE ) * (amplitude?HIGH_AMPLITUDE:LOW_AMPLITUDE); 

#if FIND_SD_HEIGHT_FIELD

	double runningSum = 0;

	for (int i=0; i < meshHeight; i++){
		for (int j=0; j < meshWidth; j++){
			runningSum += radiusField[i][j]*radiusField[i][j];
		}
	}

	runningSum /= meshHeight*meshWidth - 1;
	runningSum = sqrt(runningSum);

#endif


	setPrincipalCurvatures((float**)radiusField,(float**) principalCurvature1Field,(float**) principalCurvature2Field, INIT_MESH_SIZE, 
		(float)  1.0/INIT_MESH_SIZE, amplitude?HIGH_AMPLITUDE:LOW_AMPLITUDE);



	GLfloat currentHeight;
	GLfloat currentWidth;


	for (int i = 0; i < meshHeight; i++){
		currentHeight = ((GLfloat)FIELD_HEIGHT)*i/(meshHeight-1)-(((float)FIELD_HEIGHT)/2);
		for (int j = 0; j < meshWidth; j++){

			currentWidth = ((GLfloat)FIELD_WIDTH)*j/(meshWidth-1)-(((float)FIELD_WIDTH)/2);

			wavyField[i][j][0] = currentHeight; //x
			wavyField[i][j][1] = currentWidth; //y
#if USE_SINE_SURFACE
			wavyField[i][j][2] = sin(currentHeight*4)*sin(currentWidth*4)*HIGH_AMPLITUDE;
#else
			wavyField[i][j][2] = radiusField[i][j]; //z
#endif

#if USE_1D_FUNCTION
			wavyField[i][j][2] = wavyField[i][0][2];
#endif

		}
	}
	/*
	GLfloat holder1, holder2, holder3;

	for (int i=0; i < meshHeight; i++){
		for (int j=0; j < meshHeight; j++){
			holder1 = wavyField[i][j][0];
			holder2 = wavyField[i][j][1];
			holder3 = wavyField[i][j][2];
		}
	}
	*/

#if PROJECT_POLAR
wavyField = convertCartesianSurfaceToPolar(wavyField, VIEWPOINT_REAL_DISTANCE_TO_SCREEN, meshHeight, meshWidth);
#endif

#if INTEGRATE_SURFACE

#if USE_1D_FUNCTION

	wavyField = getIntegratedSurface_1D(wavyField, meshHeight, meshWidth);
	meshHeight --;

#else

	wavyField = getIntegratedSurface(wavyField, meshHeight, meshWidth);
	meshHeight --; meshWidth --;

#endif
#endif

#if PROJECT_POLAR
wavyField = convertPolarSurfaceToCartesian(wavyField, VIEWPOINT_REAL_DISTANCE_TO_SCREEN, meshHeight, meshWidth);
#endif

	/*
	for (int i=0; i < meshHeight; i++){
		for (int j=0; j < meshHeight; j++){
			holder1 = wavyField[i][j][0];
			holder2 = wavyField[i][j][1];
			holder3 = wavyField[i][j][2];
		}
	}
	*/

	GLfloat vec1[3];
	GLfloat vec2[3];

	// these store the normals for each triangle that borders the current vertex
	GLfloat norm1[3];
	GLfloat norm2[3];
	GLfloat norm3[3];
	GLfloat norm4[3];
	GLfloat temp;
	// note that we only handle internal vertices for now
	for (int i = 1; i< meshHeight - 1; i++){
		for (int j = 1; j < meshWidth-1; j++){

			// find norm for the first of four neighboring faces
			vec1[0] = wavyField[i][j][0] - wavyField[i][j-1][0];
			vec1[1] = wavyField[i][j][1] - wavyField[i][j-1][1];
			vec1[2] = wavyField[i][j][2] - wavyField[i][j-1][2];

			vec2[0] = wavyField[i][j][0] - wavyField[i+1][j][0];
			vec2[1] = wavyField[i][j][1] - wavyField[i+1][j][1];
			vec2[2] = wavyField[i][j][2] - wavyField[i+1][j][2];

			norm1[0] = vec1[1]*vec2[2] - vec1[2]*vec2[1];
			norm1[1] = vec1[2]*vec2[0] - vec1[0]*vec2[2];
			norm1[2] = vec1[0]*vec2[1] - vec1[1]*vec2[0];

			temp = sqrt(norm1[0]*norm1[0]+norm1[1]*norm1[1]+norm1[2]*norm1[2]);
			norm1[0] /= temp;
			norm1[1] /= temp;
			norm1[2] /= temp;


			// find norm for the second of four neighboring faces
			vec1[0] = wavyField[i][j][0] - wavyField[i+1][j][0];
			vec1[1] = wavyField[i][j][1] - wavyField[i+1][j][1];
			vec1[2] = wavyField[i][j][2] - wavyField[i+1][j][2];

			vec2[0] = wavyField[i][j][0] - wavyField[i][j+1][0];
			vec2[1] = wavyField[i][j][1] - wavyField[i][j+1][1];
			vec2[2] = wavyField[i][j][2] - wavyField[i][j+1][2];

			norm2[0] = vec1[1]*vec2[2] - vec1[2]*vec2[1];
			norm2[1] = vec1[2]*vec2[0] - vec1[0]*vec2[2];
			norm2[2] = vec1[0]*vec2[1] - vec1[1]*vec2[0];

			temp = sqrt(norm2[0]*norm2[0]+norm2[1]*norm2[1]+norm2[2]*norm2[2]);
			norm2[0] /= temp;
			norm2[1] /= temp;
			norm2[2] /= temp;


			// find norm for the third of four neighboring faces
			vec1[0] = wavyField[i][j][0] - wavyField[i][j+1][0];
			vec1[1] = wavyField[i][j][1] - wavyField[i][j+1][1];
			vec1[2] = wavyField[i][j][2] - wavyField[i][j+1][2];

			vec2[0] = wavyField[i][j][0] - wavyField[i-1][j][0];
			vec2[1] = wavyField[i][j][1] - wavyField[i-1][j][1];
			vec2[2] = wavyField[i][j][2] - wavyField[i-1][j][2];

			norm3[0] = vec1[1]*vec2[2] - vec1[2]*vec2[1];
			norm3[1] = vec1[2]*vec2[0] - vec1[0]*vec2[2];
			norm3[2] = vec1[0]*vec2[1] - vec1[1]*vec2[0];

			temp = sqrt(norm3[0]*norm3[0]+norm3[1]*norm3[1]+norm3[2]*norm3[2]);
			norm3[0] /= temp;
			norm3[1] /= temp;
			norm3[2] /= temp;


			// find norm for the fourth of four neighboring faces
			vec1[0] = wavyField[i][j][0] - wavyField[i-1][j][0];
			vec1[1] = wavyField[i][j][1] - wavyField[i-1][j][1];
			vec1[2] = wavyField[i][j][2] - wavyField[i-1][j][2];

			vec2[0] = wavyField[i][j][0] - wavyField[i][j-1][0];
			vec2[1] = wavyField[i][j][1] - wavyField[i][j-1][1];
			vec2[2] = wavyField[i][j][2] - wavyField[i][j-1][2];

			norm4[0] = vec1[1]*vec2[2] - vec1[2]*vec2[1];
			norm4[1] = vec1[2]*vec2[0] - vec1[0]*vec2[2];
			norm4[2] = vec1[0]*vec2[1] - vec1[1]*vec2[0];

			temp = sqrt(norm4[0]*norm4[0]+norm4[1]*norm4[1]+norm4[2]*norm4[2]);
			norm4[0] /= temp;
			norm4[1] /= temp;
			norm4[2] /= temp;

			for (int k=0; k < 3; k++){
				normField[i][j][k] = (norm1[k] + norm2[k] + norm3[k] + norm4[k]) / 4;
				temp = (norm1[k] + norm2[k] + norm3[k] + norm4[k]) / 4;
			}

		}
	}

	//mindlessly set normals in border cases to be equal to the nearest fully evaluated normal
	// note that the four corners of the mesh are all properly instantiated using this method
	for (int i=0; i < meshHeight; i++){
		for (int k=0; k < 3; k++){
			normField[i][0][k] = normField[i][1][k];
			normField[i][meshWidth-1][k] = normField[i][meshWidth-2][k];
		}
	}

	for (int j=0; j < meshWidth; j++){
		for (int k=0; k < 3; k++){
			normField[0][j][k] = normField[1][j][k];
			normField[meshHeight-1][j][k] = normField[meshHeight-2][j][k];
		}
	}

	if (textured) setTexture();

}


void setModelLocation(){


	glLoadIdentity();									// Reset The View


	// just translates into the screen (first two coordinates 0 in the case of MOUSE_POSITION_CHANGES_VIEWPOINT == FALSE)
	glTranslatef(0.0f, 0.0f, -OPENGL_UNITS_PER_INCH*(VIEWPOINT_REAL_DISTANCE_TO_SCREEN+MODEL_REAL_DEPTH_BEHIND_SCREEN));

#if ROTATE_VERTICALLY
	glRotatef(90.0, 0.0f, 0.0f, 1.0f);
#endif

	// IF you want to slant, do it directly from model coordinates using height
	//glRotatef(SLANT_ANGLE*(slant?1:-1), 0.0f, 1.0f, 0.0f);

#if USE_SLANT_DEPENDENT_OFFSET
	if (slant){
		glTranslatef(0.3, 0.0f, 0.0f);
	} else {
		glTranslatef(-0.3, 0.0f, 0.0f);
	}
#endif

	if (wobble){
		glRotatef(WOBBLE_AMPLITUDE*sin(2*PI*(getSecondsSinceProgramStart()-startTime)/(WOBBLE_PERIOD)), 1.0f, 0.0f, 0.0f);
	}


#if ROTATE
	glRotatef((getSecondsSinceProgramStart()-startTime)*20, 1.0f, 0.0f,1.0f);
#endif

}


void setPerspective(){


//	double point3;
	double subjectPositionXInPixels = viewportWidth/2;
	double subjectPositionYInPixels = viewportHeight/2;

	double viewportWidthInOpenGLUnits = OPENGL_UNITS_PER_INCH* viewportWidth / PIXELS_PER_INCH;
	double viewportHeightInOpenGLUnits = OPENGL_UNITS_PER_INCH* viewportHeight / PIXELS_PER_INCH;


	double subjectPositionXInOpenGLUnits = OPENGL_UNITS_PER_INCH*  subjectPositionXInPixels / PIXELS_PER_INCH;
	double subjectPositionYInOpenGLUnits = OPENGL_UNITS_PER_INCH*  subjectPositionYInPixels / PIXELS_PER_INCH;


	double relativeSubjectPositionX = subjectPositionXInOpenGLUnits - viewportWidthInOpenGLUnits / 2;
	double relativeSubjectPositionY = -( subjectPositionYInOpenGLUnits - viewportHeightInOpenGLUnits / 2);


	double  nearPlane = 3*OPENGL_UNITS_PER_INCH;
	double farPlane  = 500*OPENGL_UNITS_PER_INCH;


	double   left =     nearPlane * ( -  0.5 * viewportWidthInOpenGLUnits  - relativeSubjectPositionX) / (OPENGL_UNITS_PER_INCH*(VIEWPOINT_REAL_DISTANCE_TO_SCREEN));
	double right =    nearPlane * (  0.5 * viewportWidthInOpenGLUnits  - relativeSubjectPositionX) / (OPENGL_UNITS_PER_INCH*(VIEWPOINT_REAL_DISTANCE_TO_SCREEN));
	double bottom =   nearPlane * ( -  0.5 * viewportHeightInOpenGLUnits - relativeSubjectPositionY) / (OPENGL_UNITS_PER_INCH*(VIEWPOINT_REAL_DISTANCE_TO_SCREEN));
	double top =      nearPlane * (  0.5 * viewportHeightInOpenGLUnits - relativeSubjectPositionY) / (OPENGL_UNITS_PER_INCH*(VIEWPOINT_REAL_DISTANCE_TO_SCREEN));


	glFrustum(left,right,bottom,top,nearPlane,farPlane);


}



// for use in curvature diagnostic mode
bool evaluateCurvatureDiagnosticCriterion(int i, int j){
	return (principalCurvature1Field[i][j] > CURVATURE_THRESHOLD  && principalCurvature2Field[i][j] > CURVATURE_THRESHOLD);
}

double getSecondsSinceProgramStart(){

	return (double) (((double)clock())/CLOCKS_PER_SEC);

}


void bindEnvironment(){

	currentEnvironmentTextureIndex = (int) (rand() % NUM_TEXTURES);
	glBindTexture(GL_TEXTURE_2D, texture[currentEnvironmentTextureIndex]);

}

void updateTestConditions(){

	bool allConditionsTested = true;
	int totalConditionsLeftToTest = 0;

	// figure out how many total conditions there are left to test before we're sure that each condition has been seen currentBoundOnTestConditionsTried times
	bool tempHill, tempSlant, tempWobble, tempMirror, tempAmplitude, tempSpecularOnly, tempShininess, tempDiffuseOnly, tempTextured, tempLightPosition;
	for (int i=0; i < (int)pow(2.0, NUMBER_OF_TEST_CONDITIONS); i++){
		setTestConditions(i, &tempHill, &tempSlant, &tempWobble, &tempMirror, &tempAmplitude, &tempSpecularOnly, &tempShininess, &tempDiffuseOnly, &tempTextured, &tempLightPosition);
		if (testConditionsTried[i] < currentBoundOnTestConditionsTried && validTestConditions(tempHill, tempSlant, tempWobble, tempMirror, tempAmplitude, tempSpecularOnly, tempShininess, tempDiffuseOnly, tempTextured, tempLightPosition)){
			allConditionsTested = false;
			totalConditionsLeftToTest ++;
		}
	}

	// all conditions have been tested currentBoundOnTestConditionsTried times
	if (allConditionsTested) {

		// if we would like to test each condition more times, update this bound and cound the number of conditions we still need to test
		if (currentBoundOnTestConditionsTried < TIMES_TO_TEST_EACH_CONDITION){

			currentBoundOnTestConditionsTried++;
			totalConditionsLeftToTest = 0;

			// figure out again how  many conditions there are left to test
			for (int i=0; i < (int)pow(2.0, NUMBER_OF_TEST_CONDITIONS); i++){
				setTestConditions(i, &tempHill, &tempSlant, &tempWobble, &tempMirror, &tempAmplitude, &tempSpecularOnly, &tempShininess, &tempDiffuseOnly, &tempTextured, &tempLightPosition);
				if (validTestConditions(tempHill, tempSlant, tempWobble, tempMirror, tempAmplitude, tempSpecularOnly, tempShininess, tempDiffuseOnly, tempTextured, tempLightPosition)){
					totalConditionsLeftToTest ++;
				}
			}

		} else {
			// tested all test conditions TIMES_TO_TEST_EACH_CONDITION times


			// commented out because we write data to the file as the program is running instead
			//		writeAllDataToFile(OUTPUT_FILE_NAME, pickedDots, pickedTestConditionIndices, ((int)pow(2.0, NUMBER_OF_TEST_CONDITIONS))*TIMES_TO_TEST_EACH_CONDITION);
			exit(0);
		}
	}


	//note: a "combination" here refers to a particular combination of testing conditions

	// we sample from a uniform across the total legitimate combinations... 

	bool foundNextTestConditions = false;
	int randomizedIndexIntoLegitCombinations = ((int) (rand() % totalConditionsLeftToTest)) + 1; // ranges from 1 to totalConditionsLeftToTest
	int legitCombinationsSeenSoFar = 0;
	int currentCombinationIndex = 0;

	// find an index into the randomizedIndexIntoLegitCombinations-th valid condition
	while (!foundNextTestConditions){
		setTestConditions(currentCombinationIndex, &tempHill, &tempSlant, &tempWobble, &tempMirror, &tempAmplitude, &tempSpecularOnly, &tempShininess, &tempDiffuseOnly, &tempTextured, &tempLightPosition);
		if (testConditionsTried[currentCombinationIndex] < currentBoundOnTestConditionsTried && validTestConditions(tempHill, tempSlant, tempWobble, tempMirror, tempAmplitude, tempSpecularOnly, tempShininess, tempDiffuseOnly, tempTextured, tempLightPosition)){
			legitCombinationsSeenSoFar ++;

			if (legitCombinationsSeenSoFar == randomizedIndexIntoLegitCombinations)
				foundNextTestConditions = true;
			else
				currentCombinationIndex++;
		} else {
			currentCombinationIndex++;
		}
	}



	currentTestConditionCombinationIndex = currentCombinationIndex;

	setTestConditions(currentTestConditionCombinationIndex, &hill, &slant, &wobble, &mirror, &amplitude, &specularOnly, &shininess, &diffuseOnly, &textured, &lightPosition);

	bindEnvironment();
	// NOTE: the order here matters, bindEnvironment() sets the next texture index which is used in setBlankScreen();
	setBlankScreen();

	computingNextMesh = true;
	generateNewMesh();
	pickNewDotLocation();

	computingNextMesh = false;


}

void setBlankScreen(){

	screenBlank = true;
	firstRefreshAfterScreenBlank = true;
	screenBlankStartTime = getSecondsSinceProgramStart();




}

void clearPressedKeys(){

	for (int i=0; i < 256; i++){
		keys[i] = false;
	}

}



void setMaterial(materialStruct *materials){
	glMaterialfv(GL_FRONT, GL_AMBIENT, materials->ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, materials->diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, materials->specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, materials->shininess);
}


void drawTexture(){

	glShadeModel(GL_FLAT);
	glBegin(GL_QUADS);

	for (int i = 0; i < meshHeight; i++){
		for (int j=0; j < meshWidth; j++){
			if (textureExists[i][j]){
				glNormal3f(normField[i][j][0],normField[i][j][1],normField[i][j][2]);
				for (int k=0; k < 4; k++){ //cycle through the texture points
					glVertex3f(textureCoordinates[i][j][k][0],textureCoordinates[i][j][k][1],textureCoordinates[i][j][k][2]);
				}
			}
		}
	}
	glEnd();

}

void setTexture(){

	// NOTE: correct texture generation here assumes the wavy field is built around the x-y plane, with the first component associated with "height" and the second with "width"
	// it also assumes that the z component of each normal is nonzero as we have in the wavy field case

	// these are the offsets from the center of each quad to each of the corners along the respective dimensions
	double widthDelta = 0.5*((double)FIELD_WIDTH)/meshWidth;
	double heightDelta = 0.5*((double)FIELD_HEIGHT)/meshHeight;

	double currentX, currentY;

	GLfloat vec1[3], vec2[3];
	double currRecArea;
	double baseRecArea = 4*widthDelta*heightDelta;

	for (int i = 0; i < meshHeight; i++){
		for (int j=0; j < meshWidth; j++){




			for (int k=0; k < 4; k++){ //cycle through the texture points
				if (k == 0){
					currentX = wavyField[i][j][0] - heightDelta;
					currentY = wavyField[i][j][1] - widthDelta;
				} else if (k == 1){
					currentX = wavyField[i][j][0] + heightDelta;
					currentY = wavyField[i][j][1] - widthDelta;
				} else if (k == 2){
					currentX = wavyField[i][j][0] + heightDelta;
					currentY = wavyField[i][j][1] + widthDelta;
				} else if (k == 3) {
					currentX = wavyField[i][j][0] - heightDelta;
					currentY = wavyField[i][j][1] + widthDelta;
				}

				textureCoordinates[i][j][k][0] = currentX;
				textureCoordinates[i][j][k][1] = currentY;
				// find intersection of the vertical line above (currentX, currentY) and the plane defined by the normal in normField and the point in the plane in wavyField
				textureCoordinates[i][j][k][2] = (wavyField[i][j][2]*normField[i][j][2] + (wavyField[i][j][1]-currentY)*normField[i][j][1] + (wavyField[i][j][0]-currentX)*normField[i][j][0])/normField[i][j][2]
				+ TEXTURE_POINT_OFFSET;
			}

			vec1[0] = textureCoordinates[i][j][0][0] - textureCoordinates[i][j][1][0];
			vec1[1] = textureCoordinates[i][j][0][1] - textureCoordinates[i][j][1][1];
			vec1[2] = textureCoordinates[i][j][0][2] - textureCoordinates[i][j][1][2];

			vec2[0] = textureCoordinates[i][j][1][0] - textureCoordinates[i][j][2][0];
			vec2[1] = textureCoordinates[i][j][1][1] - textureCoordinates[i][j][2][1];
			vec2[2] = textureCoordinates[i][j][1][2] - textureCoordinates[i][j][2][2];

			currRecArea = sqrt( (vec1[0]*vec1[0] + vec1[1]*vec1[1] + vec1[2]*vec1[2]) * (vec2[0]*vec2[0] + vec2[1]*vec2[1] + vec2[2]*vec2[2]) );

			// want the probability that a texture exists at a point to be proportional to the area of the corresponding rectangle
			textureExists[i][j] = (((double)rand())/((double)RAND_MAX) < TEXTURE_POINT_PROB* (currRecArea / baseRecArea) )?true:false;

		}
	}

}

void drawAllObjects(){
	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
		glLoadIdentity();									// Reset The Modelview Matrix

		if (lightPosition)
			glLightfv(GL_LIGHT0, GL_POSITION, lightPositionTop);
		else
			glLightfv(GL_LIGHT0, GL_POSITION, lightPositionBottom);

		if (mirror){
			glEnable(GL_TEXTURE_GEN_S);							
			glEnable(GL_TEXTURE_GEN_T);
			glEnable(GL_TEXTURE_2D);
		}

		setModelLocation();



		if (mirror || textured){
			// to get rid of shading effects when we use a mirror, we simply use a white ambient texture
			setMaterial(&whiteAmbientMaterial);
		} else {
			if (diffuseOnly){
				setMaterial(&diffuseMaterial);
			} else {
				if (specularOnly && shininess){
					setMaterial(&specularWideMaterial);

				} else if (specularOnly && !shininess){
					setMaterial(&specularTightMaterial);

				} else if (!specularOnly && shininess){
					setMaterial(&phongWideMaterial);

				} else if (!specularOnly && !shininess){
					setMaterial(&phongTightMaterial);
				}
			}
		}
		//	setMaterial(&whiteDiffuseMaterial);

		drawWavyField();
		//	drawSphere(1000);

		if (dotExists){
			glDisable(GL_DEPTH_TEST);
			//		if (mirror){
			glDisable(GL_TEXTURE_GEN_S);							
			glDisable(GL_TEXTURE_GEN_T);
			glDisable(GL_TEXTURE_2D);
			//		}
#if COLOR_MIN_BLUE
			if (!hill) setMaterial(&blueAmbientMaterial);
			else
#endif
				setMaterial(&redAmbientMaterial);
			drawDot(DRAWN_DOT_SCALE_FACTOR);

			if (showDotPreimage){

				//				setMaterial(&redAmbientTranslucentMaterial);
				//				glEnable (GL_BLEND);
				//				glBlendFunc (GL_SRC_ALPHA, GL_SRC_ALPHA);

				drawDot(DOT_PREIMAGE_SCALE_FACTOR);

				//				glDisable(GL_BLEND);
			}
			glEnable(GL_DEPTH_TEST);
		}
}

//GLfloat ***getMirrorTransformedSurface(GLfloat*** origSurface, GLfloat distanceToSurface){
//
//	// note that this function is one big memory leak
//	//ALSO note that it assumes that origSurface has square dimensions
//
//	GLfloat ***polarSurface = new GLfloat ** [MAX_MESH_HEIGHT];
//	GLfloat ***outputSurface = new GLfloat ** [MAX_MESH_HEIGHT];
////	GLfloat **boxSums = new GLfloat * [MAX_MESH_HEIGHT];
//	GLfloat **quadVolumes = new GLfloat * [MAX_MESH_HEIGHT];
//	GLfloat **integralsOverY = new GLfloat * [MAX_MESH_HEIGHT];
//	GLfloat **doubleIntegral = new GLfloat * [MAX_MESH_HEIGHT];
////	GLfloat **sumOverX = new GLfloat * [MAX_MESH_HEIGHT];
////	GLfloat **sumOverY = new GLfloat * [MAX_MESH_HEIGHT];
//
//	for (int i = 0; i < MAX_MESH_HEIGHT; i++){
//		polarSurface[i] = new GLfloat * [MAX_MESH_WIDTH];
//		outputSurface[i] = new GLfloat * [MAX_MESH_WIDTH];
////		boxSums[i] = new GLfloat [MAX_MESH_WIDTH];
//		quadVolumes[i] = new GLfloat [MAX_MESH_WIDTH];
//		integralsOverY[i] = new GLfloat [MAX_MESH_WIDTH];
//		doubleIntegral[i] = new GLfloat [MAX_MESH_WIDTH];
////		sumOverX[i] = new GLfloat [MAX_MESH_WIDTH];
////		sumOverY[i] = new GLfloat [MAX_MESH_WIDTH];
//		for (int j=0; j < MAX_MESH_WIDTH; j++){
//			polarSurface[i][j] = new GLfloat[3];
//			outputSurface[i][j] = new GLfloat[3];
//
//		}
//	}
//	
//	GLfloat initialAveragePolarDistance = 0.0;
//	int count = 0;
//
////	GLfloat holder1[2], holder2[2], holder3[2], holder4[2];
//
//	for (int i = 0; i < meshHeight; i++){
//		for (int j = 0; j < meshWidth; j++){
//			/*
//			polarSurface[i][j][0] = (GLfloat) atan( origSurface[i][j][0] / (distanceToSurface - origSurface[i][j][2] ) );
//			polarSurface[i][j][1] = (GLfloat) atan( origSurface[i][j][1] / (distanceToSurface - origSurface[i][j][2] ) );
//			polarSurface[i][j][2] = (GLfloat) sqrt( pow(origSurface[i][j][0],2) + pow(origSurface[i][j][1],2) + pow(distanceToSurface - origSurface[i][j][2],2));
//			*/
//			
//			polarSurface[i][j][0] = origSurface[i][j][0];
//			polarSurface[i][j][1] = origSurface[i][j][1];
//			polarSurface[i][j][2] = origSurface[i][j][2];
//			
//
//			initialAveragePolarDistance += polarSurface[i][j][2];
//			count++;
//		}
//	}
//
//	initialAveragePolarDistance /= count;
//
////	GLfloat prevTheta_x = polarSurface[0][0][0];
////	GLfloat prevTheta_y = polarSurface[0][0][1];
//
//	GLfloat quadVector1[2];// = new GLfloat[2];
//	GLfloat quadVector2[2];// = new GLfloat[2];
//	GLfloat quadArea;
//
//	/*
//	note change of meshHeight/Width!
//	*/
//
//	meshHeight--;
//	meshWidth--;
//	for (int i = 0; i < meshHeight; i++){
//		for (int j = 0; j < meshWidth; j++){
//
//			for (int k = 0; k <= 1; k++){
//				/*
//				quadVector1[k] = polarSurface[i+1][j][k] - polarSurface[i][j][k];
//				quadVector2[k] = polarSurface[i+1][j+1][k] - polarSurface[i][j+1][k];
//				*/
//
//				quadVector1[k] = polarSurface[i+1][j+1][k] - polarSurface[i][j][k];
//				quadVector2[k] = polarSurface[i+1][j][k] - polarSurface[i][j+1][k];
//
//				/*
//				holder1[k] = polarSurface[i][j][k];
//				holder2[k] = polarSurface[i+1][j][k];
//				holder3[k] = polarSurface[i+1][j+1][k];
//				holder4[k] = polarSurface[i][j+1][k];
//				*/
//			}
//
//			quadArea = 0.5*abs( quadVector1[0]*quadVector2[1]  - quadVector2[0] * quadVector1[1]);
////			quadArea = abs((polarSurface[i+1][j+1][0] - polarSurface[i][j][0])*(polarSurface[i+1][j+1][1] - polarSurface[i][j][1]));
//
//			quadVolumes[i][j] = quadArea*polarSurface[i][j][2];
//
////			totalSum += quadArea * polarSurface[i][j][2];
//
//			// note in place editing, this is dependent on the fact that the third coordiante of polarSurface
//			// is useless for computing quadArea
//			/*
//			polarSurface[i][j][2] = totalSum;
//
//			sumOfIntegralVals += totalSum;
//			numValsSeen ++;
//			*/
//		}
//	}
//	
//	GLfloat sum;
//
//	for (int i=0; i < meshHeight; i++){
//		sum = 0;
//		for (int j=0; j < meshWidth; j++){
//			sum += quadVolumes[i][j];
//			integralsOverY[i][j] = sum;
//		}
//	}
//
//	for (int j=0; j < meshWidth; j++){
//		sum = 0;
//		for (int i=0; i < meshHeight; i++){
//			sum += quadVolumes[i][j];
//			doubleIntegral[i][j] = sum;
//		}
//	}
//
//	/*
//	for (int i=0; i < meshHeight; i++){
//		for (int j=0; j < meshWidth; j++){
//			if (i == 0){
//				sumOverX[i][j] = quadVolumes[i][j];
//			} else {
//				sumOverX[i][j] = sumOverX[i-1][j] + quadVolumes[i][j];
//			}
//			if (j == 0){
//				sumOverY[i][j] = quadVolumes[i][j];
//			} else {
//				sumOverY[i][j] = sumOverY[i][j-1] + quadVolumes[i][j];
//			}
//		}
//	}
//
//	//boxSums[0][0] = quadVolumes[0][0];
//
//	for (int i = 0; i < meshHeight; i++){
//		for (int j=0; j < meshWidth; j++){
//			if (i == 0){
//				boxSums[i][j] = sumOverY[i][j];
//			} else if (j == 0){
//				boxSums[i][j] = sumOverX[i][j];
//			} else {
//				boxSums[i][j] = boxSums[i-1][j-1] + sumOverX[i-1][j] + sumOverY[i][j-1] + quadVolumes[i][j];
//			}
//		}
//	}
//	*/
//	/*
//	for (int i = 0; i < meshHeight; i++){
//		for (int j = 0; j < meshWidth; j++){
//			totalSum = 0.0;
//			for (int i1 = 0; i1 <= i; i1++){
//				for (int j1 = 0; j1 <= j; j1++){
//					// totalSum += area of base quadrilateral times height of function at that point
//					totalSum += quadAreas[i1][j1]*polarSurface[i1][j1][2];
//				}
//			}
//			runningSums[i][j] = totalSum;
//		}
//	}
//*/
//
//
//
//	GLfloat totalSum = 0.0;
//	GLfloat sumOfIntegralVals = 0.0;
//	int numValsSeen = 0;
//
//	for (int i = 0; i < meshHeight; i++){
//		for (int j = 0; j < meshWidth; j++){
//			numValsSeen ++;
//			sumOfIntegralVals += doubleIntegral[i][j];
//		}
//	}
//
//	GLfloat offsetValue = sumOfIntegralVals / ((float) numValsSeen);
////	GLfloat offsetValue = polarSurface[meshHeight/2][meshWidth/2][2];
//	
////	GLfloat max = -10000000, min=10000000;
//	for (int i = 0; i < meshHeight; i++){
//		for (int j = 0; j < meshWidth; j++){
//			polarSurface[i][j][2] = (doubleIntegral[i][j] - offsetValue)*COMPRESSION_MULTIPLIER;// + initialAveragePolarDistance;
//			//polarSurface[i][j][2] = 0.0;
////			if (polarSurface[i][j][2] > max) max = polarSurface[i][j][2];
////			if (polarSurface[i][j][2] < min) min = polarSurface[i][j][2];
//		}
//	}
//	
//	for (int i = 0; i < meshHeight; i++){
//		for (int j = 0; j < meshWidth; j++){
//			
//			/*
//			outputSurface[i][j][0] =  tan(polarSurface[i][j][0]) * abs(polarSurface[i][j][2]) / 
//				sqrt( pow( tan( polarSurface[i][j][0] ) , 2 ) + pow( tan( polarSurface[i][j][1] ) , 2 ) + 1 );
//			outputSurface[i][j][1] = tan(polarSurface[i][j][1]) * abs(polarSurface[i][j][2]) / 
//				sqrt( pow( tan( polarSurface[i][j][0] ) , 2 ) + pow( tan( polarSurface[i][j][1] ) , 2 ) + 1 );
//			outputSurface[i][j][2] = distanceToSurface - abs(polarSurface[i][j][2]) / 
//				sqrt( pow( tan( polarSurface[i][j][0] ) , 2 ) + pow( tan( polarSurface[i][j][1] ) , 2 ) + 1 );
//			*/
//			
//			outputSurface[i][j][0] = polarSurface[i][j][0];
//			outputSurface[i][j][1] = polarSurface[i][j][1];
//			outputSurface[i][j][2] = polarSurface[i][j][2];
//			
//		}
//	}
//
//	return outputSurface;
//
//}


GLfloat *** getIntegratedSurface(GLfloat ***origSurface, int dim1, int dim2){
	//origSurface[i][j][k] gives the kth coordinate of the mesh element, s.t. the third coordinate is a function of the first two

	//decrement the dimensions since taking integral
	dim1 --;
	dim2 --;

	GLfloat ***outputSurface = new GLfloat ** [dim1];
	GLfloat **quadVolumes = new GLfloat * [dim1];
	GLfloat **integralsOverY = new GLfloat * [dim1];
	GLfloat **doubleIntegral = new GLfloat * [dim1];

	GLfloat origOffset = 0;
	int numElements = 0;

	for (int i = 0; i < dim1; i++){
		outputSurface[i] = new GLfloat * [dim2];
		quadVolumes[i] = new GLfloat [dim2];
		integralsOverY[i] = new GLfloat [dim2];
		doubleIntegral[i] = new GLfloat [dim2];
		for (int j=0; j < dim2; j++){
			outputSurface[i][j] = new GLfloat[3];
		}
	}

	// calculate the average offset of the input surface, this will be duplicated in the integrated one

	for (int i = 0; i < dim1; i++){
		for (int j=0; j < dim2; j++){
			origOffset += origSurface[i][j][2];
			numElements++;
		}
	}

	origOffset /= numElements;

	GLfloat quadPoint1[2];
	GLfloat quadPoint2[2];
	GLfloat quadPoint3[2];
	GLfloat quadPoint4[2];
	GLfloat currQuadArea;

	for (int i = 0; i < dim1; i++){
		for (int j = 0; j < dim2; j++){
			for (int k=0; k < 2; k++){
				quadPoint1[k] = origSurface[i][j][k];
				quadPoint2[k] = origSurface[i+1][j][k];
				quadPoint3[k] = origSurface[i+1][j+1][k];
				quadPoint4[k] = origSurface[i][j+1][k];
			}
			currQuadArea = abs( quadPoint1[0]*quadPoint2[1] - quadPoint1[1]*quadPoint2[0] + 
								quadPoint2[0]*quadPoint3[1] - quadPoint2[1]*quadPoint3[0] +
								quadPoint3[0]*quadPoint4[1] - quadPoint3[1]*quadPoint4[0] +
								quadPoint4[0]*quadPoint1[1] - quadPoint4[1]*quadPoint1[0] ) / 2.0;

//			GLfloat alternateQuadArea = (quadPoint3[0] - quadPoint1[0])*(quadPoint3[1] - quadPoint1[1]);
//			currQuadArea = alternateQuadArea;

			quadVolumes[i][j] = currQuadArea * origSurface[i][j][2];

		}
	}

	GLfloat sum;

	for (int i=0; i < dim1; i++){
		sum = 0;
		for (int j=0; j < dim2; j++){
			sum += quadVolumes[i][j];
			integralsOverY[i][j] = sum;
		}
	}

	GLfloat resultingOffset = 0;

	for (int j=0; j < dim2; j++){
		sum = 0;
		for (int i=0; i < dim1; i++){
			sum += integralsOverY[i][j];
			doubleIntegral[i][j] = sum;
			resultingOffset += doubleIntegral[i][j];
		}
	}

	resultingOffset /= numElements;

	for (int i = 0; i < dim1; i++){
		for (int j = 0; j < dim2; j++){
			doubleIntegral[i][j] -= resultingOffset;
			doubleIntegral[i][j] *= COMPRESSION_MULTIPLIER;
			doubleIntegral[i][j] += origOffset;

			outputSurface[i][j][0] = origSurface[i][j][0];
			outputSurface[i][j][1] = origSurface[i][j][1];
			outputSurface[i][j][2] = doubleIntegral[i][j];
		}
	}

	return outputSurface;

}

GLfloat *** convertCartesianSurfaceToPolar (GLfloat ***cartesianSurface, GLfloat distanceToSurface, int dim1, int dim2){

	/*
	i,j indexes surface
	cartesianSurface[i][j][0] = x coordinate of mesh
	cartesianSurface[i][j][1] = y coordinate of mesh
	cartesianSurface[i][j][2] = height of mesh
	*/

	/*

	i,j indexes surface
	polarSurface[i][j][0] = theta_x
	polarSurface[i][j][1] = theta_y
	polarSurface[i][j][2] = depth of mesh wrt perspective

	*/

	GLfloat ***polarSurface = new GLfloat ** [dim1];

	for (int i = 0; i < dim1; i++){
		polarSurface[i] = new GLfloat * [dim2];
		for (int j=0; j < dim2; j++){
			polarSurface[i][j] = new GLfloat[3];
		}
	}

	for (int i = 0; i < dim1; i++){
		for (int j=0; j < dim2; j++){

			polarSurface[i][j][0] = (GLfloat) atan( cartesianSurface[i][j][0] / (distanceToSurface - cartesianSurface[i][j][2] ) );
			polarSurface[i][j][1] = (GLfloat) atan( cartesianSurface[i][j][1] / (distanceToSurface - cartesianSurface[i][j][2] ) );
			polarSurface[i][j][2] = (GLfloat) sqrt( pow(cartesianSurface[i][j][0],2) + pow(cartesianSurface[i][j][1],2) + pow(distanceToSurface - cartesianSurface[i][j][2],2));

		}
	}

	return polarSurface;
}

GLfloat *** convertPolarSurfaceToCartesian (GLfloat ***polarSurface, GLfloat distanceToSurface, int dim1, int dim2){

	/*
	i,j indexes surface
	cartesianSurface[i][j][0] = x coordinate of mesh
	cartesianSurface[i][j][1] = y coordinate of mesh
	cartesianSurface[i][j][2] = height of mesh
	*/

	/*

	i,j indexes surface
	polarSurface[i][j][0] = theta_x
	polarSurface[i][j][1] = theta_y
	polarSurface[i][j][2] = depth of mesh wrt perspective

	*/

	GLfloat ***cartesianSurface = new GLfloat ** [dim1];

	for (int i = 0; i < dim1; i++){
		cartesianSurface[i] = new GLfloat * [dim2];
		for (int j=0; j < dim2; j++){
			cartesianSurface[i][j] = new GLfloat[3];
		}
	}

	for (int i = 0; i < dim1; i++){
		for (int j=0; j < dim2; j++){

			cartesianSurface[i][j][0] =  tan(polarSurface[i][j][0]) * abs(polarSurface[i][j][2]) / 
				sqrt( pow( tan( polarSurface[i][j][0] ) , 2 ) + pow( tan( polarSurface[i][j][1] ) , 2 ) + 1 );

			cartesianSurface[i][j][1] = tan(polarSurface[i][j][1]) * abs(polarSurface[i][j][2]) / 
				sqrt( pow( tan( polarSurface[i][j][0] ) , 2 ) + pow( tan( polarSurface[i][j][1] ) , 2 ) + 1 );

			cartesianSurface[i][j][2] = distanceToSurface - abs(polarSurface[i][j][2]) / 
				sqrt( pow( tan( polarSurface[i][j][0] ) , 2 ) + pow( tan( polarSurface[i][j][1] ) , 2 ) + 1 );

		}
	}

	return cartesianSurface;
}


// integrates over origSurface[i][dim2/2]
GLfloat *** getIntegratedSurface_1D(GLfloat ***origSurface, int dim1, int dim2){
	//origSurface[i][j][k] gives the kth coordinate of the mesh element, s.t. the third coordinate is a function of the first two

	//decrement the dimension (for all j1, j2, origSurface[i][j1] == origSurface[i][j2])
	dim1 --;

	GLfloat ***outputSurface = new GLfloat ** [dim1];
	GLfloat *intervalAreas = new GLfloat [dim1];
	GLfloat *integral = new GLfloat [dim1];
	GLfloat *heightVals = new GLfloat[dim1];

	for (int i = 0; i < dim1; i++){
		outputSurface[i] = new GLfloat * [dim2];
		for (int j=0; j < dim2; j++){
			outputSurface[i][j] = new GLfloat[3];
		}
	}

	// calculate the average offset of the input surface, this will be duplicated in the integrated one

//	GLfloat inputMin = origSurface[0][0][2];
//	GLfloat inputMax = origSurface[0][0][2];

//	for (int i = 0; i < dim1; i++){
//		for (int j=0; j < dim2; j++){
////			inputMin = min(inputMin, origSurface[i][j][2]);
////			inputMax = max(inputMax, origSurface[i][j][2]);
//			origOffset += origSurface[i][j][2];
//			numElements++;
//		}
//	}

	GLfloat origOffset = 0;

	for (int i = 0; i < dim1; i++){
		heightVals[i] = origSurface[i][dim2/2][2];
		origOffset += heightVals[i];
	}

	origOffset /= dim1;

	GLfloat intervalLength;

	for (int i = 0; i < dim1; i++){

//		intervalLength = abs(origSurface[i+1][dim2/2][0] - origSurface[i][dim2/2][0]);

		// !! hardcoded interval length !!
		intervalLength = 1.0;

		heightVals[i] -= origOffset;

		intervalAreas[i] = intervalLength * heightVals[i];

	}

	GLfloat sum = 0.0;
	GLfloat resultingOffset = 0;

	for (int i=0; i < dim1; i++){
		sum += intervalAreas[i];
		integral[i] = sum;
		resultingOffset += integral[i];
	}

	resultingOffset /= dim1;

//	GLfloat outputMin = integral[0];
//	GLfloat outputMax = integral[0];

	for (int i = 0; i < dim1; i++){
		
		integral[i] -= resultingOffset;
		integral[i] *= COMPRESSION_MULTIPLIER_1D;
//		integral[i] = sin(i/40.0);
		integral[i] += origOffset;
		for (int j = 0; j < dim2; j++){
			outputSurface[i][j][0] = origSurface[i][j][0];
			outputSurface[i][j][1] = origSurface[i][j][1];
			outputSurface[i][j][2] = integral[i];

//			outputMin = min(inputMin, outputSurface[i][j][2]);
//			outputMax = max(inputMax, outputSurface[i][j][2]);
		}
	}

	return outputSurface;

}