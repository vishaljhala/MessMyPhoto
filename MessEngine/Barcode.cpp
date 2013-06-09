#include "stdafx.h"
#include "Barcode.h"
#include "PainterlySketch.h"
#include "GlobalHelper.h"

extern char g_pcszPath[];

extern "C" __declspec(dllexport) void Barcode(char *szPath, int bPreviewMode, int*iErrCode)
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

	clsPainterlySketch ps;
	ps.SketchMain(&pImgSource,iErrCode,g_GH);

	clsBarcode Barcode;
	Barcode.BarcodeMain(&pImgSource,bPreviewMode,iErrCode,g_GH);

	iRet = g_GH.save_RGB(szPath,pImgSource);
	if(!iRet)
	{
		*iErrCode = 1;
		return;
	}

	g_GH.cvReleaseImageProxy(&pImgSource);

}
clsBarcode::clsBarcode()
{
	
}
clsBarcode::~clsBarcode()
{
}
void clsBarcode::BarcodeMain(IplImage **pImgSrc, int bPreviewMode, int*iErrCode,GlobalHelper &g_GH)
{
	pImgSource = *pImgSrc;
	if(!pImgSource)
	{
		*iErrCode = 1;
		return;
	}

	iNC = pImgSource->width;
	iNR = pImgSource->height;
	
	ApplyCanvas(g_GH);
	
	*pImgSrc = pImgDest;
	g_GH.cvReleaseImageProxy(&pImgSource);

}

void clsBarcode::ApplyCanvas(GlobalHelper &g_GH)
{
	
	char buff[1024];
	buff[0] = NULL;
	strcpy(buff,g_pcszPath);

	strcat(buff,"Barcode");

	IplImage *imgrgb;
	int iRet = g_GH.load_RGB(buff,&imgrgb,FALSE);
	if(!iRet)
	{
		return;
	}

	pImgDest = g_GH.cvCreateImageProxy(cvSize(iNC,iNR),pImgSource->depth,pImgSource->nChannels);
	IplImage *imgTemp;
	imgTemp = g_GH.cvCreateImageProxy(cvSize(imgrgb->width,imgrgb->height),imgrgb->depth,1);
	g_GH.cvCvtColorProxy(imgrgb,imgTemp,CV_BGR2GRAY);
	g_GH.cvResizeProxy(imgTemp,pImgDest,1);
	g_GH.cvReleaseImageProxy(&imgTemp);

	int iWStep, iJStep;
	int tmpS,tmpD;
	for (int i=0;i<iNR;i++)
	{
		iWStep = i*pImgSource->widthStep;
		for(int j=0;j<iNC;j++)
		{
			//iJStep = j*3;
			tmpS = *(uchar*)(pImgSource->imageData + iWStep + j );
			tmpD = *(uchar*)(pImgDest->imageData + iWStep + j);

			if(tmpS<128 && tmpD>128)
				*(uchar*)(pImgDest->imageData + iWStep + j) = 0;
			if(tmpS<128 && tmpD<128)
				*(uchar*)(pImgDest->imageData + iWStep + j) = 255;
			
		}
	}
	


	g_GH.cvReleaseImageProxy(&imgrgb);

}
