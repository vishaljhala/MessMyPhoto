#include "StdAfx.h"
#include "HolgaArt.h"
#include "GlobalHelper.h"


extern "C" __declspec(dllexport) void HolgaArt(char *szPath, int bPreviewMode, int*iErrCode)
{
	GlobalHelper g_GH;
	g_GH.Init(szPath,iErrCode);
	if(*iErrCode >0)
		return;

	//g_GH.DebugOut("HolgaArt: Begin",TRUE);
	char szParam[1024];
	szParam[0]=0;
	sprintf(szParam,"bPreviewMode=%d",bPreviewMode);
	//g_GH.DebugOut(szParam,FALSE);


	IplImage *pImgSource;
	int iRet = g_GH.load_RGB(szPath,&pImgSource,bPreviewMode);
	if(!iRet)
	{
		//g_GH.DebugOut("HolgaArt: LoadImage failed",FALSE);
		*iErrCode = 1;
		return;
	}
	clsHolgaArt gs;
	gs.HolgaArtMain(&pImgSource,iErrCode,g_GH);

	iRet = g_GH.save_RGB(szPath,pImgSource);

	if(!iRet)
	{
		//g_GH.DebugOut("HolgaArt: save_RGB failed",FALSE);
		*iErrCode = 1;
		return;
	}

	g_GH.cvReleaseImageProxy(&pImgSource);

	//g_GH.DebugOut("HolgaArt: End",TRUE);

	*iErrCode = 0;
}
clsHolgaArt::clsHolgaArt()
{
}
clsHolgaArt::~clsHolgaArt()
{
}
void clsHolgaArt::HolgaArtMain(IplImage **pImgSrc,int*iErrCode,GlobalHelper &g_GH)
{
	pImgSource = *pImgSrc;

	if(!pImgSource)
	{
		//g_GH.DebugOut("HolgaArt: Source Image NULL",FALSE);
		*iErrCode = 1;
		return;
	}

	iNC = pImgSource->width;
	iNR = pImgSource->height;
	
	RedChannel(g_GH);
	CyanLayer(g_GH);

	*pImgSrc = pImgCanvas;
	g_GH.cvReleaseImageProxy(&pImgSource);

}
void clsHolgaArt::RedChannel(GlobalHelper &g_GH)
{
	pImgCanvas = g_GH.cvCreateImageProxy(cvSize(iNC,iNR),pImgSource->depth,pImgSource->nChannels);
	g_GH.cvCopyProxy(pImgSource,pImgCanvas,0);

	int iWStep, iJStep;
	int tmp,tmp1;
	for(int i=0;i<iNR;i++)
	{
		iWStep = i*pImgSource->widthStep;
		for(int j=0;j<iNC;j++)
		{
			iJStep = j*3;

			*(uchar*)(pImgCanvas->imageData + iWStep + iJStep + 1) = *(uchar*)(pImgCanvas->imageData + iWStep + iJStep + 0);
			
		}
	}
}
void clsHolgaArt::CyanLayer(GlobalHelper &g_GH)
{

	
	BYTE r,g,b,c,y,m,k;
	int iWStep, iJStep,iJStepDisp;
	int iDisplacement = iNC*0.05;
	for(int i=0;i<iNR;i++)
	{
		iWStep = i*pImgCanvas->widthStep;
		for(int j=0;j<iNC;j++)
		{
			iJStep = j*3;

			r = *(uchar*)(pImgSource->imageData + iWStep + iJStep + 2);
			g = *(uchar*)(pImgSource->imageData + iWStep + iJStep + 1);
			b = *(uchar*)(pImgSource->imageData + iWStep + iJStep + 0);

			g=b=r;
			r=0;

			iJStepDisp = j+iDisplacement;
			if(iJStepDisp>=iNC) iJStepDisp -=iNC;
			iJStepDisp*=3;

			*(uchar*)(pImgCanvas->imageData + iWStep + iJStepDisp + 2) = *(uchar*)(pImgCanvas->imageData + iWStep + iJStepDisp + 2)*0.6 + r*0.4;
			*(uchar*)(pImgCanvas->imageData + iWStep + iJStepDisp + 1) = *(uchar*)(pImgCanvas->imageData + iWStep + iJStepDisp + 1)*0.6 + g*0.3;
			*(uchar*)(pImgCanvas->imageData + iWStep + iJStepDisp + 0) = *(uchar*)(pImgCanvas->imageData + iWStep + iJStepDisp + 0)*0.6 + b*0.4;

		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//
// RGB --> CMYK                              CMYK --> RGB
// ---------------------------------------   --------------------------------------------
// Black   = minimum(1-Red,1-Green,1-Blue)   Red   = 1-minimum(1,Cyan*(1-Black)+Black)
// Cyan    = (1-Red-Black)/(1-Black)         Green = 1-minimum(1,Magenta*(1-Black)+Black)
// Magenta = (1-Green-Black)/(1-Black)       Blue  = 1-minimum(1,Yellow*(1-Black)+Black)
// Yellow  = (1-Blue-Black)/(1-Black)
//


///////////////////////////////////////////////////////////////////////////////
// RGB2CMYK
void clsHolgaArt::RGB2CMYK(BYTE r, BYTE g, BYTE b, BYTE& c, BYTE& m, BYTE& y, BYTE& k)
{
	R = (double) r;
	G = (double) g;
	B = (double) b;

	R = 1.0 - (R / 255.0);
	G = 1.0 - (G / 255.0);
	B = 1.0 - (B / 255.0);

	if (R < G)
		K = R;
	else
		K = G;
	if (B < K)
		K = B;

	C = (R - K)/(1.0 - K);
	M = (G - K)/(1.0 - K);
	Y = (B - K)/(1.0 - K);

	C = (C * 100) + 0.5;
	M = (M * 100) + 0.5;
	Y = (Y * 100) + 0.5;
	K = (K * 100) + 0.5;

	c = (BYTE) C;
	m = (BYTE) M;
	y = (BYTE) Y;
	k = (BYTE) K;
}

///////////////////////////////////////////////////////////////////////////////
// CMYK2RGB
void clsHolgaArt::CMYK2RGB(BYTE c, BYTE m, BYTE y, BYTE k,BYTE &r, BYTE &g, BYTE &b)
{


	C = (double) c;
	M = (double) m;
	Y = (double) y;
	K = (double) k;

	C = C / 255.0;
	M = M / 255.0;
	Y = Y / 255.0;
	K = K / 255.0;

	R = C * (1.0 - K) + K;
	G = M * (1.0 - K) + K;
	B = Y * (1.0 - K) + K;

	R = (1.0 - R) * 255.0 + 0.5;
	G = (1.0 - G) * 255.0 + 0.5;
	B = (1.0 - B) * 255.0 + 0.5;

	r = (BYTE) R;
	g = (BYTE) G;
	b = (BYTE) B;
}


