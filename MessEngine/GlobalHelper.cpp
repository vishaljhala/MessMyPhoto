#include "StdAfx.h"
#include "GlobalHelper.h"
#include <CommDlg.h>
#include <Commctrl.h>


GlobalHelper::GlobalHelper()
{
}
void GlobalHelper::Init(char *szFileName,int *iErrCode)
{
	fDebug = NULL;
	*iErrCode = 0;
	
	if(szFileName)
	{
		char szPath[2048];
		szPath[0] = 0;
		strcpy(szPath,szFileName);
		strcat(szPath,".log");
		
		WCHAR wszPath[2048];
		wsprintf(wszPath,_T("%S"),szPath);
		fDebug = CreateFile(wszPath,GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
		if (fDebug == INVALID_HANDLE_VALUE)
			fDebug = CreateFile(wszPath,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS ,FILE_ATTRIBUTE_NORMAL,NULL);
		else
		{
			//fDebug = NULL;
			DWORD dw = SetFilePointer(fDebug,GetFileSize(fDebug,NULL),0,FILE_BEGIN);
			//if(dw ==INVALID_SET_FILE_POINTER)
		}
	}
	m_hTBB = m_hOpenCVCore = m_hOpenCVImgProc = m_hOpenCVHighGui = NULL;

#ifdef _DEBUG 
	m_hTBB =		   LoadLibrary(_T("c:\\home\\bin\\tbb.dll"));
	m_hOpenCVCore =    LoadLibrary(_T("c:\\home\\bin\\opencv_core230.dll"));
	m_hOpenCVImgProc = LoadLibrary(_T("C:\\home\\bin\\opencv_imgproc230.dll"));
	m_hOpenCVHighGui = LoadLibrary(_T("C:\\home\\bin\\opencv_highgui230.dll"));
#else

	m_hTBB =		   LoadLibrary(_T("D:\\home\\C339925\\messmyphoto.com\\bin\\tbb.dll"));
	m_hOpenCVCore =    LoadLibrary(_T("D:\\home\\C339925\\messmyphoto.com\\bin\\opencv_core230.dll"));
	m_hOpenCVImgProc = LoadLibrary(_T("D:\\home\\C339925\\messmyphoto.com\\bin\\opencv_imgproc230.dll"));
	m_hOpenCVHighGui = LoadLibrary(_T("D:\\home\\C339925\\messmyphoto.com\\bin\\opencv_highgui230.dll"));

	//m_hTBB =		   LoadLibrary(_T("C:\\home\\bin\\tbb.dll"));
	//m_hOpenCVCore =    LoadLibrary(_T("C:\\home\\bin\\opencv_core230.dll"));
	//m_hOpenCVImgProc = LoadLibrary(_T("C:\\home\\bin\\opencv_imgproc230.dll"));
	//m_hOpenCVHighGui = LoadLibrary(_T("C:\\home\\bin\\opencv_highgui230.dll"));
#endif
	//if(m_hOpenCVCore == NULL)
		//DebugOut("Failed to load OpenCVCore DLL",FALSE);
	//if(m_hOpenCVImgProc == NULL)
		//DebugOut("Failed to load OpenCVImgProc DLL",FALSE);
	//if(m_hOpenCVHighGui == NULL)
		//DebugOut("Failed to load OpenCVHighGui DLL",FALSE);
	if (m_hOpenCVCore == NULL || m_hOpenCVImgProc == NULL || m_hOpenCVHighGui == NULL )
	{		
		*iErrCode = 1;
		return;
	}
	cvResizeProxy = (cvResizePtr)GetProcAddress(m_hOpenCVImgProc,"cvResize") ;
	cvCvtColorProxy = (cvCvtColorPtr)GetProcAddress(m_hOpenCVImgProc,"cvCvtColor") ;
	cvSmoothProxy = (cvSmoothPtr)GetProcAddress(m_hOpenCVImgProc,"cvSmooth");
	cvDilateProxy = (cvDilatePtr)GetProcAddress(m_hOpenCVImgProc,"cvDilate");
	cvErodeProxy = (cvErodePtr)GetProcAddress(m_hOpenCVImgProc,"cvErode");
	cvGetRotationMatrix2DProxy = (cvGetRotationMatrix2DPtr)( m_hOpenCVImgProc,"getRotationMatrix2D");
	cvWarpAffineProxy =  (cvWarpAffinePtr)(m_hOpenCVImgProc,"warpAffine");

	cvCreateImageProxy = (cvCreateImagePtr) GetProcAddress(m_hOpenCVCore,"cvCreateImage") ;
	cvGetSizeProxy = (cvGetSizePtr) GetProcAddress(m_hOpenCVCore,"cvGetSize") ;
	cvReleaseImageProxy = (cvReleaseImagePtr) GetProcAddress(m_hOpenCVCore,"cvReleaseImage") ;
	cvGet2DProxy = (cvGet2DPtr)GetProcAddress(m_hOpenCVCore,"cvGet2D") ;
	cvSet2DProxy = (cvSet2DPtr)GetProcAddress(m_hOpenCVCore,"cvSet2D") ;
	cvSetProxy = (cvSetPtr)GetProcAddress(m_hOpenCVCore,"cvSet") ;
	cvLineProxy = (cvLinePtr)GetProcAddress(m_hOpenCVCore,"cvLine") ;
	cvCopyProxy = (cvCopyPtr)GetProcAddress(m_hOpenCVCore,"cvCopy");
	cvTransposeProxy = (cvTransposePtr)GetProcAddress(m_hOpenCVCore,"cvTranspose");
	cvFlipProxy = (cvFlipPtr)GetProcAddress(m_hOpenCVCore,"cvFlip");
	cvEllipseProxy = (cvEllipsePtr)GetProcAddress(m_hOpenCVCore,"cvEllipse");

	cvLoadImageProxy = (cvLoadImagePtr) GetProcAddress(m_hOpenCVHighGui,"cvLoadImage") ;
	cvSaveImageProxy = (cvSaveImagePtr) GetProcAddress(m_hOpenCVHighGui,"cvSaveImage") ;

}

GlobalHelper::~GlobalHelper(void)
{
	if(m_hOpenCVCore)
		FreeLibrary(m_hOpenCVCore);	
	if(m_hOpenCVImgProc)
		FreeLibrary(m_hOpenCVImgProc);
	if(m_hOpenCVHighGui)
		FreeLibrary(m_hOpenCVHighGui);
	if(m_hTBB)
		FreeLibrary(m_hTBB);

	CloseHandle(fDebug);
}
void GlobalHelper::ReportStatus(HWND hDlg,BOOL bStepIT)
{
	/*if(bStepIT)
		SendMessage(hDlg,PBM_STEPIT,0,0);
	MSG msg;
	while (PeekMessage(&msg, NULL,  WM_PAINT, WM_PAINT, PM_REMOVE)) 
    { 
		DispatchMessage(&msg);
	}*/

}
void GlobalHelper::GetLastErrorWrapper()
{
	LPVOID lpMsgBuf;
	FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
	   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf,0, NULL );

	MessageBox( NULL, (LPWSTR)lpMsgBuf, LPWSTR("GetLastError"), MB_OK|MB_ICONINFORMATION );
	LocalFree( lpMsgBuf );

}
int GlobalHelper::WcharToChar( char* &pStr, const WCHAR* pwStr, int len, BOOL IsEnd)
{    
	//ASSERT_POINTER(pwStr, WCHAR) ;    
	//ASSERT( len &gt;= 0 || len == -1 ) ;
	
	if(pStr)
		delete(pStr);

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
int GlobalHelper::CharToWchar( const char* pStr, WCHAR* &pwStr)
{
	wsprintf(pwStr,_T("%S"),pStr);
	return 1;
}
int GlobalHelper::getTimeDiff(SYSTEMTIME *st1, SYSTEMTIME *st2)
{
    LARGE_INTEGER nFrom, nTo;
    __int64 n64From, n64To, nDiff;
    FILETIME fileTimeTemp;
    float fReturn = 0.0f;
 
  
    SystemTimeToFileTime(st1, &fileTimeTemp);
    nFrom.HighPart = fileTimeTemp.dwHighDateTime;
    nFrom.LowPart = fileTimeTemp.dwLowDateTime;
 
    n64From = (__int64)&nFrom; // timeFrom as an int
 
    SystemTimeToFileTime(st2, &fileTimeTemp);
    nTo.HighPart = fileTimeTemp.dwHighDateTime;
    nTo.LowPart = fileTimeTemp.dwLowDateTime;
 
    n64To = (__int64)&nTo;  // timeTo as an int
 
    nDiff = n64To - n64From; // difference should be positive
 
	return nDiff;
}

void GlobalHelper::DebugOut(char* szMsg)
{
	DWORD i;

	if(!fDebug)
		return;
	//char ch[] = "\r\n";
	WriteFile(fDebug,szMsg,strlen(szMsg),&i,NULL);
	WriteFile(fDebug,"\r\n",2,&i,NULL);
	/*if(bAppendTime)
	{
		SYSTEMTIME st;
		GetLocalTime(&st);
		char pszTimec[128];pszTimec[0] = 0;
		
		sprintf(pszTimec,"%d-%02d-%02d %02d:%02d:%02d:%04d \0",st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
		WriteFile(fDebug,pszTimec,strlen(pszTimec),&i,NULL);
		WriteFile(fDebug,"\t",1,&i,NULL);
	}*/

}
/*void GlobalHelper::OpenFileDialogWrapper(WCHAR* szImage)
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
	
}*/
IMAGE GlobalHelper::load_image (char *name,BOOL bLoadPreview=false)
{
	IMAGE x=0;
	IplImage *image = 0;

	char nameW[2048];
	nameW[0] = 0;
	strcpy(nameW,name);
	if(bLoadPreview)
		strcat(nameW,"tn.jpg");
	else
		strcat(nameW,".jpg");

	//char *szName= NULL;
	//WcharToChar(szName,nameW,wcslen(nameW),TRUE);

	image = cvLoadImageProxy(nameW, 0);
	if (image <= 0) return 0;
	
	x = fromOpenCV (image);
	cvReleaseImageProxy( &image );

	//delete szName;

	return x;
}

UCHAR GlobalHelper::GetByte(IplImage *img, int i, int j, int iChannel)
{
	if(iChannel<3)
		return *(uchar*)(img->imageData + i*img->widthStep + j*3 + iChannel);
	else
		return (UCHAR)(((float)*(uchar*)(img->imageData + i*img->widthStep + j*3 + 0)) +
		*(uchar*)(img->imageData + i*img->widthStep + j*3 + 1) +
		*(uchar*)(img->imageData + i*img->widthStep + j*3 + 2) )/3;
}
void GlobalHelper::SetByte(IplImage *img, int i, int j,int iChannel, UCHAR byte)
{
	*(uchar*)(img->imageData + i*img->widthStep + j*3 + iChannel) = byte;
}

int GlobalHelper::load_RGB (char *name,IplImage**ppiImg,BOOL bLoadPreview)
{
		
	//if(*ppiImg)
		//DebugOut("GlobalHelper::load_RGB - Memory Leak situation",FALSE);
	char nameW[1024];
	nameW[0] = 0;
	strcpy(nameW,name);
	strcat(nameW,".jpg");

	// Read the image from a file into Ip1Image
	*ppiImg = cvLoadImageProxy(nameW, 1);
	if (*ppiImg <= 0) return 0;

	return 1;
	/*if(bLoadPreview)
	{
		float fPercent;
		if((*ppiImg)->width > (*ppiImg)->height)
			fPercent = (float)800/(*ppiImg)->width;
		else
			fPercent = (float)800/(*ppiImg)->height;

		IplImage *destination = cvCreateImageProxy
		( cvSize((int)((*ppiImg)->width*fPercent) , (int)((*ppiImg)->height*fPercent) ),
										 (*ppiImg)->depth, (*ppiImg)->nChannels );

		if (destination<=0 )
		{
			DebugOut("Preview: Failed to Destination Image.",FALSE);
			return 0 ;
		}
		cvResizeProxy((*ppiImg), destination,1);
		
		cvReleaseImageProxy(ppiImg);
		*ppiImg = destination;

	}*/
	IplImage *destination = cvCreateImageProxy
	( cvSize((int)((*ppiImg)->width*0.65) , (int)((*ppiImg)->height*0.65) ),
										(*ppiImg)->depth, (*ppiImg)->nChannels );

	if (destination<=0 )
	{
		//DebugOut("GlobalHelper:LoadImage : Failed to Destination Image.",FALSE);
		return 0 ;
	}
	cvResizeProxy((*ppiImg), destination,1);
		
	cvReleaseImageProxy(ppiImg);
	*ppiImg = destination;

	return 1;
}
int GlobalHelper::load_RGB (IMAGE *r, IMAGE *g, IMAGE *b, char *name)
{
	IplImage *image = 0;
	IMAGE i1, i2, i3;
	int i,j;

	// Read the image from a file into Ip1Image
	image = cvLoadImageProxy(name, 1);
	if (image <= 0) return 0;

	// Create three AIPCV images of correct size
	i1 = newimage (image->height, image->width);
	i2 = newimage (image->height, image->width);
	i3 = newimage (image->height, image->width);

	// Copy pixels from Ip1Image to AIPCV images
	for (i=0; i<image->height; i++)
		for (j=0; j<image->width; j++)
		{
		  i1->data[i][j] = (image->imageData+i*image->widthStep)[j*image->nChannels+0];
		  i2->data[i][j] = (image->imageData+i*image->widthStep)[j*image->nChannels+1];
		  i3->data[i][j] = (image->imageData+i*image->widthStep)[j*image->nChannels+2];
		}
	cvReleaseImageProxy(&image);
	*r = i3; 
	*g = i2;
	*b = i1;
	return 1;
}
int GlobalHelper::save_image (IMAGE x, char *name)
{
	IplImage *image = 0;

	char nameW[1024];
	nameW[0] = 0;
	strcpy(nameW,name);
	strcat(nameW,"$.jpg");

	image = toOpenCV (x);
	if (image <0) return 0;
	
	int iRet = cvSaveImageProxy( nameW, image );
	//if(!iRet)
		//DebugOut("GlobalHelper:save_image - Failed to save image",FALSE);

	cvReleaseImageProxy( &image );
	return iRet;
	//delete szName;
}
int GlobalHelper::save_RGB (char *name,IplImage *pImg)
{
	int k;
	if (pImg <= 0) 
		return 0;

	char nameW[1024];
	nameW[0] = 0;
	strcpy(nameW,name);
	strcat(nameW,"$.jpg");

	/*IplImage *destination = cvCreateImageProxy
	( cvSize((int)((pImg)->width*1.538462) , (int)((pImg)->height*1.538462) ),
										(pImg)->depth, (pImg)->nChannels );

	if (destination<=0 )
	{
		DebugOut("GlobalHelper:SaveImage : Failed to Destination Image.",FALSE);
		return 0 ;
	}
	cvResizeProxy((pImg), destination,1);
	k = cvSaveImageProxy(nameW, destination);
	return k;

	cvReleaseImageProxy(&destination);
	//delete szName;
		*/
	k = cvSaveImageProxy(nameW, pImg);
	return k;


}
IMAGE GlobalHelper::fromOpenCV (IplImage *x)
{
	IMAGE img = NULL;
	int color=0, i=0;
	int k=0, j=0;
	CvScalar s;
	
	if ((x->depth==IPL_DEPTH_8U) &&(x->nChannels==1))								// 1 Pixel (grey) image
		img = newimage (x->height, x->width);
	else if ((x->depth==8) && (x->nChannels==3)) //Color
	{
		color = 1;
		img = newimage (x->height, x->width);
	}
	else return 0;
	if(!img)
	{
		//DebugOut("GlobalHelper:fromOpenCV - failed to allocate new image",FALSE);
		return 0;
	}
	for (i=0; i<x->height; i++)
	{
		for (j=0; j<x->width; j++)
		{
			s = cvGet2DProxy (x, i, j);
			if (color) 
			  k = (unsigned char)((s.val[0] + s.val[1] + s.val[2])/3);
			else k = (unsigned char)(s.val[0]);
			img->data[i][j] = k;
		}
	}
	return img;
}
struct image  *GlobalHelper::newimage (int nr, int nc)
{
	struct image  *x;                /* New image */
	int i;
	unsigned char *p;

	if (nr < 0 || nc < 0) {
		//DebugOut("GlobalHelper:newimage - Bad image size",FALSE);
		return 0;
	}

/*      Allocate the image structure    */
	x = (struct image  *) malloc( sizeof (struct image) );
	if (!x) {
		//DebugOut ("GlobalHelper:newimage - Out of storage",FALSE);
		return 0;
	}

/*      Allocate and initialize the header      */

	x->info = (struct header *)malloc( sizeof(struct header) );
	if (!(x->info)) 
	{
		//DebugOut ("GlobalHelper:newimage - Out of storage",FALSE);
		return 0;
	}
	x->info->nr = nr;       x->info->nc = nc;
	x->info->oi = x->info->oj = 0;

/*      Allocate the pixel array        */

	x->data = (unsigned char **)malloc(sizeof(unsigned char *)*nr); 

/* Pointers to rows */
	if (!(x->data)) 
	{
		//DebugOut ("GlobalHelper:newimage - Out of storage",FALSE);
		return 0;
	}

	x->data[0] = (unsigned char *)malloc (nr*nc);
	p = x->data[0];
	if (x->data[0]==0)
	  {
		//DebugOut ("GlobalHelper:newimage - Out of storage",FALSE);
		exit(1);
	  }

	for (i=1; i<nr; i++) 
	{
	  x->data[i] = (p+i*nc);
	}

	return x;
}
float ** GlobalHelper::FloatHeap2D (int nr, int nc)
{
	float **x;
	int i;

	x = (float **)calloc ( nr, sizeof (float *) );
	if (x == 0)
		return NULL;
	for (i=0; i<nr; i++)
	{  
	  x[i] = (float *) calloc ( nc, sizeof (float)  );
	  if (x[i] == 0)
		  return NULL;
	}
	return x;
}
int ** GlobalHelper::IntHeap2D (int nr, int nc)
{
	int **x;
	int i;

	x = (int **)calloc ( nr, sizeof (int *) );
	if (x == 0)
		return NULL;
	for (i=0; i<nr; i++)
	{  
	  x[i] = (int *) calloc ( nc, sizeof (int)  );
	  if (x[i] == 0)
		  return NULL;
	}
	return x;
}
IplImage *GlobalHelper::toOpenCV (IMAGE x)
{
	IplImage *img;
	int i=0, j=0;
	CvScalar s;
	
	img = cvCreateImageProxy(cvSize(x->info->nc, x->info->nr),8, 1);
	if(!img)
	{
		//DebugOut("GlobalHelper:toOpenCV - CreateImage failed",FALSE);
		return 0;
	}
	
	for (i=0; i<x->info->nr; i++)
	{
		for (j=0; j<x->info->nc; j++)
		{
			s.val[0] = x->data[i][j];
			cvSet2DProxy (img, i,j,s);
		}
	}
	return img;
}


int GlobalHelper::get_RGB (IMAGE *r, IMAGE *g, IMAGE *b, IplImage *image)
{
	
	IMAGE i1, i2, i3;
	int i,j;

	//if(*r || *g || *b)
		//DebugOut("GlobalHelper::get_RGB - Memory Leak situation",FALSE);
	// Read the image from a file into Ip1Image
	
	// Create three AIPCV images of correct size
	i1 = newimage (image->height, image->width);
	i2 = newimage (image->height, image->width);
	i3 = newimage (image->height, image->width);

	if( !i1 || !i2 || !i3 )
		return 0;

	// Copy pixels from Ip1Image to AIPCV images
	for (i=0; i<image->height; i++)
		for (j=0; j<image->width; j++)
		{
		  i1->data[i][j] = (image->imageData+i*image->widthStep)[j*image->nChannels+0];
		  i2->data[i][j] = (image->imageData+i*image->widthStep)[j*image->nChannels+1];
		  i3->data[i][j] = (image->imageData+i*image->widthStep)[j*image->nChannels+2];
		}
	cvReleaseImageProxy(&image);
	*r = i3; 
	*g = i2;
	*b = i1;
	return 1;
}
int GlobalHelper::get_HSV (IMAGE *r, IMAGE *g, IMAGE *b, IplImage *image)
{
	IMAGE i1, i2, i3;
	int i,j;

	//if(*r || *g || *b)
		//DebugOut("GlobalHelper::get_HSV - Memory Leak situation",FALSE);

	if (image <= 0) return 0;

	// Create three AIPCV images of correct size
	i1 = newimage (image->height, image->width);
	i2 = newimage (image->height, image->width);
	i3 = newimage (image->height, image->width);
	if( !i1 || !i2 || !i3 )
		return 0;

	// Copy pixels from Ip1Image to AIPCV images
	for (i=0; i<image->height; i++)
		for (j=0; j<image->width; j++)
		{
		  i1->data[i][j] = (image->imageData+i*image->widthStep)[j*image->nChannels+0];
		  i2->data[i][j] = (image->imageData+i*image->widthStep)[j*image->nChannels+1];
		  i3->data[i][j] = (image->imageData+i*image->widthStep)[j*image->nChannels+2];
		}
	//cvReleaseImage(&image);
	*r = i1; 
	*g = i2;
	*b = i3;
	return 1;
}

IplImage* GlobalHelper::set_RGB (IMAGE r, IMAGE g, IMAGE b)
{
	IplImage *image = 0;
	int i,j,k;
	CvScalar s;

	if ( (r->info->nc != g->info->nc) || (r->info->nr != g->info->nr) ) return 0;
	if ( (r->info->nc != b->info->nc) || (r->info->nr != b->info->nr) ) return 0;

	// Create an  IplImage
	image = cvCreateImageProxy(cvSize(r->info->nc, r->info->nr),IPL_DEPTH_8U,3);
	if (image <= 0) 
	{
		//DebugOut("GlobalHelper::set_RGB - Create Image Failed",FALSE);
		return 0;
	}

		
	for (i=0; i<image->height; i++)
		for (j=0; j<image->width; j++)
		{
		  (image->imageData+i*image->widthStep)[j*image->nChannels+0] = b->data[i][j];
		  (image->imageData+i*image->widthStep)[j*image->nChannels+1] = g->data[i][j];
		  (image->imageData+i*image->widthStep)[j*image->nChannels+2] = r->data[i][j];
		} 

	return image;

}
IplImage* GlobalHelper::set_HSV (IMAGE r, IMAGE g, IMAGE b)
{
	IplImage *image = 0;
	int i,j,k;
	CvScalar s;

	if ( (r->info->nc != g->info->nc) || (r->info->nr != g->info->nr) ) return 0;
	if ( (r->info->nc != b->info->nc) || (r->info->nr != b->info->nr) ) return 0;

	// Create an  IplImage
	image = cvCreateImageProxy(cvSize(r->info->nc, r->info->nr),IPL_DEPTH_8U,3);
	if (image <= 0) 
	{
		//DebugOut("GlobalHelper::set_HSV - Create Image Failed",FALSE);
		return 0;
	}

		
	for (i=0; i<image->height; i++)
		for (j=0; j<image->width; j++)
		{
		  (image->imageData+i*image->widthStep)[j*image->nChannels+0] = r->data[i][j];
		  (image->imageData+i*image->widthStep)[j*image->nChannels+1] = g->data[i][j];
		  (image->imageData+i*image->widthStep)[j*image->nChannels+2] = b->data[i][j];
		} 
//	k = cvSaveImage(name, image);
//	cvReleaseImage(&image);
	return image;

}

IplImage* GlobalHelper::convertImageRGBtoHSV(const IplImage *imageRGB)
{
	float fR, fG, fB;
	float fH, fS, fV;
	const float FLOAT_TO_BYTE = 255.0f;
	const float BYTE_TO_FLOAT = 1.0f / FLOAT_TO_BYTE;

	// Create a blank HSV image
	IplImage *imageHSV = cvCreateImageProxy(cvGetSizeProxy(imageRGB), 8, 3);
	if (!imageHSV || imageRGB->depth != 8 || imageRGB->nChannels != 3) {
		//DebugOut("ERROR in convertImageRGBtoHSV()! Bad input image.",FALSE);
		return 0;
	}

	int h = imageRGB->height;		// Pixel height.
	int w = imageRGB->width;		// Pixel width.
	int rowSizeRGB = imageRGB->widthStep;	// Size of row in bytes, including extra padding.
	char *imRGB = imageRGB->imageData;	// Pointer to the start of the image pixels.
	int rowSizeHSV = imageHSV->widthStep;	// Size of row in bytes, including extra padding.
	char *imHSV = imageHSV->imageData;	// Pointer to the start of the image pixels.
	for (int y=0; y<h; y++) {
		for (int x=0; x<w; x++) {
			// Get the RGB pixel components. NOTE that OpenCV stores RGB pixels in B,G,R order.
			uchar *pRGB = (uchar*)(imRGB + y*rowSizeRGB + x*3);
			int bB = *(uchar*)(pRGB+0);	// Blue component
			int bG = *(uchar*)(pRGB+1);	// Green component
			int bR = *(uchar*)(pRGB+2);	// Red component

			// Convert from 8-bit integers to floats.
			fR = bR * BYTE_TO_FLOAT;
			fG = bG * BYTE_TO_FLOAT;
			fB = bB * BYTE_TO_FLOAT;
			// Convert from RGB to HSV, using float ranges 0.0 to 1.0.
			float fDelta;
			float fMin, fMax;
			int iMax;
			// Get the min and max, but use integer comparisons for slight speedup.
			if (bB < bG) {
				if (bB < bR) {
					fMin = fB;
					if (bR > bG) {
						iMax = bR;
						fMax = fR;
					}
					else {
						iMax = bG;
						fMax = fG;
					}
				}
				else {
					fMin = fR;
					fMax = fG;
					iMax = bG;
				}
			}
			else {
				if (bG < bR) {
					fMin = fG;
					if (bB > bR) {
						fMax = fB;
						iMax = bB;
					}
					else {
						fMax = fR;
						iMax = bR;
					}
				}
				else {
					fMin = fR;
					fMax = fB;
					iMax = bB;
				}
			}
			fDelta = fMax - fMin;
			fV = fMax;				// Value (Brightness).
			if (iMax != 0) {			// Make sure its not pure black.
				fS = fDelta / fMax;		// Saturation.
				float ANGLE_TO_UNIT = 1.0f / (6.0f * fDelta);	// Make the Hues between 0.0 to 1.0 instead of 6.0
				if (iMax == bR) {		// between yellow and magenta.
					fH = (fG - fB) * ANGLE_TO_UNIT;
				}
				else if (iMax == bG) {		// between cyan and yellow.
					fH = (2.0f/6.0f) + ( fB - fR ) * ANGLE_TO_UNIT;
				}
				else {				// between magenta and cyan.
					fH = (4.0f/6.0f) + ( fR - fG ) * ANGLE_TO_UNIT;
				}
				// Wrap outlier Hues around the circle.
				if (fH < 0.0f)
					fH += 1.0f;
				if (fH >= 1.0f)
					fH -= 1.0f;
			}
			else {
				// color is pure Black.
				fS = 0;
				fH = 0;	// undefined hue
			}

			// Convert from floats to 8-bit integers.
			int bH = (int)(0.5f + fH * 255.0f);
			int bS = (int)(0.5f + fS * 255.0f);
			int bV = (int)(0.5f + fV * 255.0f);

			// Clip the values to make sure it fits within the 8bits.
			if (bH > 255)
				bH = 255;
			if (bH < 0)
				bH = 0;
			if (bS > 255)
				bS = 255;
			if (bS < 0)
				bS = 0;
			if (bV > 255)
				bV = 255;
			if (bV < 0)
				bV = 0;

			// Set the HSV pixel components.
			uchar *pHSV = (uchar*)(imHSV + y*rowSizeHSV + x*3);
			*(pHSV+0) = bH;		// H component
			*(pHSV+1) = bS;		// S component
			*(pHSV+2) = bV;		// V component
		}
	}
	return imageHSV;
}
IplImage* GlobalHelper::convertImageHSVtoRGB(const IplImage *imageHSV)
{
	float fH, fS, fV;
	float fR, fG, fB;
	const float FLOAT_TO_BYTE = 255.0f;
	const float BYTE_TO_FLOAT = 1.0f / FLOAT_TO_BYTE;

	// Create a blank RGB image
	IplImage *imageRGB = cvCreateImageProxy(cvGetSizeProxy(imageHSV), 8, 3);
	if (!imageRGB || imageHSV->depth != 8 || imageHSV->nChannels != 3) {
		//DebugOut("ERROR in convertImageHSVtoRGB()! Bad input image.",FALSE);
		return 0;
	}

	int h = imageHSV->height;			// Pixel height.
	int w = imageHSV->width;			// Pixel width.
	int rowSizeHSV = imageHSV->widthStep;		// Size of row in bytes, including extra padding.
	char *imHSV = imageHSV->imageData;		// Pointer to the start of the image pixels.
	int rowSizeRGB = imageRGB->widthStep;		// Size of row in bytes, including extra padding.
	char *imRGB = imageRGB->imageData;		// Pointer to the start of the image pixels.
	for (int y=0; y<h; y++) {
		for (int x=0; x<w; x++) {
			// Get the HSV pixel components
			uchar *pHSV = (uchar*)(imHSV + y*rowSizeHSV + x*3);
			int bH = *(uchar*)(pHSV+0);	// H component
			int bS = *(uchar*)(pHSV+1);	// S component
			int bV = *(uchar*)(pHSV+2);	// V component

			// Convert from 8-bit integers to floats
			fH = (float)bH * BYTE_TO_FLOAT;
			fS = (float)bS * BYTE_TO_FLOAT;
			fV = (float)bV * BYTE_TO_FLOAT;

			// Convert from HSV to RGB, using float ranges 0.0 to 1.0
			int iI;
			float fI, fF, p, q, t;

			if( bS == 0 ) {
				// achromatic (grey)
				fR = fG = fB = fV;
			}
			else {
				// If Hue == 1.0, then wrap it around the circle to 0.0
				if (fH >= 1.0f)
					fH = 0.0f;

				fH *= 6.0;			// sector 0 to 5
				fI = floor( fH );		// integer part of h (0,1,2,3,4,5 or 6)
				iI = (int) fH;			//		"		"		"		"
				fF = fH - fI;			// factorial part of h (0 to 1)

				p = fV * ( 1.0f - fS );
				q = fV * ( 1.0f - fS * fF );
				t = fV * ( 1.0f - fS * ( 1.0f - fF ) );

				switch( iI ) {
					case 0:
						fR = fV;
						fG = t;
						fB = p;
						break;
					case 1:
						fR = q;
						fG = fV;
						fB = p;
						break;
					case 2:
						fR = p;
						fG = fV;
						fB = t;
						break;
					case 3:
						fR = p;
						fG = q;
						fB = fV;
						break;
					case 4:
						fR = t;
						fG = p;
						fB = fV;
						break;
					default:		// case 5 (or 6):
						fR = fV;
						fG = p;
						fB = q;
						break;
				}
			}

			// Convert from floats to 8-bit integers
			int bR = (int)(fR * FLOAT_TO_BYTE);
			int bG = (int)(fG * FLOAT_TO_BYTE);
			int bB = (int)(fB * FLOAT_TO_BYTE);

			// Clip the values to make sure it fits within the 8bits.
			if (bR > 255)
				bR = 255;
			if (bR < 0)
				bR = 0;
			if (bG > 255)
				bG = 255;
			if (bG < 0)
				bG = 0;
			if (bB > 255)
				bB = 255;
			if (bB < 0)
				bB = 0;

			// Set the RGB pixel components. NOTE that OpenCV stores RGB pixels in B,G,R order.
			uchar *pRGB = (uchar*)(imRGB + y*rowSizeRGB + x*3);
			*(pRGB+0) = bB;		// B component
			*(pRGB+1) = bG;		// G component
			*(pRGB+2) = bR;		// R component
		}
	}
	return imageRGB;
}
int GlobalHelper::GetHue(int iRGB)
{
	float fR, fG, fB;
	float fH, fS, fV;
	const float FLOAT_TO_BYTE = 255.0f;
	const float BYTE_TO_FLOAT = 1.0f / FLOAT_TO_BYTE;


	int bR = (iRGB >> 16) & 0xFF;	// Blue component
	int bG = (iRGB >> 8) & 0xFF;	// Green component
	int bB = (iRGB) & 0xFF;	// Red component

	// Convert from 8-bit integers to floats.
	fR = bR * BYTE_TO_FLOAT;
	fG = bG * BYTE_TO_FLOAT;
	fB = bB * BYTE_TO_FLOAT;
	// Convert from RGB to HSV, using float ranges 0.0 to 1.0.
	float fDelta;
	float fMin, fMax;
	int iMax;
	// Get the min and max, but use integer comparisons for slight speedup.
	if (bB < bG) {
		if (bB < bR) {
			fMin = fB;
			if (bR > bG) {
				iMax = bR;
				fMax = fR;
			}
			else {
				iMax = bG;
				fMax = fG;
			}
		}
		else {
			fMin = fR;
			fMax = fG;
			iMax = bG;
		}
	}
	else {
		if (bG < bR) {
			fMin = fG;
			if (bB > bR) {
				fMax = fB;
				iMax = bB;
			}
			else {
				fMax = fR;
				iMax = bR;
			}
		}
		else {
			fMin = fR;
			fMax = fB;
			iMax = bB;
		}
	}
	fDelta = fMax - fMin;
	fV = fMax;				// Value (Brightness).
	if (iMax != 0) {			// Make sure its not pure black.
		fS = fDelta / fMax;		// Saturation.
		float ANGLE_TO_UNIT = 1.0f / (6.0f * fDelta);	// Make the Hues between 0.0 to 1.0 instead of 6.0
		if (iMax == bR) {		// between yellow and magenta.
			fH = (fG - fB) * ANGLE_TO_UNIT;
		}
		else if (iMax == bG) {		// between cyan and yellow.
			fH = (2.0f/6.0f) + ( fB - fR ) * ANGLE_TO_UNIT;
		}
		else {				// between magenta and cyan.
			fH = (4.0f/6.0f) + ( fR - fG ) * ANGLE_TO_UNIT;
		}
		// Wrap outlier Hues around the circle.
		if (fH < 0.0f)
			fH += 1.0f;
		if (fH >= 1.0f)
			fH -= 1.0f;
	}
	else {
		// color is pure Black.
		fS = 0;
		fH = 0;	// undefined hue
	}

	// Convert from floats to 8-bit integers.
	int bH = (int)(0.5f + fH * 255.0f);
	int bS = (int)(0.5f + fS * 255.0f);
	int bV = (int)(0.5f + fV * 255.0f);

	// Clip the values to make sure it fits within the 8bits.
	if (bH > 255)
		bH = 255;
	if (bH < 0)
		bH = 0;
	if (bS > 255)
		bS = 255;
	if (bS < 0)
		bS = 0;
	if (bV > 255)
		bV = 255;
	if (bV < 0)
		bV = 0;

	return bH;		// H component
}
void GlobalHelper::BlendScreen(IplImage *pBottom,IplImage *pTop,int iPos)
{
	int iWStep, iJStep;
	int iWStepM, iJStepM;
	int jSeedTop = 0,jSeedBottom = 0;
	int iLength = pTop->width;
	int iTemp;
	if(iPos==-1)
	{
		jSeedBottom = 0;
		jSeedTop = pTop->width/2.0;
		iLength = pTop->width/2.0;
	}
	if(iPos==1)
	{	
		jSeedBottom = pTop->width/2.0;
		jSeedTop = pTop->width*1.5;
		//iLength = pTop->width/2.0;
	}
	for(int i=0;i<pTop->height;i++)
	{
		iWStep = i*pBottom->widthStep + (int)pBottom->imageData;
		iWStepM = i*pTop->widthStep + (int)pTop->imageData;
		for(int j=jSeedBottom;j<iLength;j++)
		{
			iJStep  = (j)*3;
			if(iPos==1)
			{
				iJStepM = (jSeedTop-j);
				if(iJStepM<=0) iJStepM=0;
				if(iJStepM>=pTop->width) iJStepM=pTop->width-1;
				iJStepM*=3;
			}
			else
			{
				iJStepM = (jSeedTop+j)*3;
			}
			*(uchar*)(iWStep +iJStep )   = 255 - (((255 - *(uchar*)(iWStepM +iJStepM))*(255 - *(uchar*)(iWStep +iJStep )))/255);
			*(uchar*)(iWStep +iJStep+1 ) = 255 - (((255 - *(uchar*)(iWStepM +iJStepM+1))*(255 - *(uchar*)(iWStep +iJStep+1 )))/255);
			*(uchar*)(iWStep +iJStep+2 ) = 255 - (((255 - *(uchar*)(iWStepM +iJStepM+2))*(255 - *(uchar*)(iWStep +iJStep+2 )))/255);
		}
	}

}


/*
void GlobalHelper::ScaleImage(IMAGE channel,float **ppfArrX,float **ppfArrY,float min,float max)
{
	float factor;
	float temp;

	if(min ==0 && max==0)
	{
		min = 1000; max = -1000;
		for(int i=0;i<channel->info->nr;i++)
		{
			for(int j=0;j<channel->info->nc;j++)
			{
				if(ppfArrX[i][j]>max)
					max = ppfArrX[i][j];
				if(ppfArrX[i][j]<max)
					min = ppfArrX[i][j];
			}
		}
	}

	if(max>255)
		factor = 1;
	else
		factor = 255/max;
	
	for(int i=0;i<channel->info->nr;i++)
	{
		for(int j=0;j<channel->info->nc;j++)
		{
			temp = factor*sqrt(ppfArrX[i][j]*ppfArrX[i][j] + ppfArrY[i][j]*ppfArrY[i][j]);
			//temp = factor*(ppfArrX[i][j]+ppfArrY[i][j]);
			if(temp>255)
				channel->data[i][j] = 255;
			else
				channel->data[i][j] = temp;
		}
	}

}
void GlobalHelper::ScaleImage(IMAGE channel,float **ppfArrX)
{
	float factor;
	float temp;

	float	min = 1000; float max = -1000;
	for(int i=0;i<channel->info->nr;i++)
	{
		for(int j=0;j<channel->info->nc;j++)
		{
			if(ppfArrX[i][j]>max)
				max = ppfArrX[i][j];
			if(ppfArrX[i][j]<max)
				min = ppfArrX[i][j];
		}
	}

	if(max>255)
		factor = 1;
	else
		factor = 255/max;
	
	for(int i=0;i<channel->info->nr;i++)
	{
		for(int j=0;j<channel->info->nc;j++)
		{
			temp = factor*ppfArrX[i][j];
			//temp = factor*(ppfArrX[i][j]+ppfArrY[i][j]);
			if(temp>255)
				channel->data[i][j] = 255;
			else
				channel->data[i][j] = temp;
			if(ppfArrX[i][j]<0)
				channel->data[i][j] = 128;
		}
	}

}
*/
