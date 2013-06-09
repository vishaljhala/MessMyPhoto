#include "StdAfx.h"
#include "TieNDye.h"
#include "PerfectSketch.h"
#include "GlobalHelper.h"


extern "C" __declspec(dllexport) void TieNDye(char *szPath, int iSoftColor, int iHardColor, int iDetails, int*iErrCode)
{
	DWORD dwStart = GetTickCount();
	SYSTEMTIME stStart;
	GetLocalTime(&stStart);

	GlobalHelper g_GH;
	g_GH.Init(szPath,iErrCode);
	if(*iErrCode >0)
		return;

	//g_GH.DebugOut("TieNDye: Begin",TRUE);
	char szParam[1024];
	szParam[0]=0;
	sprintf(szParam,"bPreviewMode=%d",false);
	//g_GH.DebugOut(szParam,FALSE);


	IplImage *pImgSource;
	int iRet = g_GH.load_RGB(szPath,&pImgSource,false);
	if(!iRet)
	{
		//g_GH.DebugOut("TieNDye: LoadImage failed",FALSE);
		*iErrCode = 1;
		return;
	}
	clsPerfectSketch sktch;
	sktch.SketchFilter(&pImgSource,iDetails,iErrCode,g_GH);

	clsTieNDye gs;
	gs.TieNDyeMain(&pImgSource,iSoftColor,iHardColor,iErrCode,g_GH);

	iRet = g_GH.save_RGB(szPath,pImgSource);

	if(!iRet)
	{
		//g_GH.DebugOut("TieNDye: save_RGB failed",FALSE);
		*iErrCode = 1;
		return;
	}

	g_GH.cvReleaseImageProxy(&pImgSource);

	//g_GH.DebugOut("TieNDye: End",TRUE);

	*iErrCode = 0;

	DWORD dwEnd = GetTickCount();
	char pszTimec[1024];pszTimec[0] = 0;
	sprintf(pszTimec,"TechniColor,%d-%02d-%02d %02d:%02d:%02d,%ld\0",stStart.wYear,stStart.wMonth,stStart.wDay,stStart.wHour,
		stStart.wMinute, stStart.wSecond, dwEnd - dwStart);

	g_GH.DebugOut(pszTimec);

}
clsTieNDye::clsTieNDye()
{
}
clsTieNDye::~clsTieNDye()
{
}
void clsTieNDye::TieNDyeMain(IplImage **pImgSrc,int iSoftColor, int iHardColor,int*iErrCode,GlobalHelper &g_GH)
{
	pImgSource = *pImgSrc;
	m_iSoftColor = iSoftColor;
	m_iHardColor = iHardColor;

	if(!pImgSource)
	{
		//g_GH.DebugOut("TieNDye: Source Image NULL",FALSE);
		*iErrCode = 1;
		return;
	}

	iNC = pImgSource->width;
	iNR = pImgSource->height;
	Threshold(g_GH);
	Lines(g_GH);

	g_GH.cvReleaseImageProxy(&pImgSource);
	*pImgSrc = pImgCanvas;



}
void clsTieNDye::Threshold(GlobalHelper &g_GH)
{
	pImgCanvas = g_GH.cvCreateImageProxy(cvSize(iNC,iNR),pImgSource->depth,3);

	int sR = (m_iSoftColor >> 16) & 0xFF;	// Blue component
	int sG = (m_iSoftColor >> 8) & 0xFF;	// Green component
	int sB = (m_iSoftColor) & 0xFF;	// Red component

	int hR = (m_iHardColor >> 16) & 0xFF;	// Blue component
	int hG = (m_iHardColor >> 8) & 0xFF;	// Green component
	int hB = (m_iHardColor) & 0xFF;	// Red component

	// dark color 156,78,16
	int iWStep, iJStep;
	int tmp,tmp1;
	for(int i=0;i<iNR;i++)
	{
		iWStep = i*pImgCanvas->widthStep + (int)pImgCanvas->imageData;
		for(int j=0;j<iNC;j++)
		{
			iJStep = j*3;
			if(*(uchar*)(pImgSource->imageData + i*pImgSource->widthStep + j)<=128)
			{
				*(uchar*)( iWStep + iJStep + 0) =hB;
				*(uchar*)(iWStep + iJStep + 1) =hG; 
				*(uchar*)(iWStep + iJStep + 2) =hR;
			}
			else
			{
				*(uchar*)(iWStep + iJStep + 0) =sB;//129;
				*(uchar*)(iWStep + iJStep + 1) =sG;//232; 
				*(uchar*)(iWStep + iJStep + 2) =sR;//255;
			}
		}
	}
}
void clsTieNDye::Lines(GlobalHelper &g_GH)
{
	int hR = (m_iHardColor >> 16) & 0xFF;	// Blue component
	int hG = (m_iHardColor >> 8) & 0xFF;	// Green component
	int hB = (m_iHardColor) & 0xFF;	// Red component

	int tmp;
	int bFlag = 1;
	int iWCanvas, iWSource;
	for (int i=0;i<iNR;i++)
	{

		if(i%3==0) 
			if( bFlag) bFlag = 0;
			else bFlag = 1;
		if(bFlag)
		{
			iWCanvas = (int)pImgCanvas->imageData + i*pImgCanvas->widthStep;
			iWSource = (int)pImgSource->imageData + i*pImgSource->widthStep;

			for(int j=0;j<iNC;j++)
			{
				if(*(uchar*)( iWSource + j)>64 && *(uchar*)(iWSource + j)<192 )
				{
					/**(uchar*)(pImgCanvas->imageData + (i-1)*pImgCanvas->widthStep + j*3 + 0) =16;
					*(uchar*)(pImgCanvas->imageData + (i-1)*pImgCanvas->widthStep + j*3 + 1) =78; 
					*(uchar*)(pImgCanvas->imageData + (i-1)*pImgCanvas->widthStep + j*3 + 2) =156;
					*/
					*(uchar*)(iWCanvas + j*3 + 0) =hB;
					*(uchar*)(iWCanvas + j*3 + 1) =hG; 
					*(uchar*)(iWCanvas + j*3 + 2) =hR;
					/*
					*(uchar*)(pImgCanvas->imageData + (i+1)*pImgCanvas->widthStep + j*3 + 0) =16;
					*(uchar*)(pImgCanvas->imageData + (i+1)*pImgCanvas->widthStep + j*3 + 1) =78; 
					*(uchar*)(pImgCanvas->imageData + (i+1)*pImgCanvas->widthStep + j*3 + 2) =156;
					*/
				}
			}
		}
	}
}

