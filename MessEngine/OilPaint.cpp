#include "stdafx.h"
#include "OilPaint.h"
#include "GlobalHelper.h"
 

extern "C" __declspec(dllexport) void OilPaint(char *szPath, int bPreviewMode, int*iErrCode)
{
	GlobalHelper g_GH;
	g_GH.Init(szPath,iErrCode);
	if(*iErrCode >0)
		return;

	//g_GH.DebugOut("OilPaint: Begin",TRUE);
	
	IplImage *pImgSource;
	int iRet = g_GH.load_RGB(szPath,&pImgSource,bPreviewMode);
	if(!iRet)
	{
		//g_GH.DebugOut("OilPaint: LoadImage failed",FALSE);
		*iErrCode = 1;
		return;
	}


	clsOilPaint OilPaint;
	OilPaint.OilPaintMain(&pImgSource,bPreviewMode,iErrCode,g_GH);

	iRet = g_GH.save_RGB(szPath,pImgSource);
	if(!iRet)
	{
		//g_GH.DebugOut("OilPaint: save_RGB failed",FALSE);
		*iErrCode = 1;
		return;
	}

	g_GH.cvReleaseImageProxy(&pImgSource);
	//g_GH.DebugOut("OilPaint: End",TRUE);

}
clsOilPaint::clsOilPaint()
{
	
}
clsOilPaint::~clsOilPaint()
{
}
void clsOilPaint::OilPaintMain(IplImage **pImgSrc, int bPreviewMode, int*iErrCode,GlobalHelper &g_GH)
{
	pImgSource = *pImgSrc;
	if(!pImgSource)
	{
		//g_GH.DebugOut("OilPaintMain: Source Image NULL",FALSE);
		*iErrCode = 1;
		return;
	}

	iNC = pImgSource->width;
	iNR = pImgSource->height;
	
	ppIConv = g_GH.IntHeap2D(iNR,iNC);
	if(!ppIConv)
	{
		//g_GH.DebugOut("clsSobel::Sobel - FloatHelp2D failed",FALSE);
		return;
	}

	int iWStep, iJStep;
	for (int i=0;i<iNR;i++)
	{
		iWStep = i*pImgSource->widthStep;
		for(int j=0;j<iNC;j++)
		{
			iJStep = j*3;
			ppIConv[i][j] = (*(uchar*)(pImgSource->imageData + iWStep + iJStep + 0) +
				*(uchar*)(pImgSource->imageData + iWStep + iJStep + 1) +
				*(uchar*)(pImgSource->imageData + iWStep + iJStep + 2) )/3.0;
		}
	}
	pImgDest = g_GH.cvCreateImageProxy(cvSize(iNC,iNR),pImgSource->depth,3);

	OilPaintEffect(g_GH);

	*pImgSrc = pImgDest;
	g_GH.cvReleaseImageProxy(&pImgSource);
	free(ppIConv[0]);free(ppIConv);


}
void clsOilPaint::OilPaintEffect(GlobalHelper &g_GH)
{
	int iFilterMidPnt = 5;
	int iStep, jStep,kStep,lStep;
	
	int iInctyLvl = 24, iInctyBin[24][4], iMax=0, iMaxIndx=0,iCurIncty;

	for(int i=0;i<iNR;i++)
	{
		iStep = i*pImgDest->widthStep;
		for(int j=0;j<iNC;j++)
		{
			jStep = j*3;
			for(int m=0;m<24;m++)
				iInctyBin[m][0] = iInctyBin[m][1] = iInctyBin[m][2] = iInctyBin[m][3] = 0;
			iMax = iMaxIndx = 0;

			for(int k=i-iFilterMidPnt;k<=i+iFilterMidPnt;k++)
			{
				if(k<0 || k>=iNR) continue;
				kStep = k*pImgDest->widthStep;
		
				for(int l=j-iFilterMidPnt;l<=j+iFilterMidPnt;l++)
				{
					if(l<0 || l>=iNC) continue;
					lStep = l*3;
					iCurIncty = (ppIConv[k][l]*iInctyLvl)/256.0f;

					iInctyBin[iCurIncty][0]++;
					iInctyBin[iCurIncty][1]+=*(uchar*)(pImgSource->imageData + kStep + lStep + 2);
					iInctyBin[iCurIncty][2]+=*(uchar*)(pImgSource->imageData + kStep + lStep + 1);
					iInctyBin[iCurIncty][3]+=*(uchar*)(pImgSource->imageData + kStep + lStep + 0);
					
					if(iInctyBin[iCurIncty][0]>iMax)
					{
						iMax = iInctyBin[iCurIncty][0];
						iMaxIndx = iCurIncty;
					}

				}
			}
			*(uchar*)(pImgDest->imageData + iStep + jStep + 2) = (float)iInctyBin[iMaxIndx][1]/iMax;   
			*(uchar*)(pImgDest->imageData + iStep + jStep + 1) = (float)iInctyBin[iMaxIndx][2]/iMax;   
			*(uchar*)(pImgDest->imageData + iStep + jStep + 0) = (float)iInctyBin[iMaxIndx][3]/iMax;   
		}
	}


}
