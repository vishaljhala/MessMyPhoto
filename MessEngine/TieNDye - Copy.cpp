#include "StdAfx.h"
#include "TieNDye.h"
#include "PerfectSketch.h"
#include "GlobalHelper.h"


extern "C" __declspec(dllexport) void TieNDye(char *szPath, int bPreviewMode, int*iErrCode)
{
	GlobalHelper g_GH;
	g_GH.Init(szPath,iErrCode);
	if(*iErrCode >0)
		return;

	g_GH.DebugOut("TieNDye: Begin",TRUE);
	char szParam[1024];
	szParam[0]=0;
	sprintf(szParam,"bPreviewMode=%d",bPreviewMode);
	g_GH.DebugOut(szParam,FALSE);


	IplImage *pImgSource;
	int iRet = g_GH.load_RGB(szPath,&pImgSource,bPreviewMode);
	if(!iRet)
	{
		g_GH.DebugOut("TieNDye: LoadImage failed",FALSE);
		*iErrCode = 1;
		return;
	}
	clsPerfectSketch sktch;
	sktch.SketchFilter(&pImgSource,0,iErrCode,g_GH);

	clsTieNDye gs;
	gs.TieNDyeMain(&pImgSource,iErrCode,g_GH);

	iRet = g_GH.save_RGB(szPath,pImgSource);

	if(!iRet)
	{
		g_GH.DebugOut("TieNDye: save_RGB failed",FALSE);
		*iErrCode = 1;
		return;
	}

	g_GH.cvReleaseImageProxy(&pImgSource);

	g_GH.DebugOut("TieNDye: End",TRUE);

	*iErrCode = 0;
}
clsTieNDye::clsTieNDye()
{
}
clsTieNDye::~clsTieNDye()
{
}
void clsTieNDye::TieNDyeMain(IplImage **pImgSrc,int*iErrCode,GlobalHelper &g_GH)
{
	pImgSource = *pImgSrc;

	if(!pImgSource)
	{
		g_GH.DebugOut("TieNDye: Source Image NULL",FALSE);
		*iErrCode = 1;
		return;
	}

	iNC = pImgSource->width;
	iNR = pImgSource->height;
	pImgGrey = g_GH.cvCreateImageProxy(cvSize(iNC,iNR),pImgSource->depth,1);
	g_GH.cvCvtColorProxy(pImgSource,pImgGrey,CV_BGR2GRAY);
	ppfSobelX = g_GH.IntHeap2D(iNR,iNC);


	Sobel(g_GH);
	Histogram(g_GH);
	Threshold(g_GH);
	Lines(g_GH);

	*pImgSrc = pImgCanvas;
	g_GH.cvReleaseImageProxy(&pImgSource);
	g_GH.cvReleaseImageProxy(&pImgGrey);
	free(ppfSobelX[0]);free(ppfSobelX);

}
void clsTieNDye::Histogram(GlobalHelper &g_GH)
{
	int iPercentGreyLevel,iPerGradient;
	
	for(int i=0;i<256;i++)
		iHistGreyLevel[i]=iHistGradient[i]=0;

	for (int i=0;i<iNR;i++)
	{
		for(int j=0;j<iNC;j++)
		{
			iHistGreyLevel[*(uchar*)(pImgGrey->imageData + i*pImgGrey->widthStep + j )]++;
			iHistGradient[ ppfSobelX[i][j] ]++;
		}
	}
	iPercentGreyLevel = iNC*iNR*0.25;
	iPerGradient = iNC*iNR*0.30;
	int iCum = 0;

	for(int i=0;i<256;i++)
	{
		iCum += iHistGreyLevel[i];
		if(iCum>iPercentGreyLevel)
		{
			iCutOff = i;
			break;
		}
	}
	iCum=0;
	for(int i=255;i>=0;i--)
	{
		iCum += iHistGradient[i];
		if(iCum>iPerGradient)
		{
			iCutOffSobel = i;
			break;
		}
	}

}

