#include "stdafx.h"
#include "WaterColor.h"
#include "Painterly.h"
#include "GlobalHelper.h"
 
extern char g_pcszPath[];

extern "C" __declspec(dllexport) void WaterColor(char *szPath, int bPreviewMode, int*iErrCode)
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

	PainterlyParam param;
	param.fSigma_blur_factor = 0.5;
	param.fThreshold = 50; 			
	param.iMaxStrokeLength = 32;
	param.iMinStrokeLength = 8;
	param.iMaxBrushSize = 16;

	clsPainterly paint;
	paint.PainterlyMain(param,&pImgSource,g_GH);

	clsWaterColor WaterColor;
	WaterColor.WaterColorMain(&pImgSource,bPreviewMode,iErrCode,g_GH);

	iRet = g_GH.save_RGB(szPath,pImgSource);
	if(!iRet)
	{
		*iErrCode = 1;
		return;
	}

	g_GH.cvReleaseImageProxy(&pImgSource);
	//g_GH.DebugOut("WaterColor: End",TRUE);

}
clsWaterColor::clsWaterColor()
{
	
}
clsWaterColor::~clsWaterColor()
{
}
void clsWaterColor::WaterColorMain(IplImage **pImgSrc, int bPreviewMode, int*iErrCode,GlobalHelper &g_GH)
{
	pImgSource = *pImgSrc;
	if(!pImgSource)
	{
		//g_GH.DebugOut("WaterColorMain: Source Image NULL",FALSE);
		*iErrCode = 1;
		return;
	}

	iNC = pImgSource->width;
	iNR = pImgSource->height;
	
	ppfConv = g_GH.FloatHeap2D(iNR,iNC);
	if(!ppfConv)
	{
		//g_GH.DebugOut("clsSobel::Sobel - FloatHelp2D failed",FALSE);
		return;
	}

	pImgGrey = g_GH.cvCreateImageProxy(cvSize(iNC,iNR),pImgSource->depth,1);
	g_GH.cvCvtColorProxy(pImgSource,pImgGrey,CV_BGR2GRAY);

	pImgHSV = g_GH.convertImageRGBtoHSV(pImgSource);

	Sobel(g_GH);
	WaterStrokes(g_GH);
	
	pImgDest = g_GH.convertImageHSVtoRGB(pImgHSV);
	ApplyCanvas(g_GH);

	*pImgSrc = pImgDest;
	g_GH.cvReleaseImageProxy(&pImgSource);
	g_GH.cvReleaseImageProxy(&pImgGrey);
	g_GH.cvReleaseImageProxy(&pImgHSV);
	free(ppfConv[0]);free(ppfConv);


}
void clsWaterColor::WaterStrokes(GlobalHelper &g_GH)
{
	int iWStep, iJStep;
	int tmp;
	for (int i=0;i<iNR;i++)
	{
		iWStep = i*pImgSource->widthStep;

		for(int j=0;j<iNC;j++)
		{
			iJStep = j*3;

			if(ppfConv[i][j]>10)
			{
				tmp = *(uchar*)(pImgHSV->imageData + iWStep + iJStep + 1)+ppfConv[i][j];
				if(tmp>255) tmp=255;
				*(uchar*)(pImgHSV->imageData + iWStep + iJStep + 1) = tmp;
			}

		}
	}

}
void clsWaterColor::ApplyCanvas(GlobalHelper &g_GH)
{
	
	char buff[1024];
	buff[0] = NULL;
	strcpy(buff,g_pcszPath);

	strcat(buff,"watercolortexture");

	IplImage *imgrgb;
	int iRet = g_GH.load_RGB(buff,&imgrgb,FALSE);
	if(!iRet)
	{
		//g_GH.DebugOut("clsPatronize::ApplyCanvas - Failed to load canvas",FALSE);
		//g_GH.DebugOut(buff,FALSE);
		return;
	}

	IplImage *pImgTemp = g_GH.cvCreateImageProxy(cvSize(iNC,iNR),pImgSource->depth,pImgSource->nChannels);
	g_GH.cvResizeProxy(imgrgb,pImgTemp,1);

	int iWStep, iJStep;
	int tmp;
	for (int i=0;i<iNR;i++)
	{
		iWStep = i*pImgSource->widthStep;
		for(int j=0;j<iNC;j++)
		{
			iJStep = j*3;
			*(uchar*)(pImgDest->imageData + iWStep + iJStep + 0) = 
			*(uchar*)(pImgDest->imageData + iWStep + iJStep + 0)*0.8+
			*(uchar*)(pImgTemp->imageData + iWStep + iJStep + 0)*0.2;

			*(uchar*)(pImgDest->imageData + iWStep + iJStep + 1) = 
			*(uchar*)(pImgDest->imageData + iWStep + iJStep + 1)*0.8+
			*(uchar*)(pImgTemp->imageData + iWStep + iJStep + 1)*0.2;

			*(uchar*)(pImgDest->imageData + iWStep + iJStep + 2) = 
			*(uchar*)(pImgDest->imageData + iWStep + iJStep + 2)*0.8+
			*(uchar*)(pImgTemp->imageData + iWStep + iJStep + 2)*0.2;
		}
	}
	


	g_GH.cvReleaseImageProxy(&imgrgb);
	g_GH.cvReleaseImageProxy(&pImgTemp);
}
int clsWaterColor::Sobel(GlobalHelper &g_GH)
{
	int x,y;
	int i_1Step, iStep, ip1Step;
	for(int i=1;i<iNR-1;i++)
	{
		iStep = i*pImgGrey->widthStep;
		i_1Step = (i-1)*pImgGrey->widthStep;
		ip1Step = (i+1)*pImgGrey->widthStep;
		for(int j=1;j<iNC-1;j++)
		{
			x = -1* (*(UCHAR*)(pImgGrey->imageData+i_1Step+j-1))  + -2*(*(UCHAR*)(pImgGrey->imageData+i_1Step+j)) + 
				-1*(*(UCHAR*)(pImgGrey->imageData+i_1Step+j+1)) + (*(UCHAR*)(pImgGrey->imageData+ip1Step+j-1)) + 
				2*(*(UCHAR*)(pImgGrey->imageData+ip1Step+j)) + 1*(*(UCHAR*)(pImgGrey->imageData+ip1Step+j+1));

			y = -1*(*(UCHAR*)(pImgGrey->imageData+i_1Step+j-1)) + -2*(*(UCHAR*)(pImgGrey->imageData+iStep+j-1)) + 
				-1*(*(UCHAR*)(pImgGrey->imageData+ip1Step+j-1)) +(*(UCHAR*)(pImgGrey->imageData+i_1Step+j+1)) + 
				2*(*(UCHAR*)(pImgGrey->imageData+iStep+j+1)) + 1*(*(UCHAR*)(pImgGrey->imageData+ip1Step+j+1));

			ppfConv[i][j] = sqrt((float)x*x+y*y);
		}
	}
	return 0;
}
