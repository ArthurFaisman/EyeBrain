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

//#include "CImg.h"
#include <atlimage.h>
#include <string.h>
#include <sstream>
#include <fftw3.h>
#include "bmp.h"
#include "curvatures.h"
#include <cmath>
#include <time.h>
#include "dataProcess.h"
#include <string>
#include "lesson7.h"

using namespace std;



void LoadGLTextures()								// Load Bitmaps And Convert To Textures
{
	/*
	BYTE *bmpData[NUM_TEXTURES];
	BYTE *rgbData[NUM_TEXTURES];
	int widths[NUM_TEXTURES];
	int heights[NUM_TEXTURES];
	long sizes[NUM_TEXTURES];
	*/
	BYTE **bmpData = new BYTE*[NUM_TEXTURES];
	BYTE **rgbData = new BYTE*[NUM_TEXTURES];
	int *widths = new int[NUM_TEXTURES];
	int *heights = new int[NUM_TEXTURES];
	long *sizes = new long[NUM_TEXTURES];

	//memset(bmpData,0,sizeof(void *)*NUM_TEXTURES);  //memset(TextureImage,0,sizeof(void *)*2);  		         	// Set The Pointer To NULL
	//memset(rgbData,0,sizeof(void *)*NUM_TEXTURES);


	for (int i = 0; i < NUM_TEXTURES; i++){
		// TextureImage[i]=LoadBMP((char *)textureFilenames[i].c_str());
//		BYTE *tmp = LoadBMP(&(widths[i]), &(heights[i]),&(sizes[i]),(char *)textureFilenames[i].c_str());
		bmpData[i]=LoadBMP(&(widths[i]), &(heights[i]),&(sizes[i]),(char *)textureFilenames[i].c_str());
		rgbData[i] = ConvertBMPToRGBBuffer ( bmpData[i], widths[i], heights[i] );
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
		glTexImage2D(GL_TEXTURE_2D, 0, 3, widths[i], heights[i], 
			0, GL_RGB, GL_UNSIGNED_BYTE, rgbData[i]);
		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

		//		double value = -1.0;
		// compute the average color of each texture

		for (int j=0; j<widths[i]*heights[i]*3; j++){
			//			value = max(TextureImage[i]->data[j], value);
			tempPixel[j%3] += rgbData[i][j];
		}

		for (int j=0; j < 3; j++){
			//			value = (1.0/256)*((double)tempPixel[j]) / ((double)TextureImage[i]->sizeX*TextureImage[i]->sizeY);
			averageTextureColor[i][j] = (1.0/256)*((double)tempPixel[j]) / ((double)widths[i]*heights[i]);
		}
	}
	
	for (int i=0; i < NUM_TEXTURES; i++){
		delete [] rgbData[i];
		delete [] bmpData[i];
	}
	
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
//	BYTE* tmp;

	srand( time(NULL));

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

	startTime = getSecondsSinceProgramStart();

	numSamplesSoFar = 0;

//	tmp = new BYTE[1000000000];
//	delete [] tmp;

	//initialize mesh variables:
	//realPart = new double[MAX_INIT_MESH_SIZE*MAX_INIT_MESH_SIZE];
	//imPart = new double[MAX_INIT_MESH_SIZE*MAX_INIT_MESH_SIZE];
	fftComplexField = (fftw_complex *) fftw_alloc_complex(MAX_INIT_MESH_SIZE*MAX_INIT_MESH_SIZE);
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
//	tmp = new BYTE[1000000000];
//	delete [] tmp;
	for (int i = 0; i < MAX_MESH_HEIGHT; i++){
		normField[i] = new GLfloat * [MAX_MESH_WIDTH];
		for (int j=0; j < MAX_MESH_WIDTH; j++){
			normField[i][j] = new GLfloat[3];
		}
	}
//	tmp = new BYTE[100000000];
//	delete [] tmp;
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

//	tmp = new BYTE[1000000];
//	delete [] tmp;

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
	
	int screenshotCenterX = (int) extremaWin_x;
	int screenshotCenterY = (int) extremaWin_y;
	std::stringstream ss;
	ss << screenshotBaseDir;
	ss << "ss.bmp";
	const string tmp = ss.str();

	saveScreenshot(screenshotCenterX, screenshotCenterY, screenshotBoxWidth,screenshotBoxHeight, tmp.c_str());
	
	//return TRUE;										// Keep Going
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
		int earlyIndexWidth = TEXTURE_DRAW_BORDER_PROPORTION*MESH_WIDTH/2;
		int lateIndexWidth = (1.0-TEXTURE_DRAW_BORDER_PROPORTION/2)*MESH_WIDTH;
		int earlyIndexHeight = TEXTURE_DRAW_BORDER_PROPORTION*MESH_HEIGHT/2;
		int lateIndexHeight = (1.0-TEXTURE_DRAW_BORDER_PROPORTION/2)*MESH_HEIGHT;

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
		for (int i = 0; i < MESH_HEIGHT - 1; i++){
			glBegin(GL_TRIANGLE_STRIP);

			for (int j = 0; j < MESH_WIDTH*2; j++){
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
		for (int i = 0; i < MESH_HEIGHT - 1; i++){
			glBegin(GL_TRIANGLE_STRIP);

			for (int j = 0; j < MESH_WIDTH*2; j++){
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
	drawSphere(200);

	// get on-screen coordinates

	GLdouble model_view[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, model_view);

	GLdouble projection[16];
	glGetDoublev(GL_PROJECTION_MATRIX, projection);

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	gluProject(0.0,0.0,0.0,
				model_view, projection, viewport,
				&extremaWin_x, &extremaWin_y, &extremaWin_z);



	glPopMatrix();

}

void pickNewDotLocation(){

	int heightBoundMin;
	int heightBoundMax;
	int widthBoundMin;
	int widthBoundMax;

	heightBoundMin = MIN_DISTANCE_FROM_DOT_TO_EDGE;
	heightBoundMax = MESH_HEIGHT - MIN_DISTANCE_FROM_DOT_TO_EDGE;
	widthBoundMin = MIN_DISTANCE_FROM_DOT_TO_EDGE;
	widthBoundMax = MESH_WIDTH - MIN_DISTANCE_FROM_DOT_TO_EDGE;


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


	int dims[2] = {INIT_MESH_SIZE, INIT_MESH_SIZE};

	for (int i = 0; i < INIT_MESH_SIZE*INIT_MESH_SIZE; i++){
		fftComplexField[i][0] = ((double)rand())/RAND_MAX - 0.5;
		fftComplexField[i][1] = 0.0;
//		realPart[i] = ((double)rand())/RAND_MAX - 0.5;   //  Between -.5 and .5 with mean 0.
//		imPart[i] = 0.0;
	}

	fftw_plan fftPlan = fftw_plan_dft_2d(INIT_MESH_SIZE, INIT_MESH_SIZE, fftComplexField, fftComplexField, FFTW_FORWARD, FFTW_ESTIMATE);
	fftw_execute(fftPlan);
//	fftn(2, dims, realPart, imPart, -1, 1.0 * INIT_MESH_SIZE);

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
				fftComplexField[i*INIT_MESH_SIZE+j][0] = fftComplexField[i*INIT_MESH_SIZE+j][1] = 0;
			else fftComplexField[i*INIT_MESH_SIZE+j][1] = 0;
		}
	}
	
	fftw_destroy_plan(fftPlan);
	//destroy_plan(fftPlan);
	fftPlan = fftw_plan_dft_2d(INIT_MESH_SIZE, INIT_MESH_SIZE, fftComplexField, fftComplexField, FFTW_BACKWARD, FFTW_ESTIMATE);
	fftw_execute(fftPlan);
//	fftn(2, dims, realPart, imPart, 1, 1.0 * INIT_MESH_SIZE);


	double sumsquare = 0.0;

	for (int i = 0; i < INIT_MESH_SIZE; i++)
		for (int j = 0; j < INIT_MESH_SIZE; j++)
			sumsquare +=  fftComplexField[i*INIT_MESH_SIZE+j][0] * fftComplexField[i*INIT_MESH_SIZE+j][0];
//			sumsquare +=  realPart[i*INIT_MESH_SIZE+j] * realPart[i*INIT_MESH_SIZE+j];

	for (int i = 0; i < INIT_MESH_SIZE; i++)
		for (int j = 0; j < INIT_MESH_SIZE; j++)
			radiusField[i][j] = (GLfloat) fftComplexField[i*INIT_MESH_SIZE+j][0] / 
			( sqrt(sumsquare)/INIT_MESH_SIZE ) * (amplitude?HIGH_AMPLITUDE:LOW_AMPLITUDE); 
//			radiusField[i][j] = (GLfloat) realPart[i*INIT_MESH_SIZE+j] / 
//			( sqrt(sumsquare)/INIT_MESH_SIZE ) * (amplitude?HIGH_AMPLITUDE:LOW_AMPLITUDE); 

	fftw_destroy_plan(fftPlan);

#if FIND_SD_HEIGHT_FIELD

	double runningSum = 0;

	for (int i=0; i < MESH_HEIGHT; i++){
		for (int j=0; j < MESH_WIDTH; j++){
			runningSum += radiusField[i][j]*radiusField[i][j];
		}
	}

	runningSum /= MESH_HEIGHT*MESH_WIDTH - 1;
	runningSum = sqrt(runningSum);

#endif


	setPrincipalCurvatures((float**)radiusField,(float**) principalCurvature1Field,(float**) principalCurvature2Field, INIT_MESH_SIZE, 
		(float)  1.0/INIT_MESH_SIZE, amplitude?HIGH_AMPLITUDE:LOW_AMPLITUDE);



	GLfloat currentHeight;
	GLfloat currentWidth;


	for (int i = 0; i < MESH_HEIGHT; i++){
		currentHeight = ((GLfloat)FIELD_HEIGHT)*i/(MESH_HEIGHT-1)-(((float)FIELD_HEIGHT)/2);
		for (int j = 0; j < MESH_WIDTH; j++){
			currentWidth = ((GLfloat)FIELD_WIDTH)*j/(MESH_WIDTH-1)-(((float)FIELD_WIDTH)/2);


			wavyField[i][j][0] = currentHeight; //x
			wavyField[i][j][1] = currentWidth; //y
			wavyField[i][j][2] = radiusField[i][j]; //z

		}
	}






	GLfloat vec1[3];
	GLfloat vec2[3];

	// these store the normals for each triangle that borders the current vertex
	GLfloat norm1[3];
	GLfloat norm2[3];
	GLfloat norm3[3];
	GLfloat norm4[3];
	GLfloat temp;
	// note that we only handle internal vertices for now
	for (int i = 1; i< MESH_HEIGHT - 1; i++){
		for (int j = 1; j < MESH_WIDTH-1; j++){

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
	for (int i=0; i < MESH_HEIGHT; i++){
		for (int k=0; k < 3; k++){
			normField[i][0][k] = normField[i][1][k];
			normField[i][MESH_WIDTH-1][k] = normField[i][MESH_WIDTH-2][k];
		}
	}

	for (int j=0; j < MESH_WIDTH; j++){
		for (int k=0; k < 3; k++){
			normField[0][j][k] = normField[1][j][k];
			normField[MESH_HEIGHT-1][j][k] = normField[MESH_HEIGHT-2][j][k];
		}
	}

	if (textured) setTexture();

}


void setModelLocation(){


	glLoadIdentity();									// Reset The View



	// just translates into the screen (first two coordinates 0 in the case of MOUSE_POSITION_CHANGES_VIEWPOINT == FALSE)
	glTranslatef(0.0f, 0.0f, -OPENGL_UNITS_PER_INCH*(VIEWPOINT_REAL_DISTANCE_TO_SCREEN+MODEL_REAL_DEPTH_BEHIND_SCREEN));

	glRotatef(90.0, 0.0f, 0.0f, 1.0f);

	glRotatef(SLANT_ANGLE*(slant?1:-1), 0.0f, 1.0f, 0.0f);

	if (slant){
		glTranslatef(0.3, 0.0f, 0.0f);
	} else {
		glTranslatef(-0.3, 0.0f, 0.0f);
	}

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

	for (int i = 0; i < MESH_HEIGHT; i++){
		for (int j=0; j < MESH_WIDTH; j++){
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
	double widthDelta = 0.5*((double)FIELD_WIDTH)/MESH_WIDTH;
	double heightDelta = 0.5*((double)FIELD_HEIGHT)/MESH_HEIGHT;

	double currentX, currentY;

	GLfloat vec1[3], vec2[3];
	double currRecArea;
	double baseRecArea = 4*widthDelta*heightDelta;

	for (int i = 0; i < MESH_HEIGHT; i++){
		for (int j=0; j < MESH_WIDTH; j++){




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
/*
			if (showDotPreimage){

				//				setMaterial(&redAmbientTranslucentMaterial);
				//				glEnable (GL_BLEND);
				//				glBlendFunc (GL_SRC_ALPHA, GL_SRC_ALPHA);s

				drawDot(DOT_PREIMAGE_SCALE_FACTOR);

				//				glDisable(GL_BLEND);
			}
	*/
			glEnable(GL_DEPTH_TEST);
		}
}

void saveScreenshot(int centerX, int centerY, int width, int height, const char* fileName){

	int xStart = centerX - width/2;
	int yStart = centerY - height/2;

	//int dataSize = width*height*3;
	//GLubyte *pixels = (GLubyte*) malloc(dataSize*sizeof(GLubyte));//= new GLubyte [dataSize];
	/*
	int emptyArraySize = 138;
	char* emptyArray = new char[emptyArraySize];
	
	for (int i=0; i < emptyArraySize; i++){
		emptyArray[i] = '3';
	}
	*/
	//glReadPixels(centerX - width/2, centerY - height/2, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	/*
	unsigned char TGAheader[12]={0,0,2,0,0,0,0,0,width,height,8,0};

	FILE *fScreenshot = fopen(fileName,"wb");

	fwrite(TGAheader, sizeof(unsigned char), 12, fScreenshot);
	fwrite(pixels, sizeof(GLubyte), dataSize, fScreenshot);
	fclose(fScreenshot);
	*/

	GLubyte tmpPixel[3]; // = new GLubyte[3];
	CImage* resultImage = new CImage();
	resultImage->Create(width, height, 24);

	for (int i=0; i < width; i ++){
		for (int j = 0; j < height; j++){
			glReadPixels(xStart+i, yStart+j, 1,1,GL_RGB, GL_UNSIGNED_BYTE, tmpPixel);
			//resultImage->SetPixelRGB(i,j,pixels[j*width*3+i*3],pixels[j*width*3+i*3+1],pixels[j*width*3+i*3+2]);
			resultImage->SetPixelRGB(i,j,tmpPixel[0],tmpPixel[1],tmpPixel[2]);
		}
	}

	resultImage->Save(fileName);

	delete resultImage;
	//free(pixels);
	//delete [] pixels;


	return;
}