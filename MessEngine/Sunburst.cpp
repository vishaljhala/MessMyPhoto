#include "stdafx.h"
#include "SunBurst.h"
#include "GlobalHelper.h"
 
extern char g_pcszPath[];

extern "C" __declspec(dllexport) void SunBurst(char *szPath, int bPreviewMode, int*iErrCode)
{
	GlobalHelper g_GH;
	g_GH.Init(szPath,iErrCode);
	if(*iErrCode >0)
		return;

	//g_GH.DebugOut("SunBurst: Begin",TRUE);
	
	IplImage *pImgSource;
	int iRet = g_GH.load_RGB(szPath,&pImgSource,bPreviewMode);
	if(!iRet)
	{
		//g_GH.DebugOut("SunBurst: LoadImage failed",FALSE);
		*iErrCode = 1;
		return;
	}

	clsSunBurst SunBurst;
	SunBurst.SunBurstMain(&pImgSource,bPreviewMode,iErrCode,g_GH);

	iRet = g_GH.save_RGB(szPath,pImgSource);
	if(!iRet)
	{
		//g_GH.DebugOut("SunBurst: save_RGB failed",FALSE);
		*iErrCode = 1;
		return;
	}

	g_GH.cvReleaseImageProxy(&pImgSource);
	//g_GH.DebugOut("SunBurst: End",TRUE);

}
clsSunBurst::clsSunBurst()
{
	
}
clsSunBurst::~clsSunBurst()
{
}
void clsSunBurst::SunBurstMain(IplImage **pImgSrc, int bPreviewMode, int*iErrCode,GlobalHelper &g_GH)
{
	pImgSource = *pImgSrc;
	if(!pImgSource)
	{
		//g_GH.DebugOut("SunBurstMain: Source Image NULL",FALSE);
		*iErrCode = 1;
		return;
	}

	iNC = pImgSource->width;
	iNR = pImgSource->height;
	
	ApplyCanvas(g_GH);


}

void clsSunBurst::ApplyCanvas(GlobalHelper &g_GH)
{
	
	char buff[1024];
	buff[0] = NULL;
	strcpy(buff,g_pcszPath);

	strcat(buff,"SunBurst");

	IplImage *imgrgb;
	int iRet = g_GH.load_RGB(buff,&imgrgb,FALSE);
	if(!iRet)
	{
		//g_GH.DebugOut("clsSunBurst::ApplyCanvas - Failed to load canvas",FALSE);
		//g_GH.DebugOut(buff,FALSE);
		return;
	}

	pImgDest = g_GH.cvCreateImageProxy(cvSize(iNC,iNR),pImgSource->depth,pImgSource->nChannels);
	g_GH.cvResizeProxy(imgrgb,pImgDest,1);

	int iWStep, iJStep;
	int tmp;
	for (int i=0;i<iNR;i++)
	{
		iWStep = i*pImgSource->widthStep;
		for(int j=0;j<iNC;j++)
		{
			iJStep = j*3;
			*(uchar*)(pImgSource->imageData + iWStep + iJStep + 0) = 
			*(uchar*)(pImgSource->imageData + iWStep + iJStep + 0)*0.8+
			*(uchar*)(pImgDest->imageData + iWStep + iJStep + 0)*0.2;

			*(uchar*)(pImgSource->imageData + iWStep + iJStep + 1) = 
			*(uchar*)(pImgSource->imageData + iWStep + iJStep + 1)*0.8+
			*(uchar*)(pImgDest->imageData + iWStep + iJStep + 1)*0.2;

			*(uchar*)(pImgSource->imageData + iWStep + iJStep + 2) = 
			*(uchar*)(pImgSource->imageData + iWStep + iJStep + 2)*0.8+
			*(uchar*)(pImgDest->imageData + iWStep + iJStep + 2)*0.2;
		}
	}
	


	g_GH.cvReleaseImageProxy(&imgrgb);
	g_GH.cvReleaseImageProxy(&pImgDest);

}
