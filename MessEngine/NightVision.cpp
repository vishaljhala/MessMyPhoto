#include "StdAfx.h"
#include "NightVision.h"
#include "GlobalHelper.h"

extern char g_pcszPath[];

extern "C" __declspec(dllexport) void NightVision(char *szPath, int bIsBorder, int*iErrCode)
{
	DWORD dwStart = GetTickCount();
	SYSTEMTIME stStart;
	GetLocalTime(&stStart);

	GlobalHelper g_GH;
	g_GH.Init(szPath,iErrCode);
	if(*iErrCode >0)
		return;

	//g_GH.DebugOut("NightVision: Begin",TRUE);
	char szParam[1024];
	szParam[0]=0;
	sprintf(szParam,"bPreviewMode=%d",false);
	//g_GH.DebugOut(szParam,FALSE);


	IplImage *pImgSource;
	int iRet = g_GH.load_RGB(szPath,&pImgSource,false);
	if(!iRet)
	{
		//g_GH.DebugOut("NightVision: LoadImage failed",FALSE);
		*iErrCode = 1;
		return;
	}
	clsNightVision gs;
	gs.NightVisionMain(&pImgSource,bIsBorder,iErrCode,g_GH);

	iRet = g_GH.save_RGB(szPath,pImgSource);

	if(!iRet)
	{
		//g_GH.DebugOut("NightVision: save_RGB failed",FALSE);
		*iErrCode = 1;
		return;
	}

	g_GH.cvReleaseImageProxy(&pImgSource);

	//g_GH.DebugOut("NightVision: End",TRUE);

	*iErrCode = 0;

	DWORD dwEnd = GetTickCount();
	char pszTimec[1024];pszTimec[0] = 0;

	sprintf(pszTimec,"NightVision,%d-%02d-%02d %02d:%02d:%02d,%ld\0",stStart.wYear,stStart.wMonth,stStart.wDay,stStart.wHour,
		stStart.wMinute, stStart.wSecond, dwEnd - dwStart);

	//sprintf(pszTimec,"NightVision,%d-%02d-%02d %02d:%02d:%02d,%ld\0",stStart.wYear,stStart.wMonth,stStart.wDay,dwEnd - dwStart);

	g_GH.DebugOut(pszTimec);

}
clsNightVision::clsNightVision()
{
}
clsNightVision::~clsNightVision()
{
}
void clsNightVision::NightVisionMain(IplImage **pImgSrc,int bIsBorder, int*iErrCode,GlobalHelper &g_GH)
{
	pImgSource = *pImgSrc;

	if(!pImgSource)
	{
		//g_GH.DebugOut("NightVision: Source Image NULL",FALSE);
		*iErrCode = 1;
		return;
	}

	iNC = pImgSource->width;
	iNR = pImgSource->height;
	
	/*pImgDest = g_GH.cvCreateImageProxy(cvSize(pImgSource->width,pImgSource->height),pImgSource->depth,3);
	pImgDest2 = g_GH.cvCreateImageProxy(cvSize(pImgSource->width,pImgSource->height),pImgSource->depth,3);
	if(!pImgDest)
	{
		g_GH.DebugOut("NightVision: CreateImage failed",FALSE);
		*iErrCode = 1;
		return;
	}*/
	//g_GH.cvCvtColorProxy(pImgSource,pImgDest,CV_BGR2Lab);
	Greenize(g_GH);
	SniperView(g_GH);
	if(bIsBorder)
		ApplyBorder(g_GH);
	//g_GH.cvCvtColorProxy(pImgDest,pImgSource,CV_Lab2BGR);

	//*pImgSrc = pImgDest;
	//g_GH.cvReleaseImageProxy(&pImgSource);

}
void clsNightVision::Greenize(GlobalHelper &g_GH)
{
	int iWStep, iJStep;
	int tmp;
	for(int i=0;i<iNR;i++)
	{
		iWStep = i*pImgSource->widthStep + (int) pImgSource->imageData;
		for(int j=0;j<iNC;j++)
		{
			iJStep = iWStep + j*3;

			*(uchar*)( iJStep + 1) =(*(uchar*)(iJStep + 2) + 
			*(uchar*)( iJStep + 0) +
			*(uchar*)(iJStep + 1) ) / 3.0;

			 *(uchar*)(iJStep + 2) = 
				*(uchar*)(iJStep + 0) = 0;

		}
	}
}
void clsNightVision::SniperView(GlobalHelper &g_GH)
{
	char buff[1024];
	buff[0] = NULL;
	strcpy(buff,g_pcszPath);

	strcat(buff,"NightVision");

	IplImage *imgrgb;
	int iRet = g_GH.load_RGB(buff,&imgrgb,FALSE);
	if(!iRet)
	{
		//g_GH.DebugOut("clsNightVision::Sniperview - Failed to load canvas",FALSE);
		//g_GH.DebugOut(buff,FALSE);
		return;
	}
	int iSmall = iNR;
	int iStartNR,iStartNC;
	if(iNC<iNR) 
	{
		iSmall = iNC;
		iStartNR = (iNR-iNC)/2.0;
		iStartNC = 0;
	}
	else
	{
		iStartNC = (iNC-iNR)/2.0;
		iStartNR = 0;
	}

	IplImage *imgScaled;
	imgScaled = g_GH.cvCreateImageProxy(cvSize(iSmall,iSmall),imgrgb->depth,3);
	g_GH.cvResizeProxy(imgrgb,imgScaled,1);


	int iWStep, iJStep;
	int iWStep1, iJStep1;
	int tmp;
	int iNCEnd = iStartNC +iSmall;
	int iNREnd = iStartNR +iSmall;
	for(int i=iStartNR;i<iNREnd;i++)
	{
		iWStep = i*pImgSource->widthStep + (int) pImgSource->imageData;
		iWStep1 = (i-iStartNR)*imgScaled->widthStep + (int)imgScaled->imageData ;
		for(int j=iStartNC;j<iNCEnd;j++)
		{
			iJStep = iWStep + j*3;
			iJStep1 = iWStep1+ (j-iStartNC)*3;

			if(*(uchar*)(   iJStep1 ) <245)
			{
				//*(uchar*)(pImgSource->imageData + iWStep + iJStep + 1) = *(uchar*)(pImgSource->imageData + iWStep + iJStep + 1)*0.6 + 102;//*(uchar*)(pImgSource->imageData + iWStep + iJStep + 2) = 
				//*(uchar*)(pImgSource->imageData + iWStep + iJStep + 0) = 
				//*(uchar*)(pImgSource->imageData + iWStep + iJStep + 1) = 
				*(uchar*)(iJStep + 0) = 
					*(uchar*)(iJStep + 0)*0.6 + *(uchar*)(  iJStep1 + 0)*0.4;
				*(uchar*)(iJStep + 1) = 
					*(uchar*)(iJStep + 1)*0.6 + *(uchar*)(  iJStep1 + 1)*0.4;
				*(uchar*)(iJStep + 2) = 
					*(uchar*)(iJStep + 2)*0.6 + *(uchar*)(  iJStep1 + 2)*0.4;

			}
		}
	}



	g_GH.cvReleaseImageProxy(&imgrgb);
	g_GH.cvReleaseImageProxy(&imgScaled);

}
void clsNightVision::ApplyBorder(GlobalHelper &g_GH)
{
	char buff[1024];
	buff[0] = NULL;
	strcpy(buff,g_pcszPath);

	strcat(buff,"nvborder");

	IplImage *imgrgb,*imgtmp;
	int iRet = g_GH.load_RGB(buff,&imgrgb,FALSE);
	if(!iRet)
	{
		//g_GH.DebugOut("clsOldPhoto::ApplyBorder - Failed to load canvas",FALSE);
		//g_GH.DebugOut(buff,FALSE);
		return;
	}
	imgtmp = g_GH.cvCreateImageProxy(cvSize(iNC,iNR),pImgSource->depth,3);

	g_GH.cvResizeProxy(imgrgb,imgtmp,1);

	int iWStep, iJStep;
	for (int i=0;i<iNR;i++)
	{
		iWStep = i*pImgSource->widthStep;
		for(int j=0;j<iNC;j++)
		{
			iJStep = j*3;
			if(*(uchar*)(imgtmp->imageData + iWStep + iJStep + 0)<64)
				*(uchar*)(pImgSource->imageData + iWStep + iJStep + 0) = *(uchar*)(pImgSource->imageData + iWStep + iJStep + 1)
				= *(uchar*)(pImgSource->imageData + iWStep + iJStep + 2) = 0;
		}
	}

	g_GH.cvReleaseImageProxy(&imgtmp);
	g_GH.cvReleaseImageProxy(&imgrgb);

}

