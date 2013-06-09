#include "stdafx.h"
#include "Perfectsketch.h"
#include "GlobalHelper.h"


extern "C" __declspec(dllexport) void PerfectSketch(char *szPath, int iDetails, int bPreviewMode, int*iErrCode)
{
	DWORD dwStart = GetTickCount();
	SYSTEMTIME stStart;
	GetLocalTime(&stStart);
	
	GlobalHelper g_GH;
	g_GH.Init(szPath,iErrCode);
	if(*iErrCode >0)
		return;

	//g_GH.DebugOut("\r\nCyhorgSketch",TRUE);

	IplImage *pImgSource;
	int iRet = g_GH.load_RGB(szPath,&pImgSource,bPreviewMode);
	if(!iRet)
	{
		//g_GH.DebugOut("OldPhoto: LoadImage failed",FALSE);
		*iErrCode = 1;
		return;
	}


	clsPerfectSketch cyborg;
	cyborg.SketchFilter(&pImgSource,iDetails,iErrCode,g_GH);
	
	iRet = g_GH.save_RGB(szPath,pImgSource);
	if(!iRet)
	{
		//g_GH.DebugOut("OldPhoto: save_RGB failed",FALSE);
		*iErrCode = 1;
		return;
	}

	g_GH.cvReleaseImageProxy(&pImgSource);

	//g_GH.DebugOut("",TRUE);

	*iErrCode = 0;

	DWORD dwEnd = GetTickCount();
	char pszTimec[1024];pszTimec[0] = 0;
	sprintf(pszTimec,"PerfectSketch,%d-%02d-%02d %02d:%02d:%02d,%ld\0",stStart.wYear,stStart.wMonth,stStart.wDay,stStart.wHour,
		stStart.wMinute, stStart.wSecond, dwEnd - dwStart);
	//sprintf(pszTimec,"PerfectSketch,%d-%02d-%02d %02d:%02d:%02d,%ld\0",stStart.wYear,stStart.wMonth,stStart.wDay,dwEnd - dwStart);

	g_GH.DebugOut(pszTimec);

}
clsPerfectSketch::clsPerfectSketch()
{
	
}
clsPerfectSketch::~clsPerfectSketch()
{
}

int clsPerfectSketch::SketchFilter(IplImage **pImgSrc, int iDetails, int*iErrCode,GlobalHelper &g_GH)
{
	pImgSource = *pImgSrc;
	
	if(!pImgSource)
	{
		//g_GH.DebugOut("OldPhotoMain: Source Image NULL",FALSE);
		*iErrCode = 1;
		return 0 ;
	}
	if(iDetails<0) iDetails = 0;
	if(iDetails>50) iDetails = 50;
	m_fDetails =0.5 + iDetails/100.0;

	iNC = pImgSource->width;
	iNR = pImgSource->height;
	
	ppfConv = g_GH.IntHeap2D(iNR,iNC);
	ppfConvGrad = g_GH.IntHeap2D(iNR,iNC);

	if(!ppfConv)
	{
		//g_GH.DebugOut("clsSobel::Sobel - FloatHelp2D failed",FALSE);
		return 0;
	}
	pImgSrcHSV = g_GH.cvCreateImageProxy(cvSize(pImgSource->width,pImgSource->height),IPL_DEPTH_8U,1);
	g_GH.cvCvtColorProxy(pImgSource,pImgSrcHSV,CV_BGR2GRAY);

	Sobel(g_GH);
	DrawSketch(g_GH);

	g_GH.cvReleaseImageProxy(&pImgSource);
	*pImgSrc = pImgSrcHSV;

	free(ppfConv[0]);free(ppfConv);
	free(ppfConvGrad[0]);free(ppfConvGrad);

}
int clsPerfectSketch::Sobel(GlobalHelper &g_GH)
{
	int x,y,iBitTmp;
	int i_1Step, iStep, ip1Step;

	iMinGrad = 100000; iMaxGrad=0;

	for(int i=1;i<iNR-1;i++)
	{
		iStep = i*pImgSrcHSV->widthStep + (int)pImgSrcHSV->imageData;
		i_1Step = (i-1)*pImgSrcHSV->widthStep + (int)pImgSrcHSV->imageData;
		ip1Step = (i+1)*pImgSrcHSV->widthStep + (int)pImgSrcHSV->imageData;

		for(int j=1;j<iNC-1;j++)
		{
			x = -(*(UCHAR*)(i_1Step+j-1))    
				- (*(UCHAR*)(i_1Step+j+1)) + (*(UCHAR*)(ip1Step+j-1))  + (*(UCHAR*)(ip1Step+j+1));
			iBitTmp = (*(UCHAR*)(i_1Step+j))<<1;
			x-=iBitTmp;
			iBitTmp = (*(UCHAR*)(ip1Step+j))<<1;
			x+=iBitTmp;


			y = -(*(UCHAR*)(i_1Step+j-1))   + 
				-(*(UCHAR*)(ip1Step+j-1)) +(*(UCHAR*)(i_1Step+j+1)) + 
				 + (*(UCHAR*)(ip1Step+j+1));
			iBitTmp = (*(UCHAR*)(iStep+j-1))<<1;
			y-=iBitTmp;
			iBitTmp = (*(UCHAR*)(iStep+j+1))<<1;
			y+=iBitTmp;

			ppfConv[i][j] = sqrt((float)x*x+y*y);
			//ppfConv[i][j] = abs(x)+abs(y);

			iBitTmp = (*(UCHAR*)(iStep+j)*m_fDetails) + (*(UCHAR*)(i_1Step+j)>>1) + (*(UCHAR*)(ip1Step+j)>>1);
			x+=iBitTmp;
			iBitTmp = (*(UCHAR*)(iStep+j)*m_fDetails) + (*(UCHAR*)(iStep+j-1)>>1)+ (*(UCHAR*)(iStep+j+1)>>1);
			y+=iBitTmp;

			ppfConvGrad[i][j] = sqrt((float)x*x+y*y);
			//ppfConvGrad[i][j] = abs(x)+abs(y);
				
		}
	}
	
	for(int i=0;i<iNR;i++)
	{
		for(int j=0;j<iNC;j++)
		{
			if(ppfConv[i][j]>255) ppfConv[i][j] = 255;
			ppfConv[i][j] = 255-ppfConv[i][j];
			if(ppfConvGrad[i][j]>255) ppfConvGrad[i][j] = 255;

		}
	}
	return 0;
}
void clsPerfectSketch::DrawSketch(GlobalHelper &g_GH)
{

	int iColor;
	int iWStep, iJStep;
	int tmp,tmp1;

	for (int i=0;i<iNR;i++)
	{
		iWStep = i*pImgSrcHSV->widthStep;
		for(int j=0;j<iNC;j++)
		{
			//if(ppfConv[i][j]>255) ppfConv[i][j] = 255;
			//if(ppfConv[i][j]<128 && ppfConv[i][j]<ppfConvGrad[i][j])
				//*(uchar*)(pImgSrcHSV->imageData + iWStep + j) = ppfConv[i][j];
			//else
				*(uchar*)(pImgSrcHSV->imageData + iWStep + j) = ppfConvGrad[i][j];
			//if(*(uchar*)(pImgSrcHSV->imageData + iWStep + j)>192)
				//*(uchar*)(pImgSrcHSV->imageData + iWStep + j) = 255; 
			
		}
	}
}

			/*tmp = *(uchar*)(pImgSrcHSV->imageData + iWStep + j);
			//ppfConv[i][j] = 0;

			if(tmp>iCutOffPix && ppfConv[i][j]>iCutOffGrad)
			{
				tmp1 = tmp-ppfConv[i][j];
				if(tmp1<0) tmp1 = 0;
				*(uchar*)(pImgSrcHSV->imageData + iWStep + j) = tmp1;
			}
			else if(tmp>iCutOffPix && ppfConv[i][j]<=iCutOffGrad)
			{
				*(uchar*)(pImgSrcHSV->imageData + iWStep + j) = 255;
			}
			else if(tmp<=iCutOffPix && ppfConv[i][j]>iCutOffGrad)
			{
				*(uchar*)(pImgSrcHSV->imageData + iWStep + j) = 0;
			}
			else if(tmp<=iCutOffPix && ppfConv[i][j]<=iCutOffGrad)
			{
				tmp1 = tmp+ppfConv[i][j];
				if(tmp1>255) tmp1 = 255;
				*(uchar*)(pImgSrcHSV->imageData + iWStep + j) = tmp1;
			}*/

