#include "stdafx.h"
#include "cyborgsketch.h"
#include "GlobalHelper.h"

extern "C" __declspec(dllexport) void CyborgSketch(char *szPath, int iScanType, int iGridWidth, int iOrientation, int*iErrCode)
{
	
	DWORD dwStart = GetTickCount();
	SYSTEMTIME stStart;
	GetLocalTime(&stStart);

	GlobalHelper g_GH;
	g_GH.Init(szPath,iErrCode);
	if(*iErrCode >0)
		return;


	IplImage *pImgSource;
	int iRet = g_GH.load_RGB(szPath,&pImgSource,false);
	if(!iRet)
	{
		*iErrCode = 1;
		return;
	}


	clsCyborgSketch cyborg;
	cyborg.CyborgFilter(&pImgSource,g_GH.GetHue(iScanType), iGridWidth,  iOrientation,iErrCode,g_GH);
	
	iRet = g_GH.save_RGB(szPath,pImgSource);
	if(!iRet)
	{
		*iErrCode = 1;
		return;
	}

	g_GH.cvReleaseImageProxy(&pImgSource);

	DWORD dwEnd = GetTickCount();
	char pszTimec[1024];pszTimec[0] = 0;
	sprintf(pszTimec,"Cyborg,%d-%02d-%02d %02d:%02d:%02d,%ld\0",stStart.wYear,stStart.wMonth,stStart.wDay,stStart.wHour,
		stStart.wMinute, stStart.wSecond, dwEnd - dwStart);
	//sprintf(pszTimec,"Cyborg,%d-%02d-%02d %02d:%02d:%02d,%ld\0",stStart.wYear,stStart.wMonth,stStart.wDay,dwEnd - dwStart);

	g_GH.DebugOut(pszTimec);

	*iErrCode = 0;
}
clsCyborgSketch::clsCyborgSketch()
{
	
}
clsCyborgSketch::~clsCyborgSketch()
{
}

int clsCyborgSketch::CyborgFilter(IplImage **pImgSrc, int iScanType, int iGridWidth, int iOrientation, int*iErrCode,GlobalHelper &g_GH)
{
	pImgSource = *pImgSrc;
	m_iScanType =iScanType;
	
	if(!pImgSource)
	{
		//g_GH.DebugOut("OldPhotoMain: Source Image NULL",FALSE);
		*iErrCode = 1;
		return 0 ;
	}

	iNC = pImgSource->width;
	iNR = pImgSource->height;
	
	ppfConv = g_GH.IntHeap2D(iNR,iNC);
	if(!ppfConv)
	{
		//g_GH.DebugOut("clsSobel::Sobel - FloatHelp2D failed",FALSE);
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
		//g_GH.DebugOut("OldPhoto: ConvertImage failed",FALSE);
		*iErrCode = 1;
		return 0;
	}
	ApplyCyborg(g_GH);
	if(iOrientation>0)
		ApplyCanvas(g_GH,iGridWidth,iOrientation);

	g_GH.cvReleaseImageProxy(&pImgSource);
	pImgSource = g_GH.convertImageHSVtoRGB(pImgSrcHSV);
	if(!pImgSource)
	{
		//g_GH.DebugOut("OldPhoto: convertImage failed",FALSE);
		*iErrCode = 1;
		return 0;
	}
	*pImgSrc = pImgSource;

	g_GH.cvReleaseImageProxy(&pImgSrcHSV);

	free(ppfConv[0]);free(ppfConv);

}
int clsCyborgSketch::Sobel(GlobalHelper &g_GH)
{
	int x,y;
	int i_1Step, iStep, ip1Step;
	for(int i=1;i<iNR-1;i++)
	{
		iStep = i*pImgSrcHSV->widthStep +(int) pImgSrcHSV->imageData;
		i_1Step = (i-1)*pImgSrcHSV->widthStep+(int) pImgSrcHSV->imageData;
		ip1Step = (i+1)*pImgSrcHSV->widthStep+(int) pImgSrcHSV->imageData;
		for(int j=1;j<iNC-1;j++)
		{
			x = -(*(UCHAR*)(i_1Step+j-1)) - (*(UCHAR*)(i_1Step+j))*2
				- (*(UCHAR*)(i_1Step+j+1)) + (*(UCHAR*)(ip1Step+j-1))  + 
				(*(UCHAR*)(ip1Step+j))*2 + (*(UCHAR*)(ip1Step+j+1));
			

			y = -(*(UCHAR*)(i_1Step+j-1))   - (*(UCHAR*)(iStep+j-1))*2 
				-(*(UCHAR*)(ip1Step+j-1)) +(*(UCHAR*)(i_1Step+j+1)) + 
				 (*(UCHAR*)(iStep+j+1))*2 + (*(UCHAR*)(ip1Step+j+1));
				
				ppfConv[i][j] = abs(x)+abs(y);//sqrt((float)x*x+y*y);
		}
	}

	for(int i=0;i<iNR;i++)
	{
		for(int j=0;j<iNC;j++)
		{
			if(ppfConv[i][j]>255) ppfConv[i][j] = 255;
		}
	}
	return 0;
}
void clsCyborgSketch::ApplyCyborg(GlobalHelper &g_GH)
{

	//int iColor;
	int iWStep, iJStep;
	int tmp;

	/*if(m_iScanType == 0)
		iColor = 94;
	else if(m_iScanType == 1)
		iColor = 1;
	else
		iColor = 121; 
		*/
	for (int i=0;i<iNR;i++)
	{
		iWStep = i*pImgSrcHSV->widthStep + (int)pImgSrcHSV->imageData;
		for(int j=0;j<iNC;j++)
		{
			iJStep = j*3;
			*(uchar*)(iWStep + iJStep + 0) = m_iScanType;
			*(uchar*)(iWStep + iJStep + 1) = ppfConv[i][j]^255;
			*(uchar*)(iWStep + iJStep + 2) = ppfConv[i][j];
		}
	}
}
void clsCyborgSketch::ApplyCanvas(GlobalHelper &g_GH, int iGridWidth, int iOrientation)
{
	int iWStep, iJStep;
	int tmp;
	int bFlag = 0;
	
	if(iOrientation==1)
		for (int i=0;i<iNR;i++)
		{
			iWStep = i*pImgSrcHSV->widthStep +(int)pImgSrcHSV->imageData ;
			if(i%iGridWidth==0) 
				if( bFlag) bFlag = 0;
				else bFlag = 1;
			if(bFlag)
			for(int j=0;j<iNC;j++)
			{
				iJStep = j*3;
				//*(uchar*)(pImgSrcHSV->imageData + iWStep + iJStep + 2) = ppfConv[i][j];
				*(uchar*)(iWStep + iJStep + 2) |=0x40;
			
			}
		}
	if(iOrientation == 2)
		for (int j=0;j<iNC;j++)
		{
			iJStep = j*3;
			if(j%iGridWidth==0) 
				if( bFlag) bFlag = 0;
				else bFlag = 1;
			if(bFlag)
			for(int i=0;i<iNR;i++)
			{
				//*(uchar*)(pImgSrcHSV->imageData + iWStep + iJStep + 2) = ppfConv[i][j];
				iWStep = i*pImgSrcHSV->widthStep;
				*(uchar*)(pImgSrcHSV->imageData + iWStep + iJStep + 2) |=0x40;
			
			}
		}

}

