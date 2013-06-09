#include "stdafx.h"
#include "PainterlySketch.h"
#include "PerfectSketch.h"
#include "CartoonEffect.h"
#include "GlobalHelper.h"
#define PI 3.1415926535


extern "C" __declspec(dllexport) void PainterlySketch(char *szPath, int iSketchType, int iDetails, int*iErrCode)

{
	DWORD dwStart = GetTickCount();
	SYSTEMTIME stStart;
	GetLocalTime(&stStart);

	GlobalHelper g_GH;
	g_GH.Init(szPath,iErrCode);
	if(*iErrCode >0)
		return;

	//g_GH.DebugOut("DoG: Begin",TRUE);

	IplImage *pImgSource;
	int iRet = g_GH.load_RGB(szPath,&pImgSource,false);
	if(!iRet)
	{
		//g_GH.DebugOut("DoG: LoadImage failed",FALSE);
		*iErrCode = 1;
		return;
	}

	if(iSketchType == 0)
	{
		clsPerfectSketch sktch;
		sktch.SketchFilter(&pImgSource, iDetails,
			 iErrCode,g_GH);
	}
	if(iSketchType ==1)
	{
	
		clsPainterlySketch dog;
		dog.SketchMain(&pImgSource, 
			 iErrCode,g_GH);
	}

	iRet = g_GH.save_RGB(szPath,pImgSource);
	if(!iRet)
	{
		//g_GH.DebugOut("DoG: save_RGB failed",FALSE);
		*iErrCode = 1;
		return;
	}

	g_GH.cvReleaseImageProxy(&pImgSource);

	//g_GH.DebugOut("DoG: End",TRUE);

	*iErrCode = 0;

	DWORD dwEnd = GetTickCount();
	char pszTimec[1024];pszTimec[0] = 0;
	sprintf(pszTimec,"PainterlySketch,%d-%02d-%02d %02d:%02d:%02d,%ld\0",stStart.wYear,stStart.wMonth,stStart.wDay,stStart.wHour,
		stStart.wMinute, stStart.wSecond, dwEnd - dwStart);
//	sprintf(pszTimec,"PainterlySketch,%d-%02d-%02d %02d:%02d:%02d,%ld\0",stStart.wYear,stStart.wMonth,stStart.wDay,dwEnd - dwStart);

	g_GH.DebugOut(pszTimec);

	
}

clsPainterlySketch::clsPainterlySketch()
{
}
clsPainterlySketch::~clsPainterlySketch()
{
}
void clsPainterlySketch::SketchMain(IplImage **pImgSrc, 
	  int*iErrCode,GlobalHelper &g_GH)
{
	pImgSource = *pImgSrc;

	if(!pImgSource)
	{
		//g_GH.DebugOut("DoG: Source Image NULL",FALSE);
		*iErrCode = 1;
		return;
	}

	iNC = pImgSource->width;
	iNR = pImgSource->height;
	//Decide Params
	int iBig = iNC;
	if(iNR>iNC) iBig = iNR;
	
	int iFilSmall,iFilLarge;
	if(iBig<=800)
	{	iFilSmall = 7; iFilLarge = 11;}
	if(iBig>800 && iBig<=1800)
	{	iFilSmall = 9; iFilLarge = 15;}
	if(iBig>1800 && iBig<=3200)
	{	iFilSmall = 13; iFilLarge = 23;}
	if(iBig>3200)
	{	iFilSmall = 15; iFilLarge = 29;}

	//Create Greyscale
	pImgGrey = g_GH.cvCreateImageProxy(cvSize(pImgSource->width,pImgSource->height),IPL_DEPTH_8U,1);
	g_GH.cvCvtColorProxy(pImgSource,pImgGrey,CV_BGR2GRAY);

	//Smooth image with BF before DOG
	pImgDest = g_GH.cvCreateImageProxy(cvSize(pImgSource->width,pImgSource->height),IPL_DEPTH_8U,3);
	g_GH.cvSmoothProxy(pImgSource,pImgDest,CV_BILATERAL,11,0,16.5,300);
	g_GH.cvSmoothProxy(pImgDest,pImgSource,CV_BILATERAL,11,0,16.5,300);
	g_GH.cvCvtColorProxy(pImgSource,pImgGrey,CV_BGR2GRAY);

	//Apply DoG
	g_GH.cvReleaseImageProxy(&pImgDest);
	pImgDest = g_GH.cvCreateImageProxy(cvSize(iNC,iNR),IPL_DEPTH_8U,1);
	pImgTemp = g_GH.cvCreateImageProxy(cvSize(iNC,iNR),IPL_DEPTH_8U,1);

	g_GH.cvSmoothProxy(pImgGrey,pImgDest,CV_GAUSSIAN,iFilSmall,iFilSmall,0,0);
	g_GH.cvSmoothProxy(pImgGrey,pImgTemp,CV_GAUSSIAN,iFilLarge,iFilLarge,0,0);
	
	Diff(g_GH);

	//Dialate erode to smooth
	g_GH.cvErodeProxy(pImgDest,pImgTemp,0,1);
	g_GH.cvDilateProxy(pImgTemp,pImgDest,0,1);


	g_GH.cvReleaseImageProxy(&pImgTemp);
	g_GH.cvReleaseImageProxy(&pImgGrey);
	g_GH.cvReleaseImageProxy(&pImgSource);

	*pImgSrc = pImgDest;

}
void clsPainterlySketch::Diff(GlobalHelper &g_GH)
{
	int iw,iw1;
	for (int i=0;i<iNR;i++)
	{
		iw = i*pImgDest->widthStep + (int)pImgDest->imageData;
		iw1 = (int)pImgTemp->imageData + i*pImgTemp->widthStep;
		for(int j=0;j<iNC;j++)
		{
			if( *(UCHAR*)(iw +j) - *(UCHAR*)( iw1 +j) < 0 )
				*(UCHAR*)( iw +j) = 0;
			else
				*(UCHAR*)(iw +j) = 255;

		}
	}

}

