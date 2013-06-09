// MessMyPhoto.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "MessMyPhoto.h"
#include "GlobalHelper.h"
#include "UserInterface.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
HWND g_hWndMain;
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
GlobalHelper g_GH;
UserInterface g_UI;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
extern INT_PTR CALLBACK	OldPhotoSettings(HWND, UINT, WPARAM, LPARAM);
extern INT_PTR CALLBACK	CyborgSettings(HWND, UINT, WPARAM, LPARAM);
extern INT_PTR CALLBACK	ColorSketchSettings(HWND, UINT, WPARAM, LPARAM);

extern "C" __declspec(dllimport) void DoG(char *szPath,int iFilterSize, float fSigmaDist, float fSigmaColor, 
	 int bPreviewMode, int*iErrCode);
extern "C" __declspec(dllimport) void Painterly(char *szPath, float fSigma_blur_factor,int fThreshold, int iMaxStrokeLn, 
	int iMinStrokeLn, int iMaxBrushSz,int bPreviewMode, int*iErrCode);
extern "C" __declspec(dllimport) void PainterlySketch(char *szPath, int iSketchType, int iDetails, int*iErrCode);
extern "C" __declspec(dllimport) void GreyScale(char *szPath,int bPreviewMode, int*iErrCode);
extern "C" __declspec(dllimport) void NightVision(char *szPath, int bIsBorder, int*iErrCode);
extern "C" __declspec(dllimport) void ThermalDetector(char *szPath,int bPreviewMode, int*iErrCode);
extern "C" __declspec(dllimport) void HoneyMoon(char *szPath,int bPreviewMode, int*iErrCode);
extern "C" __declspec(dllimport) void SunBurst(char *szPath,int bPreviewMode, int*iErrCode);
extern "C" __declspec(dllimport) void Patronize(char *szPath, char *szCountry, int iAlphaBlend,  int iStyle , int*iErrCode);
extern "C" __declspec(dllimport) void StencilArt(char *szPath,int bPreviewMode, int*iErrCode);
extern "C" __declspec(dllimport) void BlacknWhite(char *szPath,int iSigmaColor,int bPreviewMode, int*iErrCode);
extern "C" __declspec(dllimport) void RotateInvert(char *szPath,int iAction, int*iErrCode);
extern "C" __declspec(dllimport) void CartoonEffect(char *szPath,int fSigmaColor, int bPreviewMode, int*iErrCode);
extern "C" __declspec(dllimport) void MotionFilter(char *szPath,int iFilterSize, int iAngle, int*iErrCode);
extern "C" __declspec(dllimport) void Vintage(char *szPath, int bPreviewMode, int*iErrCode);
extern "C" __declspec(dllimport) void Inkify(char *szPath, int bPreviewMode, int*iErrCode);
extern "C" __declspec(dllexport) void GoldEngross(char *szPath, int iDepth, int*iErrCode);
extern "C" __declspec(dllimport) void Jigsaw(char *szPath, int bPreviewMode, int*iErrCode);
extern "C" __declspec(dllimport) void Reflection(char *szPath,  int iDistance, int iShadowLength, int iShadowIntensity, int*iErrCode);
extern "C" __declspec(dllimport) void Barcode(char *szPath, int bPreviewMode, int*iErrCode);
extern "C" __declspec(dllimport) void HDR(char *szPath, int bPreviewMode, int*iErrCode);
extern "C" __declspec(dllimport) void FisheyeLens(char *szPath, int iStyle, int*iErrCode);
extern "C" __declspec(dllimport) void CrossProcess(char *szPath, int bPreviewMode, int*iErrCode);
extern "C" __declspec(dllimport) void CynoType(char *szPath, int bPreviewMode, int*iErrCode);
extern "C" __declspec(dllimport) void HolgaArt(char *szPath, int bPreviewMode, int*iErrCode);
extern "C" __declspec(dllimport) void TieNDye(char *szPath, int iSoftColor, int iHardColor, int iDetails, int*iErrCode);
extern "C" __declspec(dllimport) void WaterColor(char *szPath, int bPreviewMode, int*iErrCode);
extern "C" __declspec(dllimport) void OilPaint(char *szPath, int bPreviewMode, int*iErrCode);
extern "C" __declspec(dllimport) void OldPhoto(char *szPath, int bIsBnW, int iBlur, int iContrast, int bIsDamaged, int*iErrCode);
extern "C" __declspec(dllimport) void PerfectSketch(char *szPath, int iScanType, int bPreviewMode, int*iErrCode);
extern "C" __declspec(dllimport) void MotherNature(char *szPath, int iParam0,int iParam1, int iParam2, int iParam3, int*iErrCode);


