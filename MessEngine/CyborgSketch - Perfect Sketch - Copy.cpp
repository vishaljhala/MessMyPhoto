#include "stdafx.h"
#include "cyborgsketch.h"
#include "GlobalHelper.h"


extern "C" __declspec(dllexport) void CyborgSketch(char *szPath, int iScanType, int bPreviewMode, int*iErrCode)
{
	
	GlobalHelper g_GH;
	g_GH.Init(szPath,iErrCode);
	if(*iErrCode >0)
		return;

	g_GH.DebugOut("\r\nCyhorgSketch",TRUE);

	IplImage *pImgSource;
	int iRet = g_GH.load_RGB(szPath,&pImgSource,bPreviewMode);
	if(!iRet)
	{
		g_GH.DebugOut("OldPhoto: LoadImage failed",FALSE);
		*iErrCode = 1;
		return;
	}


	clsCyborgSketch cyborg;
	cyborg.CyborgFilter(&pImgSource,iScanType,iErrCode,g_GH);
	
	iRet = g_GH.save_RGB(szPath,pImgSource);
	if(!iRet)
	{
		g_GH.DebugOut("OldPhoto: save_RGB failed",FALSE);
		*iErrCode = 1;
		return;
	}

	g_GH.cvReleaseImageProxy(&pImgSource);

	g_GH.DebugOut("",TRUE);

	*iErrCode = 0;
}
clsCyborgSketch::clsCyborgSketch()
{
	
}
clsCyborgSketch::~clsCyborgSketch()
{
}

int clsCyborgSketch::CyborgFilter(IplImage **pImgSrc, int iScanType, int*iErrCode,GlobalHelper &g_GH)
{
	pImgSource = *pImgSrc;
	m_iScanType =iScanType;
	
	if(!pImgSource)
	{
		g_GH.DebugOut("OldPhotoMain: Source Image NULL",FALSE);
		*iErrCode = 1;
		return 0 ;
	}

	iNC = pImgSource->width;
	iNR = pImgSource->height;
	
	ppfConv = g_GH.FloatHeap2D(iNR,iNC);
	if(!ppfConv)
	{
		g_GH.DebugOut("clsSobel::Sobel - FloatHelp2D failed",FALSE);
		return 0;
	}
	/*g_GH.cvSmoothProxy(pImgSrcHSV,pImgSource,2,3,0,0.0,0.0);
	g_GH.cvCopyProxy(pImgSource,pImgSrcHSV,0);
	*/
	pImgSrcHSV = g_GH.cvCreateImageProxy(cvSize(pImgSource->width,pImgSource->height),IPL_DEPTH_8U,1);
	g_GH.cvCvtColorProxy(pImgSource,pImgSrcHSV,CV_BGR2GRAY);

	Sobel(g_GH);

	g_GH.cvReleaseImageProxy(&pImgSrcHSV);

	pImgSrcHSV = g_GH.convertImageRGBtoHSV(pImgSource);
	if(!pImgSrcHSV)
	{
		g_GH.DebugOut("OldPhoto: ConvertImage failed",FALSE);
		*iErrCode = 1;
		return 0;
	}
	ApplyCyborg(g_GH);
	ApplyCanvas(g_GH);

	g_GH.cvReleaseImageProxy(&pImgSource);
	pImgSource = g_GH.convertImageHSVtoRGB(pImgSrcHSV);
	if(!pImgSource)
	{
		g_GH.DebugOut("OldPhoto: convertImage failed",FALSE);
		*iErrCode = 1;
		return 0;
	}
	*pImgSrc = pImgSource;

	g_GH.cvReleaseImageProxy(&pImgSrcHSV);

	free(ppfConv[0]);free(ppfConv);

}
int clsCyborgSketch::Sobel(GlobalHelper &g_GH)
{
	int x,y,iBitTmp;
	int i_1Step, iStep, ip1Step;
	for(int i=1;i<iNR-1;i++)
	{
		iStep = i*pImgSrcHSV->widthStep;
		i_1Step = (i-1)*pImgSrcHSV->widthStep;
		ip1Step = (i+1)*pImgSrcHSV->widthStep;
		for(int j=1;j<iNC-1;j++)
		{
			x = -(*(UCHAR*)(pImgSrcHSV->imageData+i_1Step+j-1))    
				- (*(UCHAR*)(pImgSrcHSV->imageData+i_1Step+j+1)) + (*(UCHAR*)(pImgSrcHSV->imageData+ip1Step+j-1))  + (*(UCHAR*)(pImgSrcHSV->imageData+ip1Step+j+1));
			iBitTmp = (*(UCHAR*)(pImgSrcHSV->imageData+i_1Step+j))<<1;
			x-=iBitTmp;
			iBitTmp = (*(UCHAR*)(pImgSrcHSV->imageData+ip1Step+j))<<1;
			x+=iBitTmp;

			y = -(*(UCHAR*)(pImgSrcHSV->imageData+i_1Step+j-1))   + 
				-(*(UCHAR*)(pImgSrcHSV->imageData+ip1Step+j-1)) +(*(UCHAR*)(pImgSrcHSV->imageData+i_1Step+j+1)) + 
				 + (*(UCHAR*)(pImgSrcHSV->imageData+ip1Step+j+1));
			iBitTmp = (*(UCHAR*)(pImgSrcHSV->imageData+iStep+j-1))<<1;
			y-=iBitTmp;
			iBitTmp = (*(UCHAR*)(pImgSrcHSV->imageData+iStep+j+1))<<1;
			x+=iBitTmp;
				
				ppfConv[i][j] = sqrt((float)x*x+y*y);
		}
	}

	for(int i=0;i<iNR;i++)
	{
		for(int j=0;j<iNC;j++)
		{
			if(ppfConv[i][j]<0) ppfConv[i][j] = 0;
			else if(ppfConv[i][j]>255) ppfConv[i][j] = 255;
		}
	}
	return 0;
}
void clsCyborgSketch::ApplyCyborg(GlobalHelper &g_GH)
{

	int iColor;
	int iWStep, iJStep;
	int tmp;

	if(m_iScanType == 0)
		iColor = 94;
	else if(m_iScanType == 1)
		iColor = 1;
	else
		iColor = 121; 

	for (int i=0;i<iNR;i++)
	{
		iWStep = i*pImgSrcHSV->widthStep;
		for(int j=0;j<iNC;j++)
		{
			iJStep = j*3;
			*(uchar*)(pImgSrcHSV->imageData + iWStep + iJStep + 0) = iColor;
			*(uchar*)(pImgSrcHSV->imageData + iWStep + iJStep + 1) = 255-ppfConv[i][j];
			*(uchar*)(pImgSrcHSV->imageData + iWStep + iJStep + 2) = ppfConv[i][j];
		}
	}
}
void clsCyborgSketch::ApplyCanvas(GlobalHelper &g_GH)
{
	int iWStep, iJStep;
	int tmp;
	int bFlag = 0;
	for (int i=0;i<iNR;i++)
	{
		iWStep = i*pImgSrcHSV->widthStep;
		if(i%5==0) 
			if( bFlag) bFlag = 0;
			else bFlag = 1;
		if(bFlag)
		for(int j=0;j<iNC;j++)
		{
			iJStep = j*3;
			//*(uchar*)(pImgSrcHSV->imageData + iWStep + iJStep + 2) = ppfConv[i][j];
			*(uchar*)(pImgSrcHSV->imageData + iWStep + iJStep + 2) |=0x40;
			
		}
	}

}

