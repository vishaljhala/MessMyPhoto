#include "StdAfx.h"
#include "UserInterface.h"
#include "GlobalHelper.h"

#include <OleCtl.h>
#define HIMETRIC_INCH 2540
extern UserInterface g_UI;
extern GlobalHelper g_GH;

extern HWND g_hWndMain;
//extern "C" __declspec(dllimport) void CyborgSketch(char *szPath, int iScanType, int bPreviewMode, int*iErrCode);
extern "C" __declspec(dllexport) void CyborgSketch(char *szPath, int iScanType, int iGridWidth, int iOrientation, int*iErrCode);

UserInterface::UserInterface(void)
{
	m_dc = NULL;
	m_bmp = NULL;
	m_bImgLoaded = FALSE;
	m_szOutImgPath = NULL;
}


UserInterface::~UserInterface(void)
{

}
BOOL UserInterface::LoadMyImage(WCHAR *szInpPath)
{
	 // open file
    HANDLE hFile = CreateFile(szInpPath,
                              GENERIC_READ,
                              0,
                              NULL,
                              OPEN_EXISTING,
                              0,
                              NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        MessageBox(NULL,_T("Could not read file"),NULL,0);
        return FALSE;
    }

    // get file size
    DWORD dwFileSize = GetFileSize(hFile, NULL);

    if (dwFileSize == (DWORD)-1)
    {
        CloseHandle(hFile);
        MessageBox (NULL,_T("File seems to be empty"),NULL,0);
        return FALSE;
    }

    LPVOID pvData = NULL;

    // alloc memory based on file size
    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, dwFileSize);

    if (hGlobal == NULL)
    {
        CloseHandle(hFile);
        MessageBox (NULL,_T("Could not allocate memory for image"),NULL,0);
        return FALSE;
    }

    pvData = GlobalLock(hGlobal);

    if (pvData == NULL)
    {
        GlobalUnlock(hGlobal);
        CloseHandle(hFile);
        MessageBox (NULL,_T("Could not lock memory for image"),NULL,0);
        return FALSE;
    }

    DWORD dwBytesRead = 0;

    // read file and store in global memory
    BOOL bRead = ReadFile(hFile,
                          pvData,
                          dwFileSize,
                          &dwBytesRead,
                          NULL);

    GlobalUnlock(hGlobal);
    CloseHandle(hFile);

    if (!bRead)
    {
        MessageBox (NULL,_T("Could not read file"),NULL,0);
        return FALSE;
    }

    LPSTREAM pstm = NULL;

    // create IStream* from global memory
    HRESULT hr = CreateStreamOnHGlobal(hGlobal,
                                       TRUE,
                                       &pstm);

    if (!(SUCCEEDED(hr)))
    {
        MessageBox (NULL,_T("CreateStreamOnHGlobal() failed"),NULL,0);

        if (pstm != NULL)
            pstm->Release();
            
        return FALSE;
    }

    else if (pstm == NULL)
    {
        MessageBox (NULL,_T("CreateStreamOnHGlobal() failed"),NULL,0);
        return FALSE;
    }
	LPPICTURE pgpPicture=NULL;
	// Create IPicture from image file
	//if (*pgpPicture)
		//(*pgpPicture)->Release();

    hr = ::OleLoadPicture(pstm,
                          dwFileSize,
                          FALSE,
                          IID_IPicture,
                          (LPVOID *)&(pgpPicture));

    if (!(SUCCEEDED(hr)))
    {
    	pstm->Release();
        MessageBox(NULL,_T("Could not load image (hr failure)"),NULL,0);
        return FALSE;
    }

    else if (pgpPicture == NULL)
    {
    	pstm->Release();
        MessageBox(NULL,_T("Could not load image (pgpPicture failure)"),NULL,0);
        return FALSE;
    }
	long lw,lh;
	(pgpPicture)->get_Width(&lw);
	(pgpPicture)->get_Height(&lh);
	int nPixX = GetDeviceCaps(GetDC(NULL),LOGPIXELSX);
    int nPixY = GetDeviceCaps(GetDC(NULL),LOGPIXELSY);

	m_lWidth  = MulDiv(lw,  nPixX, HIMETRIC_INCH);
    m_lHeight = MulDiv(lh, nPixY, HIMETRIC_INCH);


	if(m_dc) DeleteDC(m_dc);
	if(m_bmp) DeleteObject(m_bmp);

	m_dc = CreateCompatibleDC(GetDC(NULL));
	m_bmp = CreateCompatibleBitmap(GetDC(NULL),m_lWidth,m_lHeight);
	SelectObject(m_dc,m_bmp);

	RECT rect;
	HRESULT hR = NULL;
	hR = pgpPicture->Render(m_dc,0,0,m_lWidth,m_lHeight,0,lh,lw,-lh,NULL);
	

	pgpPicture->Release();
    pstm->Release();
	
	if(!SUCCEEDED(hR))
		return FALSE;
    

	return TRUE;
}

