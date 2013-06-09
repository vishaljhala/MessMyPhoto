#include "stdafx.h"
#include "GoldEngross.h"
#include "GlobalHelper.h"
 
extern char g_pcszPath[];

extern "C" __declspec(dllexport) void GoldEngross(char *szPath, int iDepth, int*iErrCode)
{
	DWORD dwStart = GetTickCount();
	SYSTEMTIME stStart;
	GetLocalTime(&stStart);

	GlobalHelper g_GH;
	g_GH.Init(szPath,iErrCode);
	if(*iErrCode >0)
		return;

	//g_GH.DebugOut("GoldEngross: Begin",TRUE);
	
	IplImage *pImgSource;
	int iRet = g_GH.load_RGB(szPath,&pImgSource,false);
	if(!iRet)
	{
		//g_GH.DebugOut("GoldEngross: LoadImage failed",FALSE);
		*iErrCode = 1;
		return;
	}

	clsGoldEngross GoldEngross;
	GoldEngross.GoldEngrossMain(&pImgSource,iDepth,iErrCode,g_GH);

	iRet = g_GH.save_RGB(szPath,pImgSource);
	if(!iRet)
	{
		//g_GH.DebugOut("GoldEngross: save_RGB failed",FALSE);
		*iErrCode = 1;
		return;
	}

	g_GH.cvReleaseImageProxy(&pImgSource);

	DWORD dwEnd = GetTickCount();
	char pszTimec[1024];pszTimec[0] = 0;
	sprintf(pszTimec,"GoldenEngross,%d-%02d-%02d %02d:%02d:%02d,%ld\0",stStart.wYear,stStart.wMonth,stStart.wDay,stStart.wHour,
		stStart.wMinute, stStart.wSecond, dwEnd - dwStart);
	//sprintf(pszTimec,"GoldenEngross,%d-%02d-%02d %02d:%02d:%02d,%ld\0",stStart.wYear,stStart.wMonth,stStart.wDay,dwEnd - dwStart);

	g_GH.DebugOut(pszTimec);

	//g_GH.DebugOut("GoldEngross: End",TRUE);

}
clsGoldEngross::clsGoldEngross()
{
	
}
clsGoldEngross::~clsGoldEngross()
{
}
void clsGoldEngross::GoldEngrossMain(IplImage **pImgSrc, int iDepth, int*iErrCode,GlobalHelper &g_GH)
{
	pImgSource = *pImgSrc;
	if(!pImgSource)
	{
		//g_GH.DebugOut("GoldEngrossMain: Source Image NULL",FALSE);
		*iErrCode = 1;
		return;
	}

	iNC = pImgSource->width;
	iNR = pImgSource->height;
	
	//Create Sobel
	ppfSobelX = g_GH.FloatHeap2D(iNR,iNC);
	//ppfSobelY = g_GH.FloatHeap2D(iNR,iNC);
	pImgGrey = g_GH.cvCreateImageProxy(cvSize(pImgSource->width,pImgSource->height),IPL_DEPTH_8U,1);
	g_GH.cvCvtColorProxy(pImgSource,pImgGrey,CV_BGR2GRAY);
	Sobel(ppfSobelX,g_GH);
	for(int i=0;i<iNR;i++)
	{
		for(int j=0;j<iNC;j++)
		{
			if(ppfSobelX[i][j] <0)
				ppfSobelX[i][j] = 0;
			else if(ppfSobelX[i][j] >255)
				ppfSobelX[i][j] = 255;
				
			//ppfSobelX[i][j] = ppfSobelX[i][j] ;
			
			*(UCHAR*)(pImgGrey->imageData + i*pImgGrey->widthStep +j) = ppfSobelX[i][j];
		}
	}
	free(ppfSobelX[0]);free(ppfSobelX);
	
	//aPPLY cNAVANS
	pImgCanvas = g_GH.cvCreateImageProxy(cvSize(pImgSource->width,pImgSource->height),IPL_DEPTH_8U,3);
	g_GH.cvSetProxy(pImgCanvas,CV_RGB(255,255,255),0);
	ApplyCanvas(g_GH);
	g_GH.cvReleaseImageProxy(&pImgDest);
	
	//Engross
	Engross( iDepth, g_GH);

	*pImgSrc = pImgCanvas;
	
	g_GH.cvReleaseImageProxy(&pImgSource);
	g_GH.cvReleaseImageProxy(&pImgDest);
	g_GH.cvReleaseImageProxy(&pImgGrey);

}
int clsGoldEngross::Sobel(float **ppfSobelX,GlobalHelper &g_GH)
{
	int x,y;
	int i_1,i1,iw;
	for(int i=1;i<iNR-1;i++)
	{
		i_1 = (i-1)*pImgGrey->widthStep;
		i1 = (i+1)*pImgGrey->widthStep;
		iw = (i)*pImgGrey->widthStep;

		for(int j=1;j<iNC-1;j++)
		{

			/*x = -1*(*(UCHAR*)(pImgGrey->imageData + i_1 +j-1)) + 
				-2*(*(UCHAR*)(pImgGrey->imageData + i_1 +j)) + 
				-1*(*(UCHAR*)(pImgGrey->imageData + i_1 +j+1)) +
				*(UCHAR*)(pImgGrey->imageData + i1 +j-1) + 
				2*(*(UCHAR*)(pImgGrey->imageData + i1 +j)) + 
				1*(*(UCHAR*)(pImgGrey->imageData + i1 +j+1));*/

			x = -1*(*(UCHAR*)(pImgGrey->imageData + i_1 +j-1)) + 
				-2*(*(UCHAR*)(pImgGrey->imageData + iw +j-1)) + 
				-1*(*(UCHAR*)(pImgGrey->imageData + i1 +j-1)) +
				*(UCHAR*)(pImgGrey->imageData + i_1 +j+1) + 
				2*(*(UCHAR*)(pImgGrey->imageData + iw +j+1)) + 
				1*(*(UCHAR*)(pImgGrey->imageData + i1 +j+1));

			//ppfSobelY[i][j] = y;
			ppfSobelX[i][j] = x;
			//ppfConv[i][j] = sqrt((float)x*x+y*y);
		}
	}
	return 0;
}	

