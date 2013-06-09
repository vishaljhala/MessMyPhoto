#include "StdAfx.h"
#include "ThermalDetector.h"
#include "GlobalHelper.h"


extern "C" __declspec(dllexport) void ThermalDetector(char *szPath, int bPreviewMode, int*iErrCode)
{
	GlobalHelper g_GH;
	g_GH.Init(szPath,iErrCode);
	if(*iErrCode >0)
		return;

	//g_GH.DebugOut("ThermalDetector: Begin",TRUE);
	char szParam[1024];
	szParam[0]=0;
	sprintf(szParam,"bPreviewMode=%d",bPreviewMode);
	//g_GH.DebugOut(szParam,FALSE);


	IplImage *pImgSource;
	int iRet = g_GH.load_RGB(szPath,&pImgSource,bPreviewMode);
	if(!iRet)
	{
		//g_GH.DebugOut("ThermalDetector: LoadImage failed",FALSE);
		*iErrCode = 1;
		return;
	}
	clsThermalDetector gs;
	gs.ThermalDetectorMain(&pImgSource,iErrCode,g_GH);

	iRet = g_GH.save_RGB(szPath,pImgSource);

	if(!iRet)
	{
		//g_GH.DebugOut("ThermalDetector: save_RGB failed",FALSE);
		*iErrCode = 1;
		return;
	}

	g_GH.cvReleaseImageProxy(&pImgSource);

	//g_GH.DebugOut("ThermalDetector: End",TRUE);

	*iErrCode = 0;
}
clsThermalDetector::clsThermalDetector()
{
}
clsThermalDetector::~clsThermalDetector()
{
}
void clsThermalDetector::ThermalDetectorMain(IplImage **pImgSrc,int*iErrCode,GlobalHelper &g_GH)
{
	pImgSource = *pImgSrc;

	if(!pImgSource)
	{
		//g_GH.DebugOut("ThermalDetector: Source Image NULL",FALSE);
		*iErrCode = 1;
		return;
	}



	iNC = pImgSource->width;
	iNR = pImgSource->height;
	
	pImgDest = g_GH.cvCreateImageProxy(cvSize(pImgSource->width,pImgSource->height),pImgSource->depth,3);
	if(!pImgDest)
	{
		//g_GH.DebugOut("NightVision: CreateImage failed",FALSE);
		*iErrCode = 1;
		return;
	}
	Invert(g_GH);
	g_GH.cvCvtColorProxy(pImgSource,pImgDest,CV_BGR2HSV);
	Histogram(g_GH);
	HeatMap(g_GH);
	g_GH.cvCvtColorProxy(pImgDest,pImgSource,CV_HSV2BGR);

	//*pImgSrc = pImgDest;
	g_GH.cvReleaseImageProxy(&pImgDest);

}
void clsThermalDetector::Invert(GlobalHelper &g_GH)
{

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

void clsThermalDetector::Histogram(GlobalHelper &g_GH)
{
	int iWStep, iJStep;
	int tmp;
	
	for(int i=0;i<256;i++)
		iHistogram[i] = 0;

	for(int i=0;i<iNR;i++)
	{
		iWStep = i*pImgDest->widthStep;
		for(int j=0;j<iNC;j++)
		{
			iJStep = j*3;

			iHistogram[*(uchar*)(pImgDest->imageData + iWStep + iJStep + 2)]++; 

		}
	}
	int iTotPix = iNC*iNR;
	int iCutOffMark = (float)iTotPix*0.25;

	int iFlag=1;
	int iTemp=0;
	for(int i=0;i<256;i++)
	{
		iTemp+=iHistogram[i];
		if(iTemp>=iCutOffMark*iFlag)
		{
			iCutOff[iFlag-1] = i;
			iFlag++;
			if(iFlag>3)
				break;
		}

	}

}
void clsThermalDetector::HeatMap(GlobalHelper &g_GH)
{
	int iWStep, iJStep;
	int tmp;
	//120 - 137 - blue
	//138 -158 - pink
	//158 - 193 - red
	//193 - 212 - yellow
	float f1 = 17.0/iCutOff[0];
	float f2 = 20.0/(iCutOff[1]-iCutOff[0]);
	float f3 = 35.0/(iCutOff[2]-iCutOff[1]);
	float f4 = 20.0/(255-iCutOff[2]);
	
	for(int i=0;i<iNR;i++)
	{
		iWStep = i*pImgDest->widthStep;
		for(int j=0;j<iNC;j++)
		{
			iJStep = j*3;

			*(uchar*)(pImgDest->imageData + iWStep + iJStep + 1) =*(uchar*)(pImgDest->imageData + iWStep + iJStep + 2);

			tmp = (float)*(uchar*)(pImgDest->imageData + iWStep + iJStep + 2);

			if(tmp<=iCutOff[0])
			{
				*(uchar*)(pImgDest->imageData + iWStep + iJStep ) = 120+f1*(tmp);
			}
			else if(tmp>iCutOff[0] && tmp<iCutOff[1])
			{
				*(uchar*)(pImgDest->imageData + iWStep + iJStep ) = 138+f1*(tmp-iCutOff[0]);
			}
			else if(tmp>iCutOff[1] && tmp<iCutOff[2])
			{
				*(uchar*)(pImgDest->imageData + iWStep + iJStep ) = 158+f1*(tmp-iCutOff[1]);
			}
			else
			{
				*(uchar*)(pImgDest->imageData + iWStep + iJStep ) = 193+f1*(tmp-iCutOff[2]);
			}
			//*(uchar*)(pImgDest->imageData + iWStep + iJStep + 0) = tmp;
		}
	}
/*	for(int i=0;i<256;i++)
	{
		iWStep = i*pImgDest->widthStep;
		for(int j=0;j<20;j++)
		{
			iJStep = j*3;

			*(uchar*)(pImgDest->imageData + iWStep + iJStep + 1) =*(uchar*)(pImgDest->imageData + iWStep + iJStep + 2) =255;
			*(uchar*)(pImgDest->imageData + iWStep + iJStep ) = i;
		}
	}*/
}
