#include "stdafx.h"
#include "CrossProcess.h"
#include "GlobalHelper.h"
 

extern "C" __declspec(dllexport) void CrossProcess(char *szPath, int bPreviewMode, int*iErrCode)
{
	GlobalHelper g_GH;
	g_GH.Init(szPath,iErrCode);
	if(*iErrCode >0)
		return;

	
	IplImage *pImgSource;
	int iRet = g_GH.load_RGB(szPath,&pImgSource,bPreviewMode);
	if(!iRet)
	{
		*iErrCode = 1;
		return;
	}


	clsCrossProcess CrossProcess;
	CrossProcess.CrossProcessMain(&pImgSource,bPreviewMode,iErrCode,g_GH);

	iRet = g_GH.save_RGB(szPath,pImgSource);
	if(!iRet)
	{
		*iErrCode = 1;
		return;
	}

	g_GH.cvReleaseImageProxy(&pImgSource);

}
clsCrossProcess::clsCrossProcess()
{
	
}
clsCrossProcess::~clsCrossProcess()
{
}
void clsCrossProcess::CrossProcessMain(IplImage **pImgSrc, int bPreviewMode, int*iErrCode,GlobalHelper &g_GH)
{
	pImgSource = *pImgSrc;
	if(!pImgSource)
	{
		*iErrCode = 1;
		return;
	}

	iNC = pImgSource->width;
	iNR = pImgSource->height;
	
	pImgDest = g_GH.convertImageRGBtoHSV(pImgSource);
	//Histogram(g_GH);
	ApplyCanvas(g_GH);

	g_GH.cvReleaseImageProxy(&pImgSource);
	pImgSource = g_GH.convertImageHSVtoRGB(pImgDest);
	
	*pImgSrc = pImgSource;
	g_GH.cvReleaseImageProxy(&pImgDest);

}
void clsCrossProcess::Histogram(GlobalHelper &g_GH)
{
	int iWStep, iJStep;
	int tmpS,tmpD;
	
	for(int i=0;i<256;i++)
		iHistSat[i][0]=iHistSat[i][1]=iHistSat[i][2]=
		iHistVal[i][0]=iHistVal[i][1]=iHistVal[i][2]= 0;

	for (int i=0;i<iNR;i++)
	{
		iWStep = i*pImgDest->widthStep;
		for(int j=0;j<iNC;j++)
		{
			iJStep = j*3;
			iHistSat[*(uchar*)(pImgDest->imageData + iWStep + iJStep + 1 )][0]++;
			iHistVal[*(uchar*)(pImgDest->imageData + iWStep + iJStep + 0 )][0]++;
		}
	}
	int iMinCDFSat = iNC*iNR;
	int iMinCDFVal = iNC*iNR;
	for(int i=0;i<256;i++)
	{
		if(i>0)
		{
			iHistSat[i][1]=iHistSat[i][0]+iHistSat[i-1][1];
			iHistVal[i][1]=iHistVal[i][0]+iHistVal[i-1][1];
		}
		else
		{
			iHistSat[i][1]=iHistSat[i][0];
			iHistVal[i][1]=iHistVal[i][0];
		}
		if(iHistSat[i][1]<iMinCDFSat)
			iMinCDFSat = iHistSat[i][1];
		if(iHistVal[i][1]<iMinCDFVal)
			iMinCDFVal = iHistVal[i][1];

	}
	float iFactorSat = 255.0/((iNC*iNR)-iMinCDFSat);
	float iFactorVal = 255.0/((iNC*iNR)-iMinCDFVal);
	for(int i=0;i<256;i++)
	{
		iHistSat[i][2] = (iHistSat[i][1]-iMinCDFSat)*iFactorSat;
		iHistVal[i][2] = (iHistVal[i][1]-iMinCDFVal)*iFactorVal;
	}

}
void clsCrossProcess::ApplyCanvas(GlobalHelper &g_GH)
{
	int iWStep, iJStep;
	int tmpS,tmpD;

	for (int i=0;i<iNR;i++)
	{
		iWStep = i*pImgSource->widthStep;
		for(int j=0;j<iNC;j++)
		{
			iJStep = j*3;
			*(uchar*)(pImgDest->imageData + iWStep + iJStep + 0 ) = 30;
			
			//*(uchar*)(pImgDest->imageData + iWStep + iJStep + 1 ) = *(uchar*)(pImgDest->imageData + iWStep + iJStep + 0 );
			//*(uchar*)(pImgDest->imageData + iWStep + iJStep + 1 ) = 100;//iHistSat[*(uchar*)(pImgDest->imageData + iWStep + iJStep + 1 )][2];
			//*(uchar*)(pImgDest->imageData + iWStep + iJStep + 0 ) = iHistVal[*(uchar*)(pImgDest->imageData + iWStep + iJStep + 0 )][2];
				
		}
	}
}
