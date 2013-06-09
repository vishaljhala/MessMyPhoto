#include "StdAfx.h"
#include "RotateInvert.h"
#include "GlobalHelper.h"


extern "C" __declspec(dllexport) void RotateInvert(char *szPath,int iAction, int*iErrCode)
{
	GlobalHelper g_GH;
	g_GH.Init(szPath,iErrCode);
	if(*iErrCode >0)
		return;

	//g_GH.DebugOut("RotateInvert: Begin",TRUE);
	char szParam[1024];
	szParam[0]=0;
	sprintf(szParam,"iAction=%d",iAction);
	//g_GH.DebugOut(szParam,FALSE);

	IplImage *pImgSource;
	int iRet = g_GH.load_RGB(szPath,&pImgSource,FALSE);
	if(!iRet)
	{
		//g_GH.DebugOut("RotateInvert: LoadImage failed",FALSE);
		*iErrCode = 1;
		return;
	}
	clsRotateInvert ri;
	if(iAction ==1) //Rotate Left
	{
		ri.Rotate(&pImgSource,1,g_GH);
	}
	else if(iAction == 2) //Rotate Right
	{
		ri.Rotate(&pImgSource,2,g_GH);
	}
	else if(iAction == 3) //Invert Colors
	{
		ri.Invert(&pImgSource,g_GH);
	}

	iRet = g_GH.save_RGB(szPath,pImgSource);

	if(!iRet)
	{
		//g_GH.DebugOut("RotateInvert: save_RGB failed",FALSE);
		*iErrCode = 1;
		return;
	}

	g_GH.cvReleaseImageProxy(&pImgSource);

	//g_GH.DebugOut("RotateInvert: End",TRUE);

	*iErrCode = 0;
}
clsRotateInvert::clsRotateInvert()
{
}
clsRotateInvert::~clsRotateInvert()
{
}
void clsRotateInvert::Rotate(IplImage **pImgSrc,int iAngle,GlobalHelper &g_GH)
{
	pImgSource = *pImgSrc;

	iNC = pImgSource->width;
	iNR = pImgSource->height;
	
	pImgDest = g_GH.cvCreateImageProxy(cvSize(pImgSource->height,pImgSource->width),pImgSource->depth,3);
	if(!pImgDest)
	{
		//g_GH.DebugOut("Rotate: CreateImage failed",FALSE);
		return;
	}

	g_GH.cvTransposeProxy(pImgSource,pImgDest);
	if(iAngle==1)
	{
		IplImage *pImgTemp = g_GH.cvCreateImageProxy(cvSize(pImgSource->height,pImgSource->width),pImgSource->depth,3);
		g_GH.cvFlipProxy(pImgDest,pImgTemp,0);
		*pImgSrc = pImgTemp;
		g_GH.cvReleaseImageProxy(&pImgSource);
		g_GH.cvReleaseImageProxy(&pImgDest);
		return;
	}
	if(iAngle==2)
	{
		IplImage *pImgTemp = g_GH.cvCreateImageProxy(cvSize(pImgSource->height,pImgSource->width),pImgSource->depth,3);
		g_GH.cvFlipProxy(pImgDest,pImgTemp,1);
		*pImgSrc = pImgTemp;
		g_GH.cvReleaseImageProxy(&pImgSource);
		g_GH.cvReleaseImageProxy(&pImgDest);
		return;
	}

}
void clsRotateInvert::Invert(IplImage **pImgSrc,GlobalHelper &g_GH)
{
	pImgSource = *pImgSrc;

	iNC = pImgSource->width;
	iNR = pImgSource->height;
	int iWStep, iJStep;
	
	for(int i=0;i<iNR;i++)
	{
		iWStep = i*pImgSource->widthStep;

		for(int j=0;j<iNC;j++)
		{
			iJStep = j*3;
			*(uchar*)(pImgSource->imageData + iWStep + iJStep + 2) = 255 - *(uchar*)(pImgSource->imageData + iWStep + iJStep + 2);
			*(uchar*)(pImgSource->imageData + iWStep + iJStep + 1) = 255 - *(uchar*)(pImgSource->imageData + iWStep + iJStep + 1);
			*(uchar*)(pImgSource->imageData + iWStep + iJStep + 0) = 255 - *(uchar*)(pImgSource->imageData + iWStep + iJStep + 0);

		}
	}


}