void clsGoldEngross::ApplyCanvas(GlobalHelper &g_GH)
{
	
	char buff[1024];
	buff[0] = NULL;
	strcpy(buff,g_pcszPath);

	strcat(buff,"coin");

	IplImage *imgrgb;
	int iRet = g_GH.load_RGB(buff,&imgrgb,FALSE);
	if(!iRet)
	{
		//g_GH.DebugOut("clsGoldEngross::ApplyCanvas - Failed to load canvas",FALSE);
		//g_GH.DebugOut(buff,FALSE);
		return;
	}
	int iSmall,iInit=0,jInit=0;
	if(iNR<iNC) 
	{
		iSmall = iNR;
		jInit = abs(iNR-iNC)/2.0;
	}
	else
	{
		iSmall = iNC;
		iInit = abs(iNR-iNC)/2.0;
	}



	pImgDest = g_GH.cvCreateImageProxy(cvSize(iSmall,iSmall),pImgSource->depth,pImgSource->nChannels);
	g_GH.cvResizeProxy(imgrgb,pImgDest,1);

	int iWStep, iJStep;
	int tmp;
	for (int i=iInit;i<iNR-iInit-1;i++)
	{
		iWStep = i*pImgCanvas->widthStep;
		for(int j=jInit;j<iNC-jInit-1;j++)
		{
			iJStep = j*3;
			*(uchar*)(pImgCanvas->imageData + iWStep + iJStep + 0) = 
			*(uchar*)(pImgDest->imageData + (i-iInit)*pImgDest->widthStep + (j-jInit)*3 + 0);

			*(uchar*)(pImgCanvas->imageData + iWStep + iJStep + 1) = 
			*(uchar*)(pImgDest->imageData + (i-iInit)*pImgDest->widthStep + (j-jInit)*3 + 1);

			*(uchar*)(pImgCanvas->imageData + iWStep + iJStep + 2) = 
			*(uchar*)(pImgDest->imageData + (i-iInit)*pImgDest->widthStep + (j-jInit)*3 + 2);
		}
	}
	


	g_GH.cvReleaseImageProxy(&imgrgb);

}
void clsGoldEngross::Engross(int iDetails,GlobalHelper &g_GH)
{
	float fPercent;
	int iSmall,iInit=0,jInit=0;
	
	if(iNR<iNC) 
		fPercent =  iNR/(float)iNC;
	else
		fPercent =  iNC/(float)iNR;
		
	int iW = iNC*fPercent;
	int iH = iNR*fPercent;

	if(iDetails<0) iDetails = 0;
	if(iDetails>50) iDetails = 50;
	float fDetails = 1 + iDetails/100.0;

	iInit = abs(iNR-iH)/2.0;
	jInit = abs(iNC-iW)/2.0;

	pImgDest = g_GH.cvCreateImageProxy(cvSize(iW,iH),pImgGrey->depth,1);
	g_GH.cvResizeProxy(pImgGrey,pImgDest,1);


	int iWStep, iJStep;
	int tmp;
	for (int i=iInit;i<iNR-iInit-1;i++)
	{
		iWStep = i*pImgCanvas->widthStep;
		for(int j=jInit;j<iNC-jInit-1;j++)
		{
			if(*(UCHAR*)(pImgDest->imageData + (i-iInit)*pImgDest->widthStep + (j-jInit))<255
				&& *(uchar*)(pImgCanvas->imageData + iWStep + j*3 + 2) ==244 )
			{
				iJStep = j*3;
			
				/**(uchar*)(pImgCanvas->imageData + iWStep + iJStep + 0) = 
				*(uchar*)(pImgDest->imageData + (i-iInit)*pImgDest->widthStep + (j-jInit));

				*(uchar*)(pImgCanvas->imageData + iWStep + iJStep + 1) = 
				*(uchar*)(pImgDest->imageData + (i-iInit)*pImgDest->widthStep + (j-jInit)*3 + 1);

				*(uchar*)(pImgCanvas->imageData + iWStep + iJStep + 2) = 
				*(uchar*)(pImgDest->imageData + (i-iInit)*pImgDest->widthStep + (j-jInit)*3 + 2);
				*/
				tmp = *(uchar*)(pImgCanvas->imageData + iWStep + iJStep + 0);
				tmp -= *(uchar*)(pImgDest->imageData + (i-iInit)*pImgDest->widthStep + (j-jInit))*fDetails;
				if(tmp<0)tmp=0;
				*(uchar*)(pImgCanvas->imageData + iWStep + iJStep + 0) = tmp;

				tmp = *(uchar*)(pImgCanvas->imageData + iWStep + iJStep + 1);
				tmp -= *(uchar*)(pImgDest->imageData + (i-iInit)*pImgDest->widthStep + (j-jInit))*fDetails;
				if(tmp<0)tmp=0;
				*(uchar*)(pImgCanvas->imageData + iWStep + iJStep + 1) = tmp;

				tmp = *(uchar*)(pImgCanvas->imageData + iWStep + iJStep + 2);
				tmp -= *(uchar*)(pImgDest->imageData + (i-iInit)*pImgDest->widthStep + (j-jInit))*fDetails;
				if(tmp<0)tmp=0;
				*(uchar*)(pImgCanvas->imageData + iWStep + iJStep + 2) = tmp;

			}
		}
	}

}
