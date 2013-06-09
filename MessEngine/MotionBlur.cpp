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

	//make ifltersize as odd number;
	pImgDest = g_GH.cvCreateImageProxy(cvSize(pImgSource->width,pImgSource->height),IPL_DEPTH_8U,3);
	IplImage* pImgFinal = g_GH.cvCreateImageProxy(cvSize(pImgSource->width,pImgSource->height),IPL_DEPTH_8U,3);
	IplImage* pImgTemp = g_GH.cvCreateImageProxy(cvSize(pImgSource->width,pImgSource->height),IPL_DEPTH_8U,3);
	
	g_GH.cvSmoothProxy(pImgSource,pImgDest,CV_GAUSSIAN,11,11,0,0);

	int iWStepSrc,iWStepFinal, iJStep;
	for (int i=0;i<iNR;i++)
	{
		iWStepSrc = (int)pImgSource->imageData+ i*pImgSource->widthStep;
		iWStepFinal = (int)pImgFinal->imageData+ i*pImgFinal->widthStep;

		for(int j=0;j<iNC;j++)
		{
			iJStep = j*3;

			*(uchar*)(iWStepFinal + iJStep) = 
			*(uchar*)(iWStepSrc + iJStep)*0.3;

			*(uchar*)(iWStepFinal + iJStep + 1) = 
			*(uchar*)(iWStepSrc + iJStep + 1)*0.3;

			*(uchar*)(iWStepFinal + iJStep + 2) = 
			*(uchar*)(iWStepSrc + iJStep + 2)*0.3;

		}
	}
	int iWStepDest;
	for(int iLoop=0;iLoop<10;iLoop++)
	{
		Point2f src_center(iNC/2.0F, iNR/2.0F);
		//Mat rot_mat = g_GH.cvGetRotationMatrix2DProxy(src_center, m_iAngle/10.0, 1.0);
		
		//g_GH.cvWarpAffineProxy(Mat(pImgDest,true), Mat(pImgTemp,true),
			//g_GH.cvGetRotationMatrix2DProxy(src_center, m_iAngle/10.0, 1.0), Size(iNC,iNR),INTER_LINEAR,BORDER_CONSTANT,Scalar());
		
		IplImage *pImgHolder = pImgDest;
		pImgDest = pImgTemp;
		pImgTemp = pImgHolder;

		for (int i=0;i<iNR;i++)
		{
			iWStepDest = (int)pImgDest->imageData+ i*pImgDest->widthStep;
			iWStepFinal = (int)pImgFinal->imageData+ i*pImgFinal->widthStep;

			for(int j=0;j<iNC;j++)
			{
				iJStep = j*3;

				*(uchar*)(iWStepFinal + iJStep) += 
				*(uchar*)(iWStepDest + iJStep)*0.07;

				*(uchar*)(iWStepFinal + iJStep + 1) += 
				*(uchar*)(iWStepDest + iJStep + 1)*0.07;

				*(uchar*)(iWStepFinal + iJStep + 2) += 
				*(uchar*)(iWStepDest + iJStep + 2)*0.07;

			}
		}
	}	

	*pImgSrc = pImgFinal;
	g_GH.cvReleaseImageProxy(&pImgSource);
	g_GH.cvReleaseImageProxy(&pImgDest);
	g_GH.cvReleaseImageProxy(&pImgTemp);

}
/*void clsMotionBlur::CreateGaussFilter(int iFilterSize,float fSigma, float*pfFilter)
{
	//int iCenter = (int)((float)iFilterSize/2);
	float fTot = 0;
	for(int i=0;i<iFilterSize;i++)
	{
			pfFilter[i] = Gauss(i, fSigma);
			fTot += pfFilter[i];
	}
	for(int i=0;i<iFilterSize;i++)
		pfFilter[i] /=fTot;
}

float clsMotionBlur::Gauss(int x, float fSigma)
{
	
	float fNorm = exp((double) ((-x*x)/(2*fSigma*fSigma)));
	return (1/(sqrt(2*PI)*fSigma)) * fNorm;
}
void clsMotionBlur::Convolution(float*pfFilter, int **ppfGradXY1,GlobalHelper &g_GH)
{
	float fTempXr,fTempXg,fTempXb;//,fTempY;
	//int iNCFilterMidPnt = iFilterSize/2;
	int iStep, ikStep;
	
	for(int i=0;i<iNR;i++)
	{
		for(int j=0;j<iNC;j++)
		{
			fTempXr = fTempXg = fTempXb = 0;//fTempY = 0;
			for(int k=0;k<m_iFilterSize;k++)
			{
					//if(k==0 && l==0) continue;
					if(i+k<0 || i+k>=iNR) continue;
					//if(j+k<0 || j+k>=iNC) continue;
					//fTempY += pfFilter[iNCFilterMidPnt+k]*(*(UCHAR*)(pImgDest->imageData + (i+k)*pImgDest->widthStep+j));
					//if(k==0) continue;
					fTempXb += pfFilter[k]*(*(UCHAR*)(pImgSource->imageData + (i)*pImgSource->widthStep+(j+k)*3+0 ));
					fTempXg += pfFilter[k]*(*(UCHAR*)(pImgSource->imageData + (i)*pImgSource->widthStep+(j+k)*3+1 ));
					fTempXr += pfFilter[k]*(*(UCHAR*)(pImgSource->imageData + (i)*pImgSource->widthStep+(j+k)*3+2 ));
					

			}
			*(UCHAR*)(pImgDest->imageData + (i)*pImgDest->widthStep+j*3+0) = fTempXb;//sqrt(fTempX*fTempX+fTempY*fTempY);
			*(UCHAR*)(pImgDest->imageData + (i)*pImgDest->widthStep+j*3+1) = fTempXg;//sqrt(fTempX*fTempX+fTempY*fTempY);
			*(UCHAR*)(pImgDest->imageData + (i)*pImgDest->widthStep+j*3+2) = fTempXr;//sqrt(fTempX*fTempX+fTempY*fTempY);

		}
	}


}
*/