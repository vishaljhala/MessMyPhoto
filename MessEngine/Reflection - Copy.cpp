#include "stdafx.h"
#include "Reflection.h"
#include "GlobalHelper.h"
 

extern "C" __declspec(dllexport) void Reflection(char *szPath, int bPreviewMode, int*iErrCode)
{
	GlobalHelper g_GH;
	g_GH.Init(szPath,iErrCode);
	if(*iErrCode >0)
		return;

	//g_GH.DebugOut("Reflection: Begin",TRUE);
	
	IplImage *pImgSource;
	int iRet = g_GH.load_RGB(szPath,&pImgSource,bPreviewMode);
	if(!iRet)
	{
		//g_GH.DebugOut("Reflection: LoadImage failed",FALSE);
		*iErrCode = 1;
		return;
	}

	clsReflection Reflection;
	Reflection.ReflectionMain(&pImgSource,bPreviewMode,iErrCode,g_GH);

	iRet = g_GH.save_RGB(szPath,pImgSource);
	if(!iRet)
	{
		//g_GH.DebugOut("Reflection: save_RGB failed",FALSE);
		*iErrCode = 1;
		return;
	}

	g_GH.cvReleaseImageProxy(&pImgSource);
	//g_GH.DebugOut("Reflection: End",TRUE);

}
clsReflection::clsReflection()
{
	
}
clsReflection::~clsReflection()
{
}
void clsReflection::ReflectionMain(IplImage **pImgSrc, int bPreviewMode, int*iErrCode,GlobalHelper &g_GH)
{
	pImgSource = *pImgSrc;
	if(!pImgSource)
	{
		//g_GH.DebugOut("ReflectionMain: Source Image NULL",FALSE);
		*iErrCode = 1;
		return;
	}

	iNC = pImgSource->width;
	iNR = pImgSource->height;
	
	ApplyCanvas(g_GH);
	
	*pImgSrc = pImgDest;

	g_GH.cvReleaseImageProxy(&pImgSource);


}

void clsReflection::ApplyCanvas(GlobalHelper &g_GH)
{
	
	pImgDest = g_GH.cvCreateImageProxy(cvSize(iNC,iNR+255),pImgSource->depth,pImgSource->nChannels);

	int iWStep, iJStep;
	int iWStepD, iJStepD;

	for (int i=0;i<iNR;i++)
	{
		iWStep = i*pImgSource->widthStep;
		iWStepD = i*pImgDest->widthStep;
		for(int j=0;j<iNC;j++)
		{
			iJStep = j*3;
			*(uchar*)(pImgDest->imageData + iWStepD + iJStep + 2) = *(uchar*)(pImgSource->imageData + iWStep + iJStep + 2);
			*(uchar*)(pImgDest->imageData + iWStepD + iJStep + 1) = *(uchar*)(pImgSource->imageData + iWStep + iJStep + 1);
			*(uchar*)(pImgDest->imageData + iWStepD + iJStep + 0) = *(uchar*)(pImgSource->imageData + iWStep + iJStep + 0);
		}
	}
	float fPer;
	for (int i=iNR;i<iNR+255;i++)
	{
		iWStep = (iNR-1+iNR-i)*pImgSource->widthStep;
		iWStepD = i*pImgDest->widthStep;
		fPer = (i-iNR)/255.0;

		for(int j=0;j<iNC;j++)
		{
			iJStep = j*3;
			*(uchar*)(pImgDest->imageData + iWStepD + iJStep + 2) = *(uchar*)(pImgSource->imageData + iWStep + iJStep + 2)*(1-fPer) + 255*(fPer);
			*(uchar*)(pImgDest->imageData + iWStepD + iJStep + 1) = *(uchar*)(pImgSource->imageData + iWStep + iJStep + 1)*(1-fPer) + 255*(fPer);
			*(uchar*)(pImgDest->imageData + iWStepD + iJStep + 0) = *(uchar*)(pImgSource->imageData + iWStep + iJStep + 0)*(1-fPer) + 255*(fPer);
		}
	}


}
