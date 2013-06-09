#include "stdafx.h"
#include "HDR.h"
#include "GlobalHelper.h"
 

extern "C" __declspec(dllexport) void HDR(char *szPath, int bPreviewMode, int*iErrCode)
{
	GlobalHelper g_GH;
	g_GH.Init(szPath,iErrCode);
	if(*iErrCode >0)
		return;

	//g_GH.DebugOut("HDR: Begin",TRUE);
	
	IplImage *pImgSource;
	int iRet = g_GH.load_RGB(szPath,&pImgSource,bPreviewMode);
	if(!iRet)
	{
		//g_GH.DebugOut("HDR: LoadImage failed",FALSE);
		*iErrCode = 1;
		return;
	}


	clsHDR HDR;
	HDR.HDRMain(&pImgSource,bPreviewMode,iErrCode,g_GH);

	iRet = g_GH.save_RGB(szPath,pImgSource);
	if(!iRet)
	{
		//g_GH.DebugOut("HDR: save_RGB failed",FALSE);
		*iErrCode = 1;
		return;
	}

	g_GH.cvReleaseImageProxy(&pImgSource);
	//g_GH.DebugOut("HDR: End",TRUE);

}
clsHDR::clsHDR()
{
	
}
clsHDR::~clsHDR()
{
}
void clsHDR::HDRMain(IplImage **pImgSrc, int bPreviewMode, int*iErrCode,GlobalHelper &g_GH)
{
	pImgSource = *pImgSrc;
	if(!pImgSource)
	{
		//g_GH.DebugOut("HDRMain: Source Image NULL",FALSE);
		*iErrCode = 1;
		return;
	}

	iNC = pImgSource->width;
	iNR = pImgSource->height;
	
	ApplyCanvas(g_GH);
	
	*pImgSrc = pImgSource;
	g_GH.cvReleaseImageProxy(&pImgDest);

}

void clsHDR::ApplyCanvas(GlobalHelper &g_GH)
{
	
	pImgDest = g_GH.convertImageRGBtoHSV(pImgSource);


	int iWStep, iJStep;
	int tmpS,tmpD;
	for (int i=0;i<iNR;i++)
	{
		iWStep = i*pImgSource->widthStep;
		for(int j=0;j<iNC;j++)
		{
			iJStep = j*3;
			tmpS = *(uchar*)(pImgDest->imageData + iWStep + iJStep + 1 )*4;
			if(tmpS>255)
				tmpS=255;
			*(uchar*)(pImgDest->imageData + iWStep + iJStep + 1 ) = tmpS;
		}
	}
	g_GH.cvReleaseImageProxy(&pImgSource);
	pImgSource = g_GH.convertImageHSVtoRGB(pImgDest);

}
