#include "StdAfx.h"
#include "GreyScale.h"
#include "GlobalHelper.h"


extern "C" __declspec(dllexport) void GreyScale(char *szPath, int bPreviewMode, int*iErrCode)
{
	DWORD dwStart = GetTickCount();
	SYSTEMTIME stStart;
	GetLocalTime(&stStart);

	GlobalHelper g_GH;
	g_GH.Init(szPath,iErrCode);
	if(*iErrCode >0)
		return;

	//g_GH.DebugOut("GreyScale: Begin",TRUE);
	char szParam[1024];
	szParam[0]=0;
	sprintf(szParam,"bPreviewMode=%d",bPreviewMode);
	//g_GH.DebugOut(szParam,FALSE);


	IplImage *pImgSource;
	int iRet = g_GH.load_RGB(szPath,&pImgSource,bPreviewMode);
	if(!iRet)
	{
		//g_GH.DebugOut("GreyScale: LoadImage failed",FALSE);
		*iErrCode = 1;
		return;
	}
	clsGreyScale gs;
	gs.GreyScaleMain(&pImgSource,iErrCode,g_GH);

	iRet = g_GH.save_RGB(szPath,pImgSource);

	if(!iRet)
	{
		//g_GH.DebugOut("GreyScale: save_RGB failed",FALSE);
		*iErrCode = 1;
		return;
	}

	g_GH.cvReleaseImageProxy(&pImgSource);

	//g_GH.DebugOut("GreyScale: End",TRUE);

	*iErrCode = 0;

	DWORD dwEnd = GetTickCount();
	char pszTimec[1024];pszTimec[0] = 0;
	sprintf(pszTimec,"GrayScale,%d-%02d-%02d %02d:%02d:%02d,%ld\0",stStart.wYear,stStart.wMonth,stStart.wDay,stStart.wHour,
		stStart.wMinute, stStart.wSecond, dwEnd - dwStart);
	
	
	//sprintf(pszTimec,"GrayScale,%d-%02d-%02d %02d:%02d:%02d,%ld\0",stStart.wYear,stStart.wMonth,stStart.wDay,dwEnd - dwStart);

	g_GH.DebugOut(pszTimec);

}
clsGreyScale::clsGreyScale()
{
}
clsGreyScale::~clsGreyScale()
{
}
void clsGreyScale::GreyScaleMain(IplImage **pImgSrc,int*iErrCode,GlobalHelper &g_GH)
{
	pImgSource = *pImgSrc;

	if(!pImgSource)
	{
		//g_GH.DebugOut("GreyScale: Source Image NULL",FALSE);
		*iErrCode = 1;
		return;
	}

	iNC = pImgSource->width;
	iNR = pImgSource->height;
	
	pImgDest = g_GH.cvCreateImageProxy(cvSize(pImgSource->width,pImgSource->height),pImgSource->depth,3);
	if(!pImgDest)
	{
		//g_GH.DebugOut("GreyScale: CreateImage failed",FALSE);
		*iErrCode = 1;
		return;
	}
	g_GH.cvCvtColorProxy(pImgSource,pImgDest,CV_BGR2Lab);
	GreyScale(g_GH);
	//g_GH.cvCvtColorProxy(pImgDest,pImgSource,CV_Lab2BGR);

	*pImgSrc = pImgDest;
	g_GH.cvReleaseImageProxy(&pImgSource);

}
void clsGreyScale::GreyScale(GlobalHelper &g_GH)
{
	int iL,iU,iV;
	int iWStep, iJStep;
	int tmp;
	for(int i=0;i<iNR;i++)
	{
		iWStep =  i*pImgSource->widthStep + (int)pImgDest->imageData;
		for(int j=0;j<iNC;j++)
		{
			iJStep = iWStep + j*3;
			*(uchar*)(  iJStep + 1) = *(uchar*)(  iJStep + 2) = 
				*(uchar*)(iJStep + 0);
		}
	}

}


