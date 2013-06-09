#include "StdAfx.h"
#include "GlobalHelper.h"
#include <CommDlg.h>


GlobalHelper::GlobalHelper(void)
{
}


GlobalHelper::~GlobalHelper(void)
{
}	

void GlobalHelper::GetLastErrorWrapper()
{
	LPVOID lpMsgBuf;
	FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
	   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf,0, NULL );

	MessageBox( NULL, (LPWSTR)lpMsgBuf, LPWSTR("GetLastError"), MB_OK|MB_ICONINFORMATION );
	LocalFree( lpMsgBuf );

}
int GlobalHelper::ToMultiBytes( char* &pStr, const WCHAR* pwStr, int len, BOOL IsEnd)
{    
	//ASSERT_POINTER(pwStr, WCHAR) ;    
	//ASSERT( len &gt;= 0 || len == -1 ) ;    
	int nChars = WideCharToMultiByte(CP_ACP, 0, pwStr, len, NULL, 0, NULL, NULL);    
	if (len == -1)    
	{        
		--nChars;    
	}    
	if (nChars == 0)    
	{        
		return 0;    
	}    
	if(IsEnd)    
	{        
		pStr = new char[nChars+1];        
		ZeroMemory(pStr, nChars+1);    
	}    
	else    
	{        
		pStr = new char[nChars];        
		ZeroMemory(pStr, nChars);    
	}    
	WideCharToMultiByte(CP_ACP, 0, pwStr, len, pStr, nChars, NULL, NULL);    
	return nChars;
}
void GlobalHelper::GetInputImageFile(WCHAR* szImage)
{

	szImage[0] = 0;
	OPENFILENAME ofn = {0};	
	ofn.lStructSize  = sizeof(ofn);	
	ofn.Flags        = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;	
	ofn.hInstance    = GetModuleHandle(0);	
	ofn.lpstrFile    = szImage;	
	ofn.nMaxFile     = 1024;	
	ofn.nFilterIndex = 1;	
	ofn.lpstrFilter	 = TEXT("JPG\0*.jpg\0\0");	
	ofn.lpstrDefExt	 = TEXT("*");
	
	
	GetOpenFileName(&ofn);
	
}