/*
void clsPerfectSketch::Histogram(GlobalHelper &g_GH)
{
	int tmpS,tmpD;
	
	for(int i=0;i<256;i++)
		iHistPix[i][0] = iHistPix[i][1] = iHistPix[i][2] =0;

	for (int i=0;i<iNR;i++)
	{
		for(int j=0;j<iNC;j++)
		{
			iHistPix[ppfConv[i][j]][0]++;
		}
	}
	int iMinCDFSat = iNC*iNR;
	for(int i=0;i<256;i++)
	{
		if(i>0)
		{
			iHistPix[i][1]=iHistPix[i][0]+iHistPix[i-1][1];
		}
		else
		{
			iHistPix[i][1]=iHistPix[i][0];
		}
		if(iHistPix[i][1]<iMinCDFSat)
			iMinCDFSat = iHistPix[i][1];

	}
	float iFactorSat = 255.0/((iNC*iNR)-iMinCDFSat);

	for(int i=0;i<256;i++)
	{
		iHistPix[i][2] = (iHistPix[i][1]-iMinCDFSat)*iFactorSat;
	}

}

void clsPerfectSketch::Normalize(GlobalHelper &g_GH)
{
	for (int i=0;i<iNR;i++)
	{
		for(int j=0;j<iNC;j++)
		{
			if(ppfConv[i][j]>64)
			ppfConv[i][j] = ppfConv[i][j]*255/iMaxGrad;
		}
	}
}

void clsPerfectSketch::Histogram(GlobalHelper &g_GH)
{
	int iWStep, iJStep;
	int tmp;
	
	for(int i=0;i<256;i++)
		iHistPix[i] = iHistGrad[i] = 0;

	for(int i=0;i<iNR;i++)
	{
		iWStep = i*pImgSrcHSV->widthStep;
		for(int j=0;j<iNC;j++)
		{

			iHistPix[*(uchar*)(pImgSrcHSV->imageData + iWStep + j)]++; 
			iHistGrad[ppfConv[i][j]]++;
		}
	}
	int iTotPix = iNC*iNR;
	int iCutOffMark = (float)iTotPix*0.5;
	int iTemp=0;
	for(int i=0;i<256;i++)
	{
		iTemp+=iHistPix[i];
		if(iTemp>=iCutOffMark)
		{
			iCutOffPix = i;
			break;
		}
	}

	iCutOffMark = (float)iTotPix*0.75;
	iTemp=0;
	for(int i=0;i<256;i++)
	{
		iTemp+=iHistGrad[i];
		if(iTemp>=iCutOffMark)
		{
			iCutOffGrad = i;
			break;
		}
	}



}*/
