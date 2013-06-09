#include "stdafx.h"
#include "MotionBlur.h"
#include "GlobalHelper.h"
#define PI 3.1415926535


extern "C" __declspec(dllexport) void MotionFilter(char *szPath,int iFilterSize, int iAngle, int*iErrCode)
{
	DWORD dwStart = GetTickCount();
	SYSTEMTIME stStart;
	GetLocalTime(&stStart);

	GlobalHelper g_GH;
	g_GH.Init(szPath,iErrCode);
	if(*iErrCode >0)
		return;

	//g_GH.DebugOut("MotionBlur: Begin",TRUE);

	IplImage *pImgSource;
	int iRet = g_GH.load_RGB(szPath,&pImgSource,FALSE);
	if(!iRet)
	{
		//g_GH.DebugOut("MotionBlur: LoadImage failed",FALSE);
		*iErrCode = 1;
		return;
	}
	clsMotionBlur MotionBlur;
	MotionBlur.MotionBlurMain(&pImgSource,iFilterSize, iAngle, 
		 iErrCode,g_GH);
	
	iRet = g_GH.save_RGB(szPath,pImgSource);
	if(!iRet)
	{
		//g_GH.DebugOut("MotionBlur: save_RGB failed",FALSE);
		*iErrCode = 1;
		return;
	}

	g_GH.cvReleaseImageProxy(&pImgSource);

	DWORD dwEnd = GetTickCount();
	char pszTimec[1024];pszTimec[0] = 0;
	sprintf(pszTimec,"MotionBlur,%d-%02d-%02d %02d:%02d:%02d,%ld\0",stStart.wYear,stStart.wMonth,stStart.wDay,stStart.wHour,
		stStart.wMinute, stStart.wSecond, dwEnd - dwStart);
	//sprintf(pszTimec,"Cyborg,%d-%02d-%02d %02d:%02d:%02d,%ld\0",stStart.wYear,stStart.wMonth,stStart.wDay,dwEnd - dwStart);

	g_GH.DebugOut(pszTimec);

	*iErrCode = 0;

	
}