int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MESSMYPHOTO, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MESSMYPHOTO));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MESSMYPHOTO));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_MESSMYPHOTO);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }
   g_hWndMain = hWnd;

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
void MyHandler(HWND hWnd,int ID)
{
	int iErrCode;
	if(g_UI.m_bImgLoaded)
	{
		if(g_UI.m_szOutImgPath)
			delete g_UI.m_szOutImgPath;
		g_GH.ToMultiBytes(g_UI.m_szOutImgPath,g_UI.m_szOrigImgPath,1024,TRUE);
		g_UI.m_szOutImgPath[strlen(g_UI.m_szOutImgPath)-4] = 0;
					
		switch(ID)
		{
		case IDM_DOG1:
			PainterlySketch(g_UI.m_szOutImgPath,1,100, &iErrCode);
			break;
		case IDM_PAINTERLY1:
			//Painterly(g_UI.m_szOutImgPath, 0.5,100, 16, 4, 8,FALSE, &iErrCode);
			Painterly(g_UI.m_szOutImgPath, 0.5,50, 32, 8, 16,FALSE, &iErrCode);
			break;
		case IDM_GRAYSCALE1:
			GreyScale(g_UI.m_szOutImgPath,FALSE, &iErrCode);
			break;
		case IDM_BLACK_N_WHITE1:
			BlacknWhite(g_UI.m_szOutImgPath,16,FALSE, &iErrCode);
			break;
		case IDM_ROTATE_LEFT1:
			RotateInvert(g_UI.m_szOutImgPath,1, &iErrCode);
			break;
		case IDM_ROTATE_RIGHT1:
			RotateInvert(g_UI.m_szOutImgPath,2, &iErrCode);
			break;
		case IDM_INVERT_COLORS1:
			RotateInvert(g_UI.m_szOutImgPath,3, &iErrCode);
			break;
		case IDM_BILATERAL_FILTER1:
			CartoonEffect(g_UI.m_szOutImgPath,16,FALSE, &iErrCode);
			break;
		case IDM_NIGHT_VISION1:
			NightVision(g_UI.m_szOutImgPath,1, &iErrCode);
			break;
		case IDM_THERMAL_DETECTOR1:
			ThermalDetector(g_UI.m_szOutImgPath,FALSE, &iErrCode);
			break;
		case IDM_HONEYMOON1:
			HoneyMoon(g_UI.m_szOutImgPath,FALSE, &iErrCode);
			break;
		case IDM_SUN_BURST1:
			SunBurst(g_UI.m_szOutImgPath,FALSE, &iErrCode);
			break;
		case IDM_PATRONIZE1:
			Patronize(g_UI.m_szOutImgPath,"United States Of America",15,0, &iErrCode);
			break;
		case IDM_STENCIL_ART1:
			StencilArt(g_UI.m_szOutImgPath,FALSE, &iErrCode);
			break;
		case IDM_MOTION_BLUR1:
			MotionFilter(g_UI.m_szOutImgPath,11,90, &iErrCode);
			break;
		case IDM_VINTAGE1:
			Vintage(g_UI.m_szOutImgPath,FALSE, &iErrCode);
			break;
		case IDM_INKIFY1:
			Inkify(g_UI.m_szOutImgPath,FALSE, &iErrCode);
			break;
		case IDM_GOLD_ENGROSS1:
			GoldEngross(g_UI.m_szOutImgPath,50, &iErrCode);
			break;
		case IDM_JIGSAW1:
			Jigsaw(g_UI.m_szOutImgPath,FALSE, &iErrCode);
			break;
		case IDM_REFLECTION1:
			Reflection(g_UI.m_szOutImgPath,0,20,70, &iErrCode);
			break;
		case IDM_BARCODE1:
			Barcode(g_UI.m_szOutImgPath,FALSE, &iErrCode);
			break;
		case IDM_HDR1:
			HDR(g_UI.m_szOutImgPath,FALSE, &iErrCode);
			break;
		case IDM_FISHEYE_LENS1:
			FisheyeLens(g_UI.m_szOutImgPath,0, &iErrCode);
			break;
		case IDM_CROSS_PROCESS1:
			CrossProcess(g_UI.m_szOutImgPath,FALSE, &iErrCode);
			break;
		case IDM_CYAN_TYPE1:
			CynoType(g_UI.m_szOutImgPath,FALSE, &iErrCode);
			break;
		case IDM_HOLGAART1:
			HolgaArt(g_UI.m_szOutImgPath,FALSE, &iErrCode);
			break;
		case IDM_TIE_AND_DYE1:
			TieNDye(g_UI.m_szOutImgPath,0xFFE881,0x9E4E10, 25,&iErrCode);
			break;
		case IDM_WATER_COLOR1:
			WaterColor(g_UI.m_szOutImgPath,FALSE, &iErrCode);
			break;
		case IDM_OIL_PAINTING1:
			OilPaint(g_UI.m_szOutImgPath,FALSE, &iErrCode);
			break;
		case IDM_OLD_PHOTO1:
			OldPhoto(g_UI.m_szOutImgPath,TRUE,15,20,TRUE,&iErrCode);
			break;
		case IDM_PERFECT_SKETCH1:
			PerfectSketch(g_UI.m_szOutImgPath,FALSE,false,&iErrCode);
			break;
		case IDM_MOTHER_NATURE1:
			MotherNature(g_UI.m_szOutImgPath,3,0,1,0,&iErrCode);//1=# of pixels, 2=whiteness%, 3=blur box size*2
			break;
		}			
		wsprintf(g_UI.m_szDestImgPath,_T("%S$.jpg"),g_UI.m_szOutImgPath);
		g_UI.LoadMyImage(g_UI.m_szDestImgPath);
		InvalidateRect(g_hWndMain,NULL,FALSE);
	}
	else
		MessageBox(hWnd,_T("Select Image First"),NULL,NULL);
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent, w,h;
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rect;
	GetClientRect(hWnd,&rect);
	
	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_OLD_PHOTO1:
			MyHandler(hWnd,wmId);
			break;
/*			if(g_UI.m_bImgLoaded)
				DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, OldPhotoSettings);
			else
				MessageBox(hWnd,_T("Select Image First"),NULL,NULL);
			
			break;*/
		case IDM_DOG1:
			MyHandler(hWnd,wmId);
			break;
		case IDM_BILATERAL_FILTER1:
			MyHandler(hWnd,wmId);
			break;
		case IDM_CYBORG1:
			if(g_UI.m_bImgLoaded)
				DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG3), hWnd, CyborgSettings);
			else
				MessageBox(hWnd,_T("Select Image First"),NULL,NULL);
			break;
		case IDM_PAINTERLY1:
			MyHandler(hWnd,wmId);
			break;
		case IDM_GRAYSCALE1:
			MyHandler(hWnd,wmId);
			break;
		case IDM_NIGHT_VISION1:
			MyHandler(hWnd,wmId);
			break;
		case IDM_THERMAL_DETECTOR1:
			MyHandler(hWnd,wmId);
			break;
		case IDM_HONEYMOON1:
			MyHandler(hWnd,wmId);
			break;
		case IDM_SUN_BURST1:
			MyHandler(hWnd,wmId);
			break;
		case IDM_PATRONIZE1:
			MyHandler(hWnd,wmId);
			break;
		case IDM_STENCIL_ART1:
			MyHandler(hWnd,wmId);
			break;
		case IDM_MOTION_BLUR1:
			MyHandler(hWnd,wmId);
			break;
		case IDM_VINTAGE1:
			MyHandler(hWnd,wmId);
			break;
		case IDM_INKIFY1:
			MyHandler(hWnd,wmId);
			break;
		case IDM_GOLD_ENGROSS1:
			MyHandler(hWnd,wmId);
			break;
		case IDM_JIGSAW1:
			MyHandler(hWnd,wmId);
			break;
		case IDM_REFLECTION1:
			MyHandler(hWnd,wmId);
			break;
		case IDM_BARCODE1:
			MyHandler(hWnd,wmId);
			break;
		case IDM_HDR1:
			MyHandler(hWnd,wmId);
			break;
		case IDM_FISHEYE_LENS1:
			MyHandler(hWnd,wmId);
			break;
		case IDM_CROSS_PROCESS1:
			MyHandler(hWnd,wmId);
			break;
		case IDM_CYAN_TYPE1:
			MyHandler(hWnd,wmId);
			break;
		case IDM_HOLGAART1:
			MyHandler(hWnd,wmId);
			break;
		case IDM_TIE_AND_DYE1:
			MyHandler(hWnd,wmId);
			break;
		case IDM_BLACK_N_WHITE1:
			MyHandler(hWnd,wmId);
			break;
		case IDM_ROTATE_LEFT1:
			MyHandler(hWnd,wmId);
			break;
		case IDM_ROTATE_RIGHT1:
			MyHandler(hWnd,wmId);
			break;
		case IDM_INVERT_COLORS1:
			MyHandler(hWnd,wmId);
			break;
		case IDM_WATER_COLOR1:
			MyHandler(hWnd,wmId);
			break;
		case IDM_OIL_PAINTING1:
			MyHandler(hWnd,wmId);
			break;
		case IDM_MOTHER_NATURE1:
			MyHandler(hWnd,wmId);
			break;
		case IDM_OPEN_PHOTO1:
			g_GH.GetInputImageFile(g_UI.m_szOrigImgPath);
			if(wcslen(g_UI.m_szOrigImgPath)>0)
			{
				if( !g_UI.LoadMyImage(g_UI.m_szOrigImgPath) )
					MessageBox(NULL,_T("Failed to load Image"),NULL,0);
				g_UI.m_bImgLoaded = TRUE;

				InvalidateRect(hWnd,NULL,TRUE);

			}
			else
				MessageBox(NULL,_T("No Image Selected"),NULL,0);
			break;
		case IDM_PERFECT_SKETCH1:
			MyHandler(hWnd,wmId);
			break;

		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		if(rect.right<g_UI.m_lWidth)
		{
			w = rect.right;
			float f = ((float)rect.right/g_UI.m_lWidth);
			h = (float)g_UI.m_lHeight*f;
		}
		else
		{
			w= g_UI.m_lWidth;
			h = g_UI.m_lHeight;
		}

		if(rect.bottom<h)
		{
			w = (float)(w*rect.bottom/h);
			h = rect.bottom;
		}
			

		hdc = BeginPaint(hWnd, &ps);
		if(g_UI.m_dc)
		{
			SetStretchBltMode(hdc,STRETCH_HALFTONE );
			StretchBlt(hdc,(rect.right-w)/2,(rect.bottom-h)/2,w,h,g_UI.m_dc,0,0,g_UI.m_lWidth,g_UI.m_lHeight,SRCCOPY);
		}
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
