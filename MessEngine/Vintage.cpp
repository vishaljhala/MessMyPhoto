#include "StdAfx.h"
#include "vintage.h"
#include "GlobalHelper.h"


extern "C" __declspec(dllexport) void Vintage(char *szPath, int bPreviewMode, int*iErrCode)
{
	GlobalHelper g_GH;
	g_GH.Init(szPath,iErrCode);
	if(*iErrCode >0)
		return;

	//g_GH.DebugOut("Vintage: Begin",TRUE);
	char szParam[1024];
	szParam[0]=0;
	sprintf(szParam,"bPreviewMode=%d",bPreviewMode);
	//g_GH.DebugOut(szParam,FALSE);


	IplImage *pImgSource;
	int iRet = g_GH.load_RGB(szPath,&pImgSource,bPreviewMode);
	if(!iRet)
	{
		//g_GH.DebugOut("Vintage: LoadImage failed",FALSE);
		*iErrCode = 1;
		return;
	}
	clsVintage gs;
	gs.VintageMain(&pImgSource,iErrCode,g_GH);

	iRet = g_GH.save_RGB(szPath,pImgSource);

	if(!iRet)
	{
		//g_GH.DebugOut("Vintage: save_RGB failed",FALSE);
		*iErrCode = 1;
		return;
	}

	g_GH.cvReleaseImageProxy(&pImgSource);

	//g_GH.DebugOut("Vintage: End",TRUE);

	*iErrCode = 0;
}
clsVintage::clsVintage()
{
}
clsVintage::~clsVintage()
{
}
void clsVintage::VintageMain(IplImage **pImgSrc,int*iErrCode,GlobalHelper &g_GH)
{
	pImgSource = *pImgSrc;

	if(!pImgSource)
	{
		//g_GH.DebugOut("Vintage: Source Image NULL",FALSE);
		*iErrCode = 1;
		return;
	}

	iNC = pImgSource->width;
	iNR = pImgSource->height;
	
	/*pImgDest = g_GH.cvCreateImageProxy(cvSize(pImgSource->width,pImgSource->height),pImgSource->depth,3);
	if(!pImgDest)
	{
		g_GH.DebugOut("Vintage: CreateImage failed",FALSE);
		*iErrCode = 1;
		return;
	}
	g_GH.cvCvtColorProxy(pImgSource,pImgDest,CV_BGR2Lab);*/
	Vintage(g_GH);
	//g_GH.cvCvtColorProxy(pImgDest,pImgSource,CV_Lab2BGR);

	//*pImgSrc = pImgDest;
	//g_GH.cvReleaseImageProxy(&pImgDest);

}
void clsVintage::Vintage(GlobalHelper &g_GH)
{
	BYTE r,g,b,c,y,m,k;
	int iWStep, iJStep;
	int tmp;
	for(int i=0;i<iNR;i++)
	{
		iWStep = i*pImgSource->widthStep;
		for(int j=0;j<iNC;j++)
		{
			iJStep = j*3;

				tmp = *(uchar*)(pImgSource->imageData + iWStep + iJStep + 2);
				*(uchar*)(pImgSource->imageData + iWStep + iJStep + 2) = tmp*0.5 + 220*0.40 + 17*0.10;

				tmp = *(uchar*)(pImgSource->imageData + iWStep + iJStep + 1);
				*(uchar*)(pImgSource->imageData + iWStep + iJStep + 1) = tmp*0.5 + 227*0.40 + 24*0.10;

				tmp = *(uchar*)(pImgSource->imageData + iWStep + iJStep + 0);
				*(uchar*)(pImgSource->imageData + iWStep + iJStep + 0) = tmp*0.5 + 84*0.40 + 66*0.10;


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
void clsVintage::RGB2CMYK(BYTE r, BYTE g, BYTE b, BYTE& c, BYTE& m, BYTE& y, BYTE& k)
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
void clsVintage::CMYK2RGB(BYTE c, BYTE m, BYTE y, BYTE k,BYTE &r, BYTE &g, BYTE &b)
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