clsMotionBlur::clsMotionBlur()
{
}
clsMotionBlur::~clsMotionBlur()
{
}
void clsMotionBlur::MotionBlurMain(IplImage **pImgSrc,int iFilterSize, int iAngle, 
	  int*iErrCode,GlobalHelper &g_GH)
{
	pImgSource = *pImgSrc;
	m_iFilterSize =iFilterSize;
	m_iAngle = iAngle;

	if(!pImgSource)
	{
		//g_GH.DebugOut("MotionBlur: Source Image NULL",FALSE);
		*iErrCode = 1;
		return;
	}

	iNC = pImgSource->width;
	iNR = pImgSource->height;

	pImgDest = g_GH.cvCreateImageProxy(cvSize(pImgSource->width,pImgSource->height),IPL_DEPTH_8U,3);
	g_GH.cvCopyProxy(pImgSource,pImgDest,0);
	//g_GH.cvCvtColorProxy(pImgSource,pImgDest,CV_BGR2GRAY);

	int ** ppfGradXY1;//, **ppfGradXY2;
	ppfGradXY1 = g_GH.IntHeap2D(iNR,iNC);
	if(!ppfGradXY1)
	{
		//g_GH.DebugOut("MotionBlur: FloatHepa2D failed",FALSE);
		*iErrCode = 1;
		return;
	}
	/*
	ppfGradXY2 = g_GH.FloatHeap2D(iNR,iNC);
	if(!ppfGradXY2)
	{
		g_GH.DebugOut("MotionBlur: FloatHepa2D failed",FALSE);
		*iErrCode = 1;
		return;
	}
	*/
	//float *pfFilter = NULL;
	//iFilterSize = 19;
	//float fSigma = 29.5;
	
	//pfFilter = (float *) calloc ( 19, sizeof (float)  );
	
	//CreateGaussFilter(iFilterSize,fSigma,pfFilter);
	Convolution( ppfGradXY1,g_GH);

	
	int iWStep, iJStep;
	int tmp;
	for (int i=0;i<iNR;i++)
	{
		iWStep = i*pImgSource->widthStep;
		for(int j=0;j<iNC;j++)
		{
			iJStep = j*3;
			*(uchar*)(pImgDest->imageData + iWStep + iJStep + 0) = 
			*(uchar*)(pImgSource->imageData + iWStep + iJStep + 0)*0.3+
			*(uchar*)(pImgDest->imageData + iWStep + iJStep + 0)*0.7;

			*(uchar*)(pImgDest->imageData + iWStep + iJStep + 1) = 
			*(uchar*)(pImgSource->imageData + iWStep + iJStep + 1)*0.3+
			*(uchar*)(pImgDest->imageData + iWStep + iJStep + 1)*0.7;

			*(uchar*)(pImgDest->imageData + iWStep + iJStep + 2) = 
			*(uchar*)(pImgSource->imageData + iWStep + iJStep + 2)*0.3+
			*(uchar*)(pImgDest->imageData + iWStep + iJStep + 2)*0.7;
		}
	}
	

	/*iFilterSize = 11;
	fSigma *=1.6;
	CreateGaussFilter(iFilterSize,fSigma,pfFilter);
	Convolution(pfFilter, ppfGradXY2,iFilterSize, fSigma,g_GH);
	*/

	//g_GH.cvCvtColorProxy(pImgDest,pImgSource,CV_GRAY2BGR);
	
	//Memory Cleaning
	//free(pfFilter);
	free(ppfGradXY1[0]);free(ppfGradXY1);
	//free(ppfGradXY2[0]);free(ppfGradXY2);
	*pImgSrc = pImgDest;
	g_GH.cvReleaseImageProxy(&pImgSource);

}
void clsMotionBlur::Convolution(int **ppfGradXY1,GlobalHelper &g_GH)
{
	int fTempXr,fTempXg,fTempXb;
	int iCenterX = iNC/2.0;
	int iCenterY = iNR/2.0;
	float fRad = m_iAngle*PI/180;
	int x,y;
	int iStep,jStep,kStep;
	float fX = iCenterX*cos(fRad);
	float fY = iCenterY*sin(fRad);

	for(int i=0;i<iNR;i++)
	{
		iStep = (int)pImgDest->imageData + (i)*pImgDest->widthStep;
		for(int j=0;j<iNC;j++)
		{
			fTempXr = fTempXg = fTempXb = 0;
			jStep = j*3;
			for(int k=0;k<m_iFilterSize;k++)
			{
					
					x = j+k + fX;
					if(x<0 || x>=iNC) continue;
					y = i +k+ fY;
					if(y<0 || y>=iNR) continue;
		
					kStep = (int)pImgSource->imageData + (y)*pImgSource->widthStep + x*3;
					//jStep = iStep + x*3;

					fTempXb += (*(UCHAR*)(kStep));
					fTempXg += (*(UCHAR*)(kStep+1 ));
					fTempXr += (*(UCHAR*)(kStep+2 ));
					

			}
			*(UCHAR*)(iStep + jStep + 0) = (float)fTempXb/m_iFilterSize;
			*(UCHAR*)(iStep + jStep + 1) = (float)fTempXg/m_iFilterSize;
			*(UCHAR*)(iStep + jStep + 2) = (float)fTempXr/m_iFilterSize;

		}
	}
//Original working code.
/*
	int fTempXr,fTempXg,fTempXb;//,fTempY;
	//int iStep, ikStep;
	int iCenter = m_iFilterSize/2.0;
	float fRad = m_iAngle*PI/180;
	int x,y;
	int iStep,jStep,kStep;

	for(int i=0;i<iNR;i++)
	{
		iStep = (int)pImgDest->imageData + (i)*pImgDest->widthStep;
		for(int j=0;j<iNC;j++)
		{
			fTempXr = fTempXg = fTempXb = 0;
			jStep = j*3;
			for(int k=0;k<m_iFilterSize;k++)
			{
					
					x = j + iCenter*cos(fRad);
					if(x<0 || x>=iNC) continue;
					y = i + iCenter*sin(fRad);
					if(y<0 || y>=iNR) continue;
		
					kStep = (int)pImgSource->imageData + (y)*pImgSource->widthStep + x*3;
					//jStep = iStep + x*3;

					fTempXb += (*(UCHAR*)(kStep));
					fTempXg += (*(UCHAR*)(kStep+1 ));
					fTempXr += (*(UCHAR*)(kStep+2 ));
					

			}
			*(UCHAR*)(iStep + jStep + 0) = (float)fTempXb/m_iFilterSize;
			*(UCHAR*)(iStep + jStep + 1) = (float)fTempXg/m_iFilterSize;
			*(UCHAR*)(iStep + jStep + 2) = (float)fTempXr/m_iFilterSize;

		}
	}

*/

}
