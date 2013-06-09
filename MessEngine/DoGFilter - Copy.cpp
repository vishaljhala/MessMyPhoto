#include "stdafx.h"
#include "DoGFilter.h"
#include "CartoonEffect.h"
#include "GlobalHelper.h"
#define PI 3.1415926535


extern "C" __declspec(dllexport) void DoG(char *szPath,int iFilterSize, float fSigmaDist, float fSigmaColor, 
	 int bPreviewMode, int*iErrCode)
{
	GlobalHelper g_GH;
	g_GH.Init(szPath,iErrCode);
	if(*iErrCode >0)
		return;

	g_GH.DebugOut("DoG: Begin",TRUE);

	IplImage *pImgSource;
	int iRet = g_GH.load_RGB(szPath,&pImgSource,bPreviewMode);
	if(!iRet)
	{
		g_GH.DebugOut("DoG: LoadImage failed",FALSE);
		*iErrCode = 1;
		return;
	}
	clsCartoonEffect Cartoon;
	Cartoon.CartoonEffectMain(&pImgSource,iFilterSize, fSigmaDist, fSigmaColor, 
		 iErrCode,g_GH);

	clsDoGFilter dog;
	dog.SketchMain(&pImgSource,iFilterSize, fSigmaDist, fSigmaColor, 
		 iErrCode,g_GH);
	
	iRet = g_GH.save_RGB(szPath,pImgSource);
	if(!iRet)
	{
		g_GH.DebugOut("DoG: save_RGB failed",FALSE);
		*iErrCode = 1;
		return;
	}

	g_GH.cvReleaseImageProxy(&pImgSource);

	g_GH.DebugOut("DoG: End",TRUE);

	*iErrCode = 0;

	
}

