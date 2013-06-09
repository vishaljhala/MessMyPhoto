#include "stdafx.h"
#include "Patronize.h"
#include "GlobalHelper.h"
 
extern char g_pcszPath[];

extern "C" __declspec(dllexport) void Patronize(char *szPath, char *szCountry, int iAlphaBlend,  int iStyle , int*iErrCode)
{
	DWORD dwStart = GetTickCount();
	SYSTEMTIME stStart;
	GetLocalTime(&stStart);

	GlobalHelper g_GH;
	g_GH.Init(szPath,iErrCode);
	if(*iErrCode >0)
		return;

	//g_GH.DebugOut("Patronize: Begin",TRUE);
	
	IplImage *pImgSource;
	int iRet = g_GH.load_RGB(szPath,&pImgSource,false);
	if(!iRet)
	{
		//g_GH.DebugOut("Patronize: LoadImage failed",FALSE);
		*iErrCode = 1;
		return;
	}

	clsPatronize Patronize;
	Patronize.PatronizeMain(&pImgSource,szCountry, iAlphaBlend, iStyle, iErrCode,g_GH);

	iRet = g_GH.save_RGB(szPath,pImgSource);
	if(!iRet)
	{
		//g_GH.DebugOut("Patronize: save_RGB failed",FALSE);
		*iErrCode = 1;
		return;
	}

	g_GH.cvReleaseImageProxy(&pImgSource);

	DWORD dwEnd = GetTickCount();
	char pszTimec[1024];pszTimec[0] = 0;
	sprintf(pszTimec,"Nationalize,%d-%02d-%02d %02d:%02d:%02d,%ld\0",stStart.wYear,stStart.wMonth,stStart.wDay,stStart.wHour,
		stStart.wMinute, stStart.wSecond, dwEnd - dwStart);
	//sprintf(pszTimec,"PerfectSketch,%d-%02d-%02d,%ld\0",stStart.wYear,stStart.wMonth,stStart.wDay,dwEnd - dwStart);

	g_GH.DebugOut(pszTimec);

}
clsPatronize::clsPatronize()
{
	
}
clsPatronize::~clsPatronize()
{
}
void clsPatronize::PatronizeMain(IplImage **pImgSrc, char *szCountry, int iAlphaBlend,  int iStyle , int*iErrCode,GlobalHelper &g_GH)
{
	pImgSource = *pImgSrc;
	if(!pImgSource)
	{
		//g_GH.DebugOut("PatronizeMain: Source Image NULL",FALSE);
		*iErrCode = 1;
		return;
	}

	iNC = pImgSource->width;
	iNR = pImgSource->height;
	
	ApplyCanvas(szCountry,iAlphaBlend,iStyle,g_GH);


}

void clsPatronize::ApplyCanvas(char *szCountry, int iAlphaBlend,  int iStyle,GlobalHelper &g_GH)
{
	
	char buff[1024];
	buff[0] = NULL;
	strcpy(buff,g_pcszPath);

	strcat(buff,szCountry);

	if(iAlphaBlend<20) iAlphaBlend = 20;
	if(iAlphaBlend>50) iAlphaBlend = 50;
	float fBlend = iAlphaBlend/100.0;
	float fBlendOrig = 1-fBlend;

	IplImage *imgrgb;
	int iRet = g_GH.load_RGB(buff,&imgrgb,FALSE);
	if(!iRet)
	{
		//g_GH.DebugOut("clsPatronize::ApplyCanvas - Failed to load canvas",FALSE);
		//g_GH.DebugOut(buff,FALSE);
		return;
	}

	pImgDest = g_GH.cvCreateImageProxy(cvSize(iNC,iNR),pImgSource->depth,pImgSource->nChannels);
	g_GH.cvResizeProxy(imgrgb,pImgDest,1);

	int iWStep, iJStep, iWStepDest;
	int tmp;
	if(iStyle ==0)
	for (int i=0;i<iNR;i++)
	{
		iWStep = i*pImgSource->widthStep+(int) pImgSource->imageData;
		iWStepDest = i*pImgSource->widthStep+(int) pImgDest->imageData;
		for(int j=0;j<iNC;j++)
		{
			iJStep = j*3;
			*(uchar*)( iWStep + iJStep + 0) = 
			*(uchar*)(iWStep + iJStep + 0)*fBlendOrig+
			*(uchar*)(iWStepDest + iJStep + 0)*fBlend;

			*(uchar*)(iWStep + iJStep + 1) = 
			*(uchar*)(iWStep + iJStep + 1)*fBlendOrig+
			*(uchar*)(iWStepDest + iJStep + 1)*fBlend;

			*(uchar*)(iWStep + iJStep + 2) = 
			*(uchar*)(iWStep + iJStep + 2)*fBlendOrig+
			*(uchar*)(iWStepDest + iJStep + 2)*fBlend;
		}
	}
	

	if(iStyle==1)
	{
		int iCanvasNR=0,iCanvasNC=0;		
		int iWStepPatch, iJStepPatch,tmp1;
		for (int i=0;i<iNR;i++)
		{
			iWStep = i*pImgSource->widthStep + (int)pImgSource->imageData ;
			iWStepPatch = iCanvasNR*imgrgb->widthStep + (int)imgrgb->imageData;

			for(int j=0;j<iNC;j++)
			{
				iJStep = j*3;
				iJStepPatch = iCanvasNC*3;

				*(uchar*)(iWStep + iJStep + 0) = 
				*(uchar*)(iWStep + iJStep + 0)*fBlendOrig+
				*(uchar*)( iWStepPatch + iJStepPatch + 0)*fBlend;

				*(uchar*)(iWStep + iJStep + 1) = 
				*(uchar*)(iWStep + iJStep + 1)*fBlendOrig+
				*(uchar*)(iWStepPatch + iJStepPatch + 1)*fBlend;

				*(uchar*)(iWStep + iJStep + 2) = 
				*(uchar*)(iWStep + iJStep + 2)*fBlendOrig+
				*(uchar*)(iWStepPatch + iJStepPatch + 2)*fBlend;

				iCanvasNC++;
				if(iCanvasNC>=imgrgb->width)
					iCanvasNC = 0;
			}
			iCanvasNC=0;
			iCanvasNR++;
			if(iCanvasNR>=imgrgb->height)
				iCanvasNR = 0;
		}

	}

	g_GH.cvReleaseImageProxy(&imgrgb);
	g_GH.cvReleaseImageProxy(&pImgDest);

}
