#include "stdafx.h"
#include "FisheyeLens.h"
#include "GlobalHelper.h"
 

extern "C" __declspec(dllexport) void FisheyeLens(char *szPath, int iStyle, int*iErrCode)
{
	DWORD dwStart = GetTickCount();
	SYSTEMTIME stStart;
	GetLocalTime(&stStart);

	GlobalHelper g_GH;
	g_GH.Init(szPath,iErrCode);
	if(*iErrCode >0)
		return;

	
	IplImage *pImgSource;
	int iRet = g_GH.load_RGB(szPath,&pImgSource,iStyle);
	if(!iRet)
	{
		*iErrCode = 1;
		return;
	}


	clsFisheyeLens FisheyeLens;
	FisheyeLens.FisheyeLensMain(&pImgSource,iStyle,iErrCode,g_GH);

	iRet = g_GH.save_RGB(szPath,pImgSource);
	if(!iRet)
	{
		*iErrCode = 1;
		return;
	}

	g_GH.cvReleaseImageProxy(&pImgSource);
	
	DWORD dwEnd = GetTickCount();
	char pszTimec[1024];pszTimec[0] = 0;
	sprintf(pszTimec,"Fisheye,%d-%02d-%02d %02d:%02d:%02d,%ld\0",stStart.wYear,stStart.wMonth,stStart.wDay,stStart.wHour,
		stStart.wMinute, stStart.wSecond, dwEnd - dwStart);
	//sprintf(pszTimec,"Fisheye,%d-%02d-%02d %02d:%02d:%02d,%ld\0",stStart.wYear,stStart.wMonth,stStart.wDay,dwEnd - dwStart);

	g_GH.DebugOut(pszTimec);

}
clsFisheyeLens::clsFisheyeLens()
{
	
}
clsFisheyeLens::~clsFisheyeLens()
{
}
void clsFisheyeLens::FisheyeLensMain(IplImage **pImgSrc, int iStyle, int*iErrCode,GlobalHelper &g_GH)
{
	pImgSource = *pImgSrc;
	if(!pImgSource)
	{
		*iErrCode = 1;
		return;
	}

	iNC = pImgSource->width;
	iNR = pImgSource->height;
	
	ApplyCanvas(iStyle,g_GH);
	
	*pImgSrc = pImgDest;
	g_GH.cvReleaseImageProxy(&pImgSource);

}
/*
1) First I determine an radian angle between a given point and the ellipse center, like this: 
Math.atan2(p2.y - p1.y, p2.x - p1.x); 
Where p1 is the ellipse center, p2 is the point to be deformed. 

2) Then, based on that angle, I determine the point on the ellipse perimeter which intersects the center through that angle, like this: 
var r = 0.5 / Math.sqrt( Math.pow(Math.cos(angle)/ellipse.width,2) + Math.pow(Math.sin(angle)/ellipse.height,2) ) ; 
var x = center.x + r*Math.cos(angle); 
var y = center.y + r*Math.sin(angle); 

3) Then I determine 2 distances: d1 is the distance from the ellipse center to the ellipse perimeter point as determined above,
and d2 is the distance from the ellipse center to original point to be deformed. This gives me a weight via d2/d1. Distance like this: 
Math.sqrt(((p1.x-p2.x)*(p1.x-p2.x))+((p1.y-p2.y)*(p1.y-p2.y))) 

4) I then re-plot the point based on quadratic easing equation with the tween class: 
-c *(t/d)*(t/d-2) + b 
Where t is the time, or weight as determined above, b is the begin value (center point), c is the change in value (perimeter point - center point), and d is the duration, ie 1 


*/
void clsFisheyeLens::ApplyCanvas(int iStyle, GlobalHelper &g_GH)
{
	
	pImgDest = g_GH.cvCreateImageProxy(cvSize(iNC,iNR),IPL_DEPTH_8U,3);
	g_GH.cvSetProxy(pImgDest,CV_RGB(255,255,255),0);

	IplImage *pImgTemp = g_GH.cvCreateImageProxy(cvSize(iNC,iNR),IPL_DEPTH_8U,1);
	g_GH.cvSetProxy(pImgTemp,CV_RGB(0,0,0),0);

	int iCenterX = iNC/2.0;
	int iCenterY = iNR/2.0;
	
	float fTheta,fRadius;
	int iPeremeterX,iPeremeterY;
	float fDist1,fDist2,fDistWt;
	int iNewX;
	int iNewY;

	int iWStep, iJStep,iWStepN,iJStepN;
	int tmpS,tmpD;
	for (int i=0;i<iNR;i++)
	{
		iWStep = i*pImgSource->widthStep;
		for(int j=0;j<iNC;j++)
		{
			iJStep = j*3;
			fTheta = atan2((double)i - iCenterY, j - iCenterX);
			
			fRadius = 0.5 / sqrt( pow(cos(fTheta)/iNC,2) + pow(sin(fTheta)/iNR,2) ) ; 
			//fRadius = 0.5 / sqrt( pow(cos(fTheta)/(iNC/2),2) + pow(sin(fTheta)/(iNR/2),2) ) ; 
			iPeremeterX = iCenterX + fRadius*cos(fTheta); 
			iPeremeterY = iCenterY + fRadius*sin(fTheta); 

			fDist1 = sqrt(  (double)(iCenterX-iPeremeterX)*(iCenterX-iPeremeterX) + (iCenterY-iPeremeterY)*(iCenterY-iPeremeterY) ) ;
			fDist2 = sqrt(  (double)(iCenterX-j)*(iCenterX-j) + (iCenterY-i)*(iCenterY-i) ) ;
			fDistWt = fDist2/fDist1;
			if(abs(fDist2)>abs(fDist1))
				continue;
			//peep in
			if(iStyle ==0)
			{
				iNewX = (-(iPeremeterX-iCenterX) *(fDistWt)*(fDistWt-2) + iCenterX); 
				iNewY = (-(iPeremeterY-iCenterY) *(fDistWt)*(fDistWt-2) + iCenterY); 
			}
			//peep out
			if(iStyle == 1)
			{
				iNewX = (-(j-iCenterX) *(fDistWt)*(fDistWt-2) + iCenterX); 
				iNewY = (-(i-iCenterY) *(fDistWt)*(fDistWt-2) + iCenterY); 
			}
			iWStepN = iNewY*pImgSource->widthStep;
			iJStepN = iNewX*3;

			if(iNewX<iNC &&iNewY<iNR && iNewX>=0 && iNewY>=0)
			{
				*(uchar*)(pImgDest->imageData + iWStepN + iJStepN + 0 ) = *(uchar*)(pImgSource->imageData + iWStep + iJStep + 0 );
				*(uchar*)(pImgDest->imageData + iWStepN + iJStepN + 1 ) = *(uchar*)(pImgSource->imageData + iWStep + iJStep + 1 );
				*(uchar*)(pImgDest->imageData + iWStepN + iJStepN + 2 ) = *(uchar*)(pImgSource->imageData + iWStep + iJStep + 2 );
				
				*(uchar*)(pImgTemp->imageData + iNewY*pImgTemp->widthStep + iNewX ) = 255;
			}
		}
	}
	int tmpR,tmpG,tmpB;
	int i_1Step,i1Step,iStep;
	int j_1Step,j1Step,jStep;
	int iTot;
	for (int i=1;i<iNR-1;i++)
	{
		i_1Step = (i-1)*pImgDest->widthStep;
		i1Step = (i+1)*pImgDest->widthStep;
		iStep = (i)*pImgDest->widthStep;
		for(int j=1;j<iNC-1;j++)
		{
			j_1Step = (j-1)*3;
			j1Step = (j+1)*3;
			jStep = (j)*3;
			iTot = tmpR = tmpG = tmpB = 0;
			if(  ! *(uchar*)(pImgTemp->imageData  + i*pImgTemp->widthStep + j ) )
			{
				
				if(   *(uchar*)(pImgTemp->imageData  + (i-1)*pImgTemp->widthStep + j-1 ) )
				{	
					tmpB += *(uchar*)(pImgDest->imageData + i_1Step + j_1Step + 0 );
					tmpG += *(uchar*)(pImgDest->imageData + i_1Step + j_1Step + 1 );
					tmpR += *(uchar*)(pImgDest->imageData + i_1Step + j_1Step + 2 );
					iTot++;
				}
				if(   *(uchar*)(pImgTemp->imageData  + (i-1)*pImgTemp->widthStep + j ) )
				{	
					tmpB += *(uchar*)(pImgDest->imageData + i_1Step + jStep + 0 );
					tmpG += *(uchar*)(pImgDest->imageData + i_1Step + jStep + 1 );
					tmpR += *(uchar*)(pImgDest->imageData + i_1Step + jStep + 2 );
					iTot++;
				}
				if(   *(uchar*)(pImgTemp->imageData  + (i-1)*pImgTemp->widthStep + j+1 ) )
				{	
					tmpB += *(uchar*)(pImgDest->imageData + i_1Step + j1Step + 0 );
					tmpG += *(uchar*)(pImgDest->imageData + i_1Step + j1Step + 1 );
					tmpR += *(uchar*)(pImgDest->imageData + i_1Step + j1Step + 2 );
					iTot++;
				}

				if(   *(uchar*)(pImgTemp->imageData  + (i)*pImgTemp->widthStep + j-1 ) )
				{	
					tmpB += *(uchar*)(pImgDest->imageData + iStep + j_1Step + 0 );
					tmpG += *(uchar*)(pImgDest->imageData + iStep + j_1Step + 1 );
					tmpR += *(uchar*)(pImgDest->imageData + iStep + j_1Step + 2 );
					iTot++;
				}
/*				if(   *(uchar*)(pImgTemp->imageData + + (i)*pImgTemp->widthStep + j ) )
				{	
					tmpB += *(uchar*)(pImgDest->imageData + iStep + jStep + 0 );
					tmpG += *(uchar*)(pImgDest->imageData + iStep + jStep + 1 );
					tmpR += *(uchar*)(pImgDest->imageData + iStep + jStep + 2 );
					iTot++;
				}*/
				if(   *(uchar*)(pImgTemp->imageData  + (i)*pImgTemp->widthStep + j+1 ) )
				{	
					tmpB += *(uchar*)(pImgDest->imageData + iStep + j1Step + 0 );
					tmpG += *(uchar*)(pImgDest->imageData + iStep + j1Step + 1 );
					tmpR += *(uchar*)(pImgDest->imageData + iStep + j1Step + 2 );
					iTot++;
				}

				if(   *(uchar*)(pImgTemp->imageData  + (i+1)*pImgTemp->widthStep + j-1 ) )
				{	
					tmpB += *(uchar*)(pImgDest->imageData + i1Step + j_1Step + 0 );
					tmpG += *(uchar*)(pImgDest->imageData + i1Step + j_1Step + 1 );
					tmpR += *(uchar*)(pImgDest->imageData + i1Step + j_1Step + 2 );
					iTot++;
				}
				if(   *(uchar*)(pImgTemp->imageData  + (i+1)*pImgTemp->widthStep + j ) )
				{	
					tmpB += *(uchar*)(pImgDest->imageData + i1Step + jStep + 0 );
					tmpG += *(uchar*)(pImgDest->imageData + i1Step + jStep + 1 );
					tmpR += *(uchar*)(pImgDest->imageData + i1Step + jStep + 2 );
					iTot++;
				}
				if(   *(uchar*)(pImgTemp->imageData  + (i+1)*pImgTemp->widthStep + j+1 ) )
				{	
					tmpB += *(uchar*)(pImgDest->imageData + i1Step + j1Step + 0 );
					tmpG += *(uchar*)(pImgDest->imageData + i1Step + j1Step + 1 );
					tmpR += *(uchar*)(pImgDest->imageData + i1Step + j1Step + 2 );
					iTot++;
				}
				if(iTot)
				{
					*(uchar*)(pImgDest->imageData + iStep + jStep + 0 )=(float)tmpB/iTot;
					*(uchar*)(pImgDest->imageData + iStep + jStep + 1 )=(float)tmpG/iTot;
					*(uchar*)(pImgDest->imageData + iStep + jStep + 2 )=(float)tmpR/iTot;

				}

			}

		}
	}
	g_GH.cvReleaseImageProxy(&pImgTemp);
}