clsDoGFilter::clsDoGFilter()
{
}
clsDoGFilter::~clsDoGFilter()
{
}
void clsDoGFilter::SketchMain(IplImage **pImgSrc,int iFilterSize, float fSigmaDist, float fSigmaColor, 
	  int*iErrCode,GlobalHelper &g_GH)
{
	pImgSource = *pImgSrc;
	m_iFilterSize =iFilterSize;
	m_fSigmaDist = fSigmaDist;
	m_fSigmaColor = fSigmaColor;

	if(!pImgSource)
	{
		g_GH.DebugOut("DoG: Source Image NULL",FALSE);
		*iErrCode = 1;
		return;
	}

	iNC = pImgSource->width;
	iNR = pImgSource->height;
	pImgDest = g_GH.cvCreateImageProxy(cvSize(pImgSource->width,pImgSource->height),IPL_DEPTH_8U,1);

	g_GH.cvCvtColorProxy(pImgSource,pImgDest,CV_BGR2GRAY);
	///g_GH.cvSaveImageProxy("c:\\testing.jpg",pImgDest);

	float ** ppfGradXY1, **ppfGradXY2;
	ppfGradXY1 = g_GH.FloatHeap2D(iNR,iNC);
	if(!ppfGradXY1)
	{
		g_GH.DebugOut("DoG: FloatHepa2D failed",FALSE);
		*iErrCode = 1;
		return;
	}
	
	ppfGradXY2 = g_GH.FloatHeap2D(iNR,iNC);
	if(!ppfGradXY2)
	{
		g_GH.DebugOut("DoG: FloatHepa2D failed",FALSE);
		*iErrCode = 1;
		return;
	}

	float *pfFilter = NULL;
	iFilterSize = 7;
	float fSigma = 4.8;
	
	pfFilter = (float *) calloc ( 19, sizeof (float)  );
	
	CreateGaussFilter(iFilterSize,fSigma,pfFilter);
	Convolution(pfFilter, ppfGradXY1,iFilterSize, fSigma,g_GH);


	iFilterSize = 11;
	fSigma *=1.6;
	CreateGaussFilter(iFilterSize,fSigma,pfFilter);
	Convolution(pfFilter, ppfGradXY2,iFilterSize, fSigma,g_GH);

	Diff(ppfGradXY1,ppfGradXY2,g_GH);

	Threshold(ppfGradXY1,g_GH);
	// g_GH.save_RGB("c:\\aftthershold.jpg",pImgDest);

	g_GH.cvCvtColorProxy(pImgDest,pImgSource,CV_GRAY2BGR);
	
	//Memory Cleaning
	free(pfFilter);
	free(ppfGradXY1[0]);free(ppfGradXY1);
	free(ppfGradXY2[0]);free(ppfGradXY2);
	*pImgSrc = pImgSource;

}
void clsDoGFilter::CreateGaussFilter(int iFilterSize,float fSigma, float*pfFilter)
{
	int iCenter = (int)((float)iFilterSize/2);

	for(int i=0;i<=iCenter;i++)
	{
			pfFilter[iCenter+i] = Gauss(i, fSigma);
			pfFilter[iCenter-i] = Gauss(i, fSigma);
	}
}
float clsDoGFilter::Gauss(int x, float fSigma)
{
	
	float fNorm = exp((double) ((-x*x)/(2*fSigma*fSigma)));
	return (1/(sqrt(2*PI)*fSigma)) * fNorm;
}
void clsDoGFilter::Convolution(float*pfFilter, float **ppfGradXY1,int iFilterSize, float fSigma,GlobalHelper &g_GH)
{
	float fTempX,fTempY;
	int iNCFilterMidPnt = iFilterSize/2;
	int iStep, ikStep;
	
	for(int i=iNCFilterMidPnt;i<iNR;i++)
	{
		for(int j=iNCFilterMidPnt;j<iNC;j++)
		{
			fTempX = fTempY = 0;
			for(int k=-iNCFilterMidPnt;k<=iNCFilterMidPnt;k++)
			{
					//if(k==0 && l==0) continue;
					if(i+k<0 || i+k>=iNR) continue;
					if(j+k<0 || j+k>=iNC) continue;
					fTempY += pfFilter[iNCFilterMidPnt+k]*(*(UCHAR*)(pImgDest->imageData + (i+k)*pImgDest->widthStep+j));
					//if(k==0) continue;
					fTempX += pfFilter[iNCFilterMidPnt+k]*(*(UCHAR*)(pImgDest->imageData + (i)*pImgDest->widthStep+j+k));

			}
			ppfGradXY1[i][j] = sqrt(fTempX*fTempX+fTempY*fTempY);

		}
	}


}
void clsDoGFilter::Diff(float **ppfGradXY1,float **ppfGradXY2,GlobalHelper &g_GH)
{
	for (int i=0;i<iNR;i++)
	{
		for(int j=0;j<iNC;j++)
		{
			ppfGradXY1[i][j] = ppfGradXY1[i][j] - ppfGradXY2[i][j];

		}
	}
}
void clsDoGFilter::Threshold(float **ppfGradXY1,GlobalHelper &g_GH)
{
	int iHist[256];
	int iCnt=0;
	for(int i=0;i<256;i++)
		iHist[i] = 0;

	for (int i=0;i<iNR;i++)
	{
		for(int j=0;j<iNC;j++)
		{
			if(ppfGradXY1[i][j]<=0)
			{
				iHist[(int)floor(abs(ppfGradXY1[i][j]))]++;
				iCnt++;
			}
		}
	}
	iCnt=(float)iCnt*0.1;
	int iTmp=0;
	for(int i=255;i>=0;i--)
	{
		iTmp+=iHist[i];
		if(iTmp>=iCnt)
		{
			iTmp= i;
			break;
		}
	}

	int iStep,jStep,tmp;
	for (int i=0;i<iNR;i++)
	{
		iStep = i*pImgDest->widthStep;
		for(int j=0;j<iNC;j++)
		{
			if(ppfGradXY1[i][j]<=0)
				*(uchar*)(pImgDest->imageData + iStep + j) = 0;
			else
				*(uchar*)(pImgDest->imageData + iStep + j) = 255;//ppfGradXY1[i][j];

		}
	}

}