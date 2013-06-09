#include "stdafx.h"
#include "StencilArt.h"
#include "GlobalHelper.h"
 
extern char g_pcszPath[];

extern "C" __declspec(dllexport) void StencilArt(char *szPath, int bPreviewMode, int*iErrCode)
{
	GlobalHelper g_GH;
	g_GH.Init(szPath,iErrCode);
	if(*iErrCode >0)
		return;

	//g_GH.DebugOut("StencilArt: Begin",TRUE);
	
	IplImage *pImgSource;
	int iRet = g_GH.load_RGB(szPath,&pImgSource,bPreviewMode);
	if(!iRet)
	{
		//g_GH.DebugOut("StencilArt: LoadImage failed",FALSE);
		*iErrCode = 1;
		return;
	}

	clsStencilArt StencilArt;
	StencilArt.StencilArtMain(&pImgSource,bPreviewMode,iErrCode,g_GH);

	iRet = g_GH.save_RGB(szPath,pImgSource);
	if(!iRet)
	{
		//g_GH.DebugOut("StencilArt: save_RGB failed",FALSE);
		*iErrCode = 1;
		return;
	}

	g_GH.cvReleaseImageProxy(&pImgSource);
	//g_GH.DebugOut("StencilArt: End",TRUE);

}
clsStencilArt::clsStencilArt()
{
	
}
clsStencilArt::~clsStencilArt()
{
}
void clsStencilArt::StencilArtMain(IplImage **pImgSrc, int bPreviewMode, int*iErrCode,GlobalHelper &g_GH)
{
	pImgSource = *pImgSrc;
	if(!pImgSource)
	{
		//g_GH.DebugOut("StencilArtMain: Source Image NULL",FALSE);
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

	IplImage * pImgGrey = g_GH.cvCreateImageProxy(cvSize(iNC,iNR),pImgSource->depth,1);
	g_GH.cvCvtColorProxy(pImgSource,pImgGrey,CV_BGR2GRAY);
	g_GH.cvReleaseImageProxy(&pImgSource);
	pImgSource = pImgGrey;

	ApplyCanvas(g_GH);
	Sobel(g_GH);

	*pImgSrc = pImgDest;
	g_GH.cvReleaseImageProxy(&pImgSource);
	free(ppfConv[0]);free(ppfConv);


}

void clsStencilArt::ApplyCanvas(GlobalHelper &g_GH)
{
	
	char buff[1024];
	buff[0] = NULL;
	strcpy(buff,g_pcszPath);

	strcat(buff,"brick");

	IplImage *imgrgb;
	int iRet = g_GH.load_RGB(buff,&imgrgb,FALSE);
	if(!iRet)
	{
		//g_GH.DebugOut("clsStencilArt::ApplyCanvas - Failed to load canvas",FALSE);
		//g_GH.DebugOut(buff,FALSE);
		return;
	}

	pImgDest = g_GH.cvCreateImageProxy(cvSize(iNC,iNR),pImgSource->depth,3);
	g_GH.cvSetProxy(pImgDest,CV_RGB(255,255,255),0);

	int iWStep, iJStep;
	int iCanvasNR=0,iCanvasNC=0;		
	int iWStepPatch, iJStepPatch,tmp;
	for (int i=0;i<iNR;i++)
	{
		iWStep = i*pImgDest->widthStep;
		iWStepPatch = iCanvasNR*imgrgb->widthStep;

		for(int j=0;j<iNC;j++)
		{
			iJStep = j*3;
			iJStepPatch = iCanvasNC*3;

			tmp = *(uchar*)(pImgDest->imageData + iWStep + iJStep + 0);
			*(uchar*)(pImgDest->imageData + iWStep + iJStep + 0) = ((float)tmp*0.8) +
				*(uchar*)(imgrgb->imageData + iWStepPatch + iJStepPatch + 0)*0.2;

			tmp = *(uchar*)(pImgDest->imageData + iWStep + iJStep + 1);
			*(uchar*)(pImgDest->imageData + iWStep + iJStep + 1) = ((float)tmp*0.8) +
				*(uchar*)(imgrgb->imageData + iWStepPatch + iJStepPatch + 1)*0.2;

			tmp = *(uchar*)(pImgDest->imageData + iWStep + iJStep + 2);
			*(uchar*)(pImgDest->imageData + iWStep + iJStep + 2) = ((float)tmp*0.8) +
				*(uchar*)(imgrgb->imageData + iWStepPatch + iJStepPatch + 2)*0.2;

			iCanvasNC++;
			if(iCanvasNC>=imgrgb->width)
				iCanvasNC = 0;
		}
		iCanvasNC=0;
		iCanvasNR++;
		if(iCanvasNR>=imgrgb->height)
			iCanvasNR = 0;
	}
	


	g_GH.cvReleaseImageProxy(&imgrgb);

}
int clsStencilArt::Sobel(GlobalHelper &g_GH)
{
	int x,y;
	int i_1Step, iStep, ip1Step;
	for(int i=1;i<iNR-1;i++)
	{
		iStep = i*pImgSource->widthStep;
		i_1Step = (i-1)*pImgSource->widthStep;
		ip1Step = (i+1)*pImgSource->widthStep;
		for(int j=1;j<iNC-1;j++)
		{
			x = -1* (*(UCHAR*)(pImgSource->imageData+i_1Step+j-1))  + -2*(*(UCHAR*)(pImgSource->imageData+i_1Step+j)) + 
				-1*(*(UCHAR*)(pImgSource->imageData+i_1Step+j+1)) + (*(UCHAR*)(pImgSource->imageData+ip1Step+j-1)) + 
				2*(*(UCHAR*)(pImgSource->imageData+ip1Step+j)) + 1*(*(UCHAR*)(pImgSource->imageData+ip1Step+j+1));

			y = -1*(*(UCHAR*)(pImgSource->imageData+i_1Step+j-1)) + -2*(*(UCHAR*)(pImgSource->imageData+iStep+j-1)) + 
				-1*(*(UCHAR*)(pImgSource->imageData+ip1Step+j-1)) +(*(UCHAR*)(pImgSource->imageData+i_1Step+j+1)) + 
				2*(*(UCHAR*)(pImgSource->imageData+iStep+j+1)) + 1*(*(UCHAR*)(pImgSource->imageData+ip1Step+j+1));

			ppfConv[i][j] = sqrt((float)x*x+y*y);
		}
	}

	for(int i=0;i<iNR;i++)
	{
		for(int j=0;j<iNC;j++)
		{
			if(ppfConv[i][j]<0) ppfConv[i][j] = 0;
			else if(ppfConv[i][j]>255) ppfConv[i][j] = 255;


			if(ppfConv[i][j]>64)
			{
				*(UCHAR*)(pImgDest->imageData + i*pImgDest->widthStep +j*3 + 0) = ppfConv[i][j];
				*(UCHAR*)(pImgDest->imageData + i*pImgDest->widthStep +j*3 + 1) = ppfConv[i][j];
				*(UCHAR*)(pImgDest->imageData + i*pImgDest->widthStep +j*3 + 2) = ppfConv[i][j];
			}
		}
	}
	return 0;
}