INT_PTR CALLBACK ColorSketchSettings(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		//# of bins
		SendMessage(GetDlgItem(hDlg,IDC_SLIDER4),TBM_SETRANGEMIN,TRUE,4);
		SendMessage(GetDlgItem(hDlg,IDC_SLIDER4),TBM_SETRANGEMAX,TRUE,8);
		SendMessage(GetDlgItem(hDlg,IDC_SLIDER4),TBM_SETPOS,TRUE,4);

		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	case WM_HSCROLL:
		if(LOWORD(wParam) ==SB_ENDSCROLL)
		{
			int iBins = SendMessage(GetDlgItem(hDlg,IDC_SLIDER4),TBM_GETPOS,0,0);
			{
			} 
		}
		break;
	}
	return (INT_PTR)FALSE;

}
INT_PTR CALLBACK CyborgSettings(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		CheckRadioButton(hDlg,IDC_RADIO1,IDC_RADIO3,IDC_RADIO1);

		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if(LOWORD(wParam) == IDC_RADIO1) //laser
		{
			CheckRadioButton(hDlg,IDC_RADIO1,IDC_RADIO3,IDC_RADIO1);
			return (INT_PTR)TRUE;
		}
		else if(LOWORD(wParam) == IDC_RADIO2) // infrared
		{
			CheckRadioButton(hDlg,IDC_RADIO1,IDC_RADIO3,IDC_RADIO2);
			return (INT_PTR)TRUE;
		}
		else if(LOWORD(wParam) == IDC_RADIO3) // ultraviolet
		{
			CheckRadioButton(hDlg,IDC_RADIO1,IDC_RADIO3,IDC_RADIO3);
			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDOK)
		{
			int iErrCode;
			if(g_UI.m_szOutImgPath)
				delete g_UI.m_szOutImgPath;
			g_GH.ToMultiBytes(g_UI.m_szOutImgPath,g_UI.m_szOrigImgPath,1024,TRUE);
			g_UI.m_szOutImgPath[strlen(g_UI.m_szOutImgPath)-4] = 0;
			
			/*if (IsDlgButtonChecked(hDlg,IDC_RADIO1) == BST_CHECKED)
				CyborgSketch(g_UI.m_szOutImgPath,0,FALSE,&iErrCode);		
			else if (IsDlgButtonChecked(hDlg,IDC_RADIO2) == BST_CHECKED)
				CyborgSketch(g_UI.m_szOutImgPath,1,TRUE,&iErrCode);		
			else*/ 
				CyborgSketch(g_UI.m_szOutImgPath,0x00ccff,5,1,&iErrCode);

			wsprintf(g_UI.m_szDestImgPath,_T("%S$.jpg"),g_UI.m_szOutImgPath);
			g_UI.LoadMyImage(g_UI.m_szDestImgPath);
			InvalidateRect(g_hWndMain,NULL,FALSE);

			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;

}
