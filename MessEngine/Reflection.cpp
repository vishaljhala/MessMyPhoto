#include "stdafx.h"
#include "Reflection.h"
#include "GlobalHelper.h"
 

extern "C" __declspec(dllexport) void Reflection(char *szPath, int iDistance, int iShadowLength, int iShadowIntensity, int*iErrCode)
{
	DWORD dwStart = GetTickCount();
	SYSTEMTIME stStart;
	GetLocalTime(&stStart);

	GlobalHelper g_GH;
	g_GH.Init(szPath,iErrCode);
	if(*iErrCode >0)
		return;

	//g_GH.DebugOut("Reflection: Begin",TRUE);
	
	IplImage *pImgSource;
	int iRet = g_GH.load_RGB(szPath,&pImgSource,false);
	if(!iRet)
	{
		//g_GH.DebugOut("Reflection: LoadImage failed",FALSE);
		*iErrCode = 1;
		return;
	}

	clsReflection Reflection;
	Reflection.ReflectionMain(&pImgSource, iDistance, iShadowLength, iShadowIntensity,g_GH);

	iRet = g_GH.save_RGB(szPath,pImgSource);
	if(!iRet)
	{
		//g_GH.DebugOut("Reflection: save_RGB failed",FALSE);
		*iErrCode = 1;
		return;
	}

	g_GH.cvReleaseImageProxy(&pImgSource);

	DWORD dwEnd = GetTickCount();
	char pszTimec[1024];pszTimec[0] = 0;
	sprintf(pszTimec,"Reflection,%d-%02d-%02d %02d:%02d:%02d,%ld\0",stStart.wYear,stStart.wMonth,stStart.wDay,stStart.wHour,
		stStart.wMinute, stStart.wSecond, dwEnd - dwStart);
	//sprintf(pszTimec,"Cyborg,%d-%02d-%02d %02d:%02d:%02d,%ld\0",stStart.wYear,stStart.wMonth,stStart.wDay,dwEnd - dwStart);

	g_GH.DebugOut(pszTimec);

}
clsReflection::clsReflection()
{
	
}
clsReflection::~clsReflection()
{
}
void clsReflection::ReflectionMain(IplImage **pImgSrc, int iDistance, int iShadowLength, int iShadowIntensity,GlobalHelper &g_GH)
{
	pImgSource = *pImgSrc;
	if(!pImgSource)
	{
		//g_GH.DebugOut("ReflectionMain: Source Image NULL",FALSE);
		//*iErrCode = 1;
		return;
	}

	iNC = pImgSource->width;
	iNR = pImgSource->height;

	m_iDistance = iDistance;
	m_iShadowLength = iShadowLength;
	m_iShadowIntensity = iShadowIntensity;

	ApplyCanvas(g_GH);
	
	*pImgSrc = pImgDest;

	g_GH.cvReleaseImageProxy(&pImgSource);


}

void clsReflection::ApplyCanvas(GlobalHelper &g_GH)
{
	int iNRNew = iNR + m_iDistance + iNR*m_iShadowLength/100.0;
	int iNRMed = iNR + m_iDistance;
	pImgDest = g_GH.cvCreateImageProxy(cvSize(iNC,iNRNew),pImgSource->depth,pImgSource->nChannels);

	int iWStep, iJStep;
	int iWStepD, iJStepD;
	//Copy Original Image
	for (int i=0;i<iNR;i++)
	{
		iWStep = (int)pImgSource->imageData + i*pImgSource->widthStep;
		iWStepD = (int)pImgDest->imageData + i*pImgDest->widthStep;
		for(int j=0;j<iNC;j++)
		{
			iJStep =  iWStep + j*3;
			iJStepD = iWStepD + j*3;
			*(uchar*)(iJStepD + 2) = *(uchar*)(iJStep + 2);
			*(uchar*)(iJStepD + 1) = *(uchar*)(iJStep + 1);
			*(uchar*)(iJStepD) = *(uchar*)(iJStep);
		}
	}

	//Copy Distance
	for (int i=iNR;i<iNRMed;i++)
	{
		iWStepD = (int)pImgDest->imageData + i*pImgDest->widthStep;
		for(int j=0;j<iNC;j++)
		{
			iJStepD =iWStepD + j*3;
			*(uchar*)(iJStepD + 2) = 
			*(uchar*)(iJStepD + 1) = 
			*(uchar*)(iJStepD) = 255;
		}
	}

	//Copy Reflection
	int iRefHgt = iNRNew - iNR;
	float fPer,fPerImg,fPerWhite;
	float fShadowIntensity = m_iShadowIntensity/100.0;

	for (int i=iNRMed;i<iNRNew;i++)
	{
		iWStep = (int)pImgSource->imageData + (iNR-1+iNRMed-i)*pImgSource->widthStep;
		iWStepD = (int)pImgDest->imageData + i*pImgDest->widthStep;
		fPer = 1-fShadowIntensity+(i-iNR)/(float)iRefHgt;
		if(fPer>1)fPer=1;
		fPerImg = 1-fPer;
		fPerWhite = 255*fPer;
		for(int j=0;j<iNC;j++)
		{
			iJStep  = iWStep + j*3;
			iJStepD = iWStepD + j*3;

			*(uchar*)(iJStepD + 2) = *(uchar*)(iJStep + 2)*(fPerImg) + (fPerWhite);
			*(uchar*)(iJStepD + 1) = *(uchar*)(iJStep + 1)*(fPerImg) + (fPerWhite);
			*(uchar*)(iJStepD) = *(uchar*)(iJStep)*(fPerImg) + (fPerWhite);
		}
	}


}