void clsTieNDye::Threshold(GlobalHelper &g_GH)
{
	pImgCanvas = g_GH.cvCreateImageProxy(cvSize(iNC,iNR),pImgSource->depth,pImgSource->nChannels);

	// dark color 156,78,16
	int iWStep, iJStep;
	int tmp,tmp1;
	for(int i=0;i<iNR;i++)
	{
		iWStep = i*pImgSource->widthStep;
		for(int j=0;j<iNC;j++)
		{
			iJStep = j*3;
			if(*(uchar*)(pImgGrey->imageData + i*pImgGrey->widthStep + j)<=iCutOff)
			{
				*(uchar*)(pImgCanvas->imageData + iWStep + iJStep + 0) =16;
				*(uchar*)(pImgCanvas->imageData + iWStep + iJStep + 1) =78; 
				*(uchar*)(pImgCanvas->imageData + iWStep + iJStep + 2) =156;
			}
			else
			{
				*(uchar*)(pImgCanvas->imageData + iWStep + iJStep + 0) =129;
				*(uchar*)(pImgCanvas->imageData + iWStep + iJStep + 1) =232; 
				*(uchar*)(pImgCanvas->imageData + iWStep + iJStep + 2) =255;
			}
			
		}
	}
}
int clsTieNDye::Sobel(GlobalHelper &g_GH)
{
	int x,y;
	int i_1,i1,iw;
	iMaxSobel = 0;
	for(int i=1;i<iNR-1;i++)
	{
		i_1 = (i-1)*pImgGrey->widthStep;
		i1 = (i+1)*pImgGrey->widthStep;
		iw = (i)*pImgGrey->widthStep;

		for(int j=1;j<iNC-1;j++)
		{

			x = -1*(*(UCHAR*)(pImgGrey->imageData + i_1 +j-1)) + 
				-2*(*(UCHAR*)(pImgGrey->imageData + i_1 +j)) + 
				-1*(*(UCHAR*)(pImgGrey->imageData + i_1 +j+1)) +
				*(UCHAR*)(pImgGrey->imageData + i1 +j-1) + 
				2*(*(UCHAR*)(pImgGrey->imageData + i1 +j)) + 
				1*(*(UCHAR*)(pImgGrey->imageData + i1 +j+1));

			y = -1*(*(UCHAR*)(pImgGrey->imageData + i_1 +j-1)) + 
				-2*(*(UCHAR*)(pImgGrey->imageData + iw +j-1)) + 
				-1*(*(UCHAR*)(pImgGrey->imageData + i1 +j-1)) +
				*(UCHAR*)(pImgGrey->imageData + i_1 +j+1) + 
				2*(*(UCHAR*)(pImgGrey->imageData + iw +j+1)) + 
				1*(*(UCHAR*)(pImgGrey->imageData + i1 +j+1));

			//ppfSobelX[i][j] = x;
			ppfSobelX[i][j] = sqrt((float)x*x+y*y);
			
			if(ppfSobelX[i][j]>iMaxSobel)
				iMaxSobel = ppfSobelX[i][j];
		}
	}

	for(int i=0;i<iNR;i++)
		ppfSobelX[i][0] = 0;
	for(int i=0;i<iNR;i++)
		ppfSobelX[i][iNC-1] = 0;
	for(int j=0;j<iNC;j++)
		ppfSobelX[0][j] = 0;
	for(int j=0;j<iNC;j++)
		ppfSobelX[iNR-1][j] = 0;
	
	float fMaxSobel = 255.0/iMaxSobel;
	for(int i=1;i<iNR-1;i++)
	{
		for(int j=1;j<iNC-1;j++)
		{
			ppfSobelX[i][j] *=fMaxSobel;			
		}
	}
	return 0;
}	
void clsTieNDye::Lines(GlobalHelper &g_GH)
{
	int tmp;
	int bFlag = 1;
	for (int i=0;i<iNR;i++)
	{
		if(i%3==0) 
			if( bFlag) bFlag = 0;
			else bFlag = 1;
		if(bFlag)
		for(int j=0;j<iNC;j++)
		{
			if(ppfSobelX[i][j]>=iCutOffSobel)
			{
				/**(uchar*)(pImgCanvas->imageData + (i-1)*pImgCanvas->widthStep + j*3 + 0) =16;
				*(uchar*)(pImgCanvas->imageData + (i-1)*pImgCanvas->widthStep + j*3 + 1) =78; 
				*(uchar*)(pImgCanvas->imageData + (i-1)*pImgCanvas->widthStep + j*3 + 2) =156;
				*/
				*(uchar*)(pImgCanvas->imageData + i*pImgCanvas->widthStep + j*3 + 0) =16;
				*(uchar*)(pImgCanvas->imageData + i*pImgCanvas->widthStep + j*3 + 1) =78; 
				*(uchar*)(pImgCanvas->imageData + i*pImgCanvas->widthStep + j*3 + 2) =156;
				/*
				*(uchar*)(pImgCanvas->imageData + (i+1)*pImgCanvas->widthStep + j*3 + 0) =16;
				*(uchar*)(pImgCanvas->imageData + (i+1)*pImgCanvas->widthStep + j*3 + 1) =78; 
				*(uchar*)(pImgCanvas->imageData + (i+1)*pImgCanvas->widthStep + j*3 + 2) =156;
				*/
			}			
		}
	}
}

