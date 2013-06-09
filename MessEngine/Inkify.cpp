#include "stdafx.h"
#include "Inkify.h"
#include "GlobalHelper.h"
#define PI 3.1415926535

extern char g_pcszPath[];

extern "C" __declspec(dllexport) void Inkify(char *szPath, int bPreviewMode, int*iErrCode)
{
	DWORD dwStart = GetTickCount();
	SYSTEMTIME stStart;
	GetLocalTime(&stStart);

	GlobalHelper g_GH;
	g_GH.Init(szPath,iErrCode);
	if(*iErrCode >0)
		return;

	//g_GH.DebugOut("Inkify: Begin",TRUE);

	IplImage *pImgSource;
	int iRet = g_GH.load_RGB(szPath,&pImgSource,bPreviewMode);
	if(!iRet)
	{
		//g_GH.DebugOut("Inkify: LoadImage failed",FALSE);
		*iErrCode = 1;
		return;
	}
	//clsCartoonEffect Cartoon;
	//Cartoon.CartoonEffectMain(&pImgSource,iFilterSize, fSigmaDist, fSigmaColor, 
		// iErrCode,g_GH);

	clsInkify Inkify;
	Inkify.InkifyMain(&pImgSource,7, 10.5,300, 
		 iErrCode,g_GH);
	
	iRet = g_GH.save_RGB(szPath,pImgSource);
	if(!iRet)
	{
		//g_GH.DebugOut("Inkify: save_RGB failed",FALSE);
		*iErrCode = 1;
		return;
	}

	g_GH.cvReleaseImageProxy(&pImgSource);

	//g_GH.DebugOut("Inkify: End",TRUE);

	*iErrCode = 0;
	
	DWORD dwEnd = GetTickCount();
	char pszTimec[1024];pszTimec[0] = 0;
	sprintf(pszTimec,"Inkify,%d-%02d-%02d %02d:%02d:%02d,%ld\0",stStart.wYear,stStart.wMonth,stStart.wDay,stStart.wHour,
		stStart.wMinute, stStart.wSecond, dwEnd - dwStart);
	//sprintf(pszTimec,"Inkify,%d-%02d-%02d %02d:%02d:%02d,%ld\0",stStart.wYear,stStart.wMonth,stStart.wDay,dwEnd - dwStart);

	g_GH.DebugOut(pszTimec);

	
}

clsInkify::clsInkify()
{
}
clsInkify::~clsInkify()
{
}
void clsInkify::InkifyMain(IplImage **pImgSrc,int iFilterSize, float fSigmaDist, float fSigmaColor, 
	  int*iErrCode,GlobalHelper &g_GH)
{
	pImgSource = *pImgSrc;
	m_iFilterSize =iFilterSize;
	m_fSigmaDist = fSigmaDist;
	m_fSigmaColor = fSigmaColor;

	if(!pImgSource)
	{
		//g_GH.DebugOut("Inkify: Source Image NULL",FALSE);
		*iErrCode = 1;
		return;
	}

	iNC = pImgSource->width;
	iNR = pImgSource->height;


	//Create Sobel
	ppfSobelX = g_GH.FloatHeap2D(iNR,iNC);
	ppfSobelY = g_GH.FloatHeap2D(iNR,iNC);
	pImgGrey = g_GH.cvCreateImageProxy(cvSize(pImgSource->width,pImgSource->height),IPL_DEPTH_8U,1);
	g_GH.cvCvtColorProxy(pImgSource,pImgGrey,CV_BGR2GRAY);
	Sobel(ppfSobelX,ppfSobelY,g_GH);

	//Smooth image with BF before Inkify
	pImgDest = g_GH.cvCreateImageProxy(cvSize(pImgSource->width,pImgSource->height),IPL_DEPTH_8U,3);
	g_GH.cvSmoothProxy(pImgSource,pImgDest,CV_BILATERAL,11,0,16.5,fSigmaDist);
	g_GH.cvSmoothProxy(pImgDest,pImgSource,CV_BILATERAL,11,0,16.5,fSigmaDist);
	g_GH.cvCvtColorProxy(pImgSource,pImgGrey,CV_BGR2GRAY);

	//Apply difference of gaussian
	g_GH.cvReleaseImageProxy(&pImgDest);
	pImgDest = g_GH.cvCreateImageProxy(cvSize(iNC,iNR),IPL_DEPTH_8U,1);
	pImgTemp = g_GH.cvCreateImageProxy(cvSize(iNC,iNR),IPL_DEPTH_8U,1);

	g_GH.cvSmoothProxy(pImgGrey,pImgDest,CV_GAUSSIAN,9,9,0,0);
	g_GH.cvSmoothProxy(pImgGrey,pImgTemp,CV_GAUSSIAN,15,15,0,0);
	
	Diff(g_GH);
	g_GH.cvCopyProxy(pImgDest,pImgGrey,NULL);

	//Paint the sketch by stroke
	g_GH.cvReleaseImageProxy(&pImgDest);
	g_GH.cvReleaseImageProxy(&pImgTemp);
	
	pImgDest = g_GH.cvCreateImageProxy(cvSize(iNC,iNR),IPL_DEPTH_8U,3);
	pImgTemp = g_GH.cvCreateImageProxy(cvSize(iNC,iNR),IPL_DEPTH_8U,3);
	
	g_GH.cvSetProxy(pImgDest,CV_RGB(255,255,255),0);
	g_GH.cvSetProxy(pImgTemp,CV_RGB(0,0,0),0);

		
	for(int i=0;i<iNR;i++)
		for(int j=0;j<iNC;j++)
			if(*(uchar*)(pImgGrey->imageData + i*pImgGrey->widthStep + j)==0)
				MakeStroke( 2, i,j,g_GH );

	//Canvas
	ApplyCanvas(g_GH);

	free(ppfSobelX[0]);free(ppfSobelX);
	free(ppfSobelY[0]);free(ppfSobelY);

	g_GH.cvReleaseImageProxy(&pImgTemp);
	g_GH.cvReleaseImageProxy(&pImgGrey);
	g_GH.cvReleaseImageProxy(&pImgSource);
	//delete iGrey;

	*pImgSrc = pImgDest;

}
void clsInkify::ApplyCanvas(GlobalHelper &g_GH)
{
	
	char buff[1024];
	buff[0] = NULL;
	strcpy(buff,g_pcszPath);

	strcat(buff,"inkifybk");

	IplImage *imgrgb;
	int iRet = g_GH.load_RGB(buff,&imgrgb,FALSE);
	if(!iRet)
	{
		//g_GH.DebugOut("clsPatronize::ApplyCanvas - Failed to load canvas",FALSE);
		//g_GH.DebugOut(buff,FALSE);
		return;
	}

	g_GH.cvResizeProxy(imgrgb,pImgTemp,1);

	int iWStep, iJStep,iWStepTemp;
	int tmp;
	for (int i=0;i<iNR;i++)
	{
		iWStep = i*pImgSource->widthStep +(int)pImgDest->imageData;
		iWStepTemp = i*pImgSource->widthStep +(int)pImgTemp->imageData;
		for(int j=0;j<iNC;j++)
		{
			iJStep = j*3;
			*(uchar*)( iWStep + iJStep + 0) = 
			(*(uchar*)(iWStep + iJStep + 0)>>1)+
			(*(uchar*)(iWStepTemp + iJStep + 0)>>1);

			*(uchar*)(iWStep + iJStep + 1) = 
			(*(uchar*)(iWStep + iJStep + 1)>>1)+
			(*(uchar*)(iWStepTemp + iJStep + 1)>>1);

			*(uchar*)(iWStep + iJStep + 2) = 
			(*(uchar*)(iWStep + iJStep + 2)>>1)+
			(*(uchar*)(iWStepTemp + iJStep + 2)>>1);
		}
	}

	g_GH.cvReleaseImageProxy(&imgrgb);

}

void clsInkify::Diff(GlobalHelper &g_GH)
{
	int iw,iWTemp;
	for (int i=0;i<iNR;i++)
	{
		iw = (i)*pImgGrey->widthStep +(int) pImgDest->imageData;
		iWTemp = (i)*pImgGrey->widthStep +(int)pImgTemp->imageData ;
		
		for(int j=0;j<iNC;j++)
		{
			if( *(UCHAR*)( iw +j) - *(UCHAR*)(iWTemp +j) < 0 )
				*(UCHAR*)(iw +j) = 0;
			else
				*(UCHAR*)(iw +j) = 255;

		}
	}

}

void clsInkify::MakeStroke(float iBrushSize, int rowInitial, int colInitial,GlobalHelper &g_GH)
{
	//int **ppfPath = g_GH.IntHeap2D(param.iMaxStrokeLength,2);
	int iMaxStrokeLength = 16;
	int iMinStrokeLength = 2;

	int rowCurr = rowInitial;
	int colCurr = colInitial;
	int iCtr = 0;
	float fLastDeltaX = 0,fLastDeltaY = 0;
	float fGradDirX,fGradDirY;
	float fDeltaX,fDeltaY;
	CvPoint pPath[16];

	pPath[iCtr].x = colCurr;
	pPath[iCtr].y = rowCurr;
	float fDiff;	
	for ( iCtr = 1; iCtr < iMaxStrokeLength; iCtr++ ) 
	{
		int dR = *(uchar*)(pImgSource->imageData + rowCurr*pImgSource->widthStep + colCurr*3 + 2) -
				 *(uchar*)(pImgSource->imageData + pPath[0].y*pImgSource->widthStep + pPath[0].x*3 + 2);
			
		int dG = *(uchar*)(pImgSource->imageData + rowCurr*pImgSource->widthStep + colCurr*3 + 1) -
				 *(uchar*)(pImgSource->imageData + pPath[0].y*pImgSource->widthStep + pPath[0].x*3 + 1);
			
		int dB = *(uchar*)(pImgSource->imageData + rowCurr*pImgSource->widthStep + colCurr*3 + 0) -
				 *(uchar*)(pImgSource->imageData + pPath[0].y*pImgSource->widthStep + pPath[0].x*3 + 0);
			
		fDiff = sqrt( ( double )( dR * dR + dB * dB + dG * dG ) );
		
		if ( iCtr > iMinStrokeLength && 
				GetDifference( rowCurr,colCurr ) < fDiff ) {
			break;
		}
		if ( getGradientMagnitude(rowCurr,colCurr) <= 10.0f ) 
			break;
			
		getGradientDirection( rowCurr,colCurr,&fGradDirX,&fGradDirY );
		fDeltaX = fGradDirX;
		fDeltaY = -fGradDirY;
			
		if ( fLastDeltaX * fDeltaX + fLastDeltaY * fDeltaY < 0 ) 
		{
			fDeltaX = -fDeltaX;
			fDeltaY = -fDeltaY;
		}
			
		float deltaMag = sqrt( fDeltaX * fDeltaX + fDeltaY * fDeltaY );
			
		//fDeltaX = param.fC * fDeltaX + ( 1 - param.fC ) * fLastDeltaX;
		fDeltaX = fDeltaX / deltaMag;
			
		//fDeltaY = param.fC * fDeltaY + ( 1 - param.fC ) * fLastDeltaY;
		fDeltaY = fDeltaY / deltaMag;

		colCurr += iBrushSize * fDeltaX;
		rowCurr += iBrushSize * fDeltaY;
			
		// brush went off canvas, so just stop it
		if ( colCurr < 0.0f || colCurr >= iNC ||
				rowCurr < 0.0f || rowCurr >= iNR ) {
			break;
		}
			
		fLastDeltaX = fDeltaX;	
		fLastDeltaY = fDeltaY;	

		pPath[iCtr].y = rowCurr;
		pPath[iCtr].x = colCurr;
	}
	if ( iCtr > iMinStrokeLength ) 
	{
		for(int i=0;i<iCtr-1;i++)
		{
			/*int ddR = *(uchar*)(pImgSource->imageData + pPath[0].y*pImgSource->widthStep + pPath[0].x*3 + 2);
			
			int ddG = *(uchar*)(pImgSource->imageData + pPath[0].y*pImgSource->widthStep + pPath[0].x*3 + 1);
			
			int ddB = *(uchar*)(pImgSource->imageData + pPath[0].y*pImgSource->widthStep + pPath[0].x*3 + 0);
			
			int iAvg = (ddR+ddG+ddB)/3.0;*/
			g_GH.cvLineProxy(pImgDest,pPath[i],pPath[i+1],CV_RGB(7,
			79,181),1,CV_AA,0);
			
			g_GH.cvLineProxy(pImgTemp,pPath[i],pPath[i+1],CV_RGB(255,255,255),1,CV_AA,0);
		}
	} 
		
	//RenderStroke( iBrushSize, pPath,iCtr);
	//free(ppfPath[0]);free(ppfPath);
}
/*
void clsInkify::MakeStroke(float iBrushSize, int rowInitial, int colInitial,GlobalHelper &g_GH)
{
//		1) if grad mag does not fall below certain level 
//		2) if already painted then again i break
//		3) if size > 10% of the image then again break.

	int rowCurr = rowInitial;
	int colCurr = colInitial;
	int iCtr = 0;
	float fLastDeltaX = 0,fLastDeltaY = 0;
	float fGradDirX,fGradDirY;
	float fDeltaX,fDeltaY;
	CvPoint ptDriverC[4];
	//0=Column number in X Direction. 1 is row number in Y direction
	
	pPath[iCtr][0] = ptDriverC[0].x = colCurr;
	pPath[iCtr][1] = ptDriverC[0].y = rowCurr;

	float fDiff;	
	for ( iCtr = 1; iCtr < iMaxStrokeLength; iCtr++ ) 
	{

		if ( *(uchar*)(pImgDest->imageData + rowCurr*pImgDest->widthStep + colCurr*3)==0)
			break;
		if ( getGradientMagnitude(rowCurr,colCurr) < 30.0f ) 
			break;
			
		getGradientDirection( rowCurr,colCurr,&fGradDirX,&fGradDirY );
		fDeltaX = fGradDirX;
		fDeltaY = -fGradDirY;
			
		if ( fLastDeltaX * fDeltaX + fLastDeltaY * fDeltaY < 0 ) 
		{
			fDeltaX = -fDeltaX;
			fDeltaY = -fDeltaY;
		}
			
		float deltaMag = sqrt( fDeltaX * fDeltaX + fDeltaY * fDeltaY );
			
		//fDeltaX = param.fC * fDeltaX + ( 1 - param.fC ) * fLastDeltaX;
		fDeltaX = fDeltaX / deltaMag;
			
		//fDeltaY = param.fC * fDeltaY + ( 1 - param.fC ) * fLastDeltaY;
		fDeltaY = fDeltaY / deltaMag;

		colCurr += iBrushSize * fDeltaX;
		rowCurr += iBrushSize * fDeltaY;
			
		// brush went off canvas, so just stop it
		if ( colCurr < 0.0f || colCurr >= iNC ||
				rowCurr < 0.0f || rowCurr >= iNR ) {
			break;
		}
			
		fLastDeltaX = fDeltaX;	
		fLastDeltaY = fDeltaY;	

		pPath[iCtr][1] = rowCurr;
		pPath[iCtr][0] = colCurr;
	}
	if(iCtr>=4)
	{
	
		ptDriverC[3].x = pPath[iCtr-1][0];
		ptDriverC[3].y = pPath[iCtr-1][1];

		ptDriverC[1].x = pPath[(int)(iCtr*0.33)][0];
		ptDriverC[1].y = pPath[(int)(iCtr*0.33)][1];

		ptDriverC[2].x = pPath[(int)(iCtr*0.67)][0];
		ptDriverC[2].y = pPath[(int)(iCtr*0.67)][1];

		BezierCurve(ptDriverC,iCtr,g_GH);
	}
	/*CvPoint ptTemp,ptNext;
	if ( iCtr > 1 ) 
	{
		for(int i=0;i<iCtr-1;i++)
		{
			int ddR = *(uchar*)(pImgSource->imageData + pPath[0][1]*pImgSource->widthStep + pPath[0][0]*3 + 2);
			
			int ddG = *(uchar*)(pImgSource->imageData + pPath[0][1]*pImgSource->widthStep + pPath[0][0]*3 + 1);
			
			int ddB = *(uchar*)(pImgSource->imageData + pPath[0][1]*pImgSource->widthStep + pPath[0][0]*3 + 0);
			
			int iAvg = (ddR+ddG+ddB)/3.0;
			ptTemp.x = pPath[i][0]; ptTemp.y = pPath[i][1];
			ptNext.x = pPath[i+1][0]; ptNext.y = pPath[i+1][1];
			g_GH.cvLineProxy(pImgDest,ptTemp,ptNext,CV_RGB(iAvg,
			iAvg,iAvg),1,CV_AA,0);
			
			g_GH.cvLineProxy(pImgTemp,ptTemp,ptNext,CV_RGB(255,255,255),1,CV_AA,0);
		}
	} *//*
	//RenderStroke( iBrushSize, pPath,iCtr);
	//free(ppfPath[0]);free(ppfPath);*/
/*}*/
float clsInkify::GetDifferenceClr( int row, int col ) 
{
	int dR = *(uchar*)(pImgSource->imageData + row*pImgSource->widthStep + col*3 + 2)
			- *(uchar*)(pImgDest->imageData + row*pImgDest->widthStep + col*3 + 2);	
	int dG = *(uchar*)(pImgSource->imageData + row*pImgSource->widthStep + col*3 + 1)
			- *(uchar*)(pImgDest->imageData + row*pImgDest->widthStep + col*3 + 1);	
	int dB = *(uchar*)(pImgSource->imageData + row*pImgSource->widthStep + col*3 + 0)
			- *(uchar*)(pImgDest->imageData + row*pImgDest->widthStep + col*3 + 0);	
	
	return (float)sqrt( ( double )( dR * dR + dB * dB + dG * dG ) );
}
float clsInkify::GetDifference( int row, int col ) 
{
	float difference;
	/*			
	if ( *(uchar*)(pImgTemp->imageData + row*pImgTemp->widthStep + col*3 + 2)==0 && 
		*(uchar*)(pImgTemp->imageData + row*pImgTemp->widthStep + col*3 + 1)==0 && 
		*(uchar*)(pImgTemp->imageData + row*pImgTemp->widthStep + col*3 + 0)==0 )
	{
		difference = (float)32000;
	} 
	else*/ 
	{
		difference = GetDifferenceClr( row, col );
	}
		
	return difference;
}
float clsInkify::getGradientMagnitude( int row, int col ) 
{
	return sqrt(ppfSobelX[row][col]*ppfSobelX[row][col] + 
			ppfSobelY[row][col]*ppfSobelY[row][col]);
}
	
void clsInkify::getGradientDirection(  int row, int col, float *fGradDirX, float *fGradDirY ) 
{
	*fGradDirX = ppfSobelX[row][col] / getGradientMagnitude( row,col );
	*fGradDirY = ppfSobelY[row][col] / getGradientMagnitude( row,col );
}
int clsInkify::Sobel(float **ppfSobelX,float **ppfSobelY,GlobalHelper &g_GH)
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

			x = -1*(*(UCHAR*)(pImgGrey->imageData + i_1 +j-1)) + 
				-2*(*(UCHAR*)(pImgGrey->imageData + i_1 +j)) + 
				-1*(*(UCHAR*)(pImgGrey->imageData + i_1 +j+1)) +
				*(UCHAR*)(pImgGrey->imageData + i1 +j-1) + 
				2*(*(UCHAR*)(pImgGrey->imageData + i1 +j)) + 
				1*(*(UCHAR*)(pImgGrey->imageData + i1 +j+1));

			y = -1*(*(UCHAR*)(pImgGrey->imageData + i_1 +j-1)) + 
				-2*(*(UCHAR*)(pImgGrey->imageData + iw +j-1)) + 
				-1*(*(UCHAR*)(pImgGrey->imageData + i1 +j-1)) +
				*(UCHAR*)(pImgGrey->imageData + i_1 +j+1) + 
				2*(*(UCHAR*)(pImgGrey->imageData + iw +j+1)) + 
				1*(*(UCHAR*)(pImgGrey->imageData + i1 +j+1));

			ppfSobelY[i][j] = y;
			ppfSobelX[i][j] = x;
			//ppfConv[i][j] = sqrt((float)x*x+y*y);
		}
	}
	return 0;
}	
void clsInkify::BezierCurve(CvPoint ptDriverC[4],int iCtr,GlobalHelper &g_GH)
{
	CvPoint pnt[100];
	int limit;
	if(iCtr<=100)
		limit= 100.0f/iCtr;
	else
		limit = 1;

	double fT,fT2, fT3;
	int i=0;
	for(int time=0;time<100;time+=limit)
	{
		fT = (float)time/100; fT2 = fT*fT; fT3 = fT2*fT; 
		pnt[i].x = ptDriverC[0].x*(1-3*fT+3*fT2-fT3) + ptDriverC[1].x*(3*fT-6*fT2+3*fT3) + ptDriverC[2].x*(3*fT2-3*fT3) + ptDriverC[3].x*fT3;
		pnt[i].y = ptDriverC[0].y*(1-3*fT+3*fT2-fT3) + ptDriverC[1].y*(3*fT-6*fT2+3*fT3) + ptDriverC[2].y*(3*fT2-3*fT3) + ptDriverC[3].y*fT3;

		if(i>0)
		g_GH.cvLineProxy(pImgDest,pnt[i-1],pnt[i],CV_RGB(0,
			0,0),1,CV_AA,0);
		i++;
	}

	//for(int i=0;i<100-1;i++)

}

/*
void clsInkify::Threshold(float **ppfGradXY1,GlobalHelper &g_GH)
{
	int iStep,jStep,tmp;
	for (int i=0;i<iNR;i++)
	{
		iStep = i*pImgDest->widthStep;
		for(int j=0;j<iNC;j++)
		{
			if(ppfGradXY1[i][j]<=0)
				*(uchar*)(pImgDest->imageData + iStep + j) = 0;
			else if(ppfGradXY1[i][j]<=255)
				*(uchar*)(pImgDest->imageData + iStep + j) = 255;//ppfGradXY1[i][j];
			else
				*(uchar*)(pImgDest->imageData + iStep + j) = 255;
		}
	}

}*/
/*
void clsInkify::CreateGaussFilter(int iFilterSize,float fSigma, float*pfFilter)
{
	int iCenter = (int)((float)iFilterSize/2);

	for(int i=0;i<=iCenter;i++)
	{
			pfFilter[iCenter+i] = Gauss(i, fSigma);
			pfFilter[iCenter-i] = Gauss(i, fSigma);
	}
}
float clsInkify::Gauss(int x, float fSigma)
{
	
	float fNorm = exp((double) ((-x*x)/(2*fSigma*fSigma)));
	return (1/(sqrt(2*PI)*fSigma)) * fNorm;
}
void clsInkify::Convolution(float*pfFilter, float **ppfGradXY1,int iFilterSize, float fSigma,GlobalHelper &g_GH)
{
	float fTempX,fTempY;
	int iNCFilterMidPnt = iFilterSize/2;
	int iStep, ikStep;
	
	for(int i=iNCFilterMidPnt;i<iNR;i++)
	{
		for(int j=iNCFilterMidPnt;j<iNC;j++)
		{
			fTempX = fTempY = 0;
			for(int k=-iNCFilterMidPnt;k<=iNCFilterMidPnt;k++)
			{
					//if(k==0 && l==0) continue;
					if(i+k<0 || i+k>=iNR) continue;
					if(j+k<0 || j+k>=iNC) continue;
					fTempY += pfFilter[iNCFilterMidPnt+k]*(*(UCHAR*)(pImgDest->imageData + (i+k)*pImgDest->widthStep+j));
					//if(k==0) continue;
					fTempX += pfFilter[iNCFilterMidPnt+k]*(*(UCHAR*)(pImgDest->imageData + (i)*pImgDest->widthStep+j+k));

			}
			ppfGradXY1[i][j] = sqrt(fTempX*fTempX+fTempY*fTempY);

		}
	}


}*/
		/*
		pnt[time].x = fT_1*fT_1*fT_1*ptDriver[0].x + 3*fT_1*fT_1*fT*ptDriver[1].x + 3*fT_1*fT*fT*ptDriver[2].x + fT*fT*fT*ptDriver[3].x;
		pnt[time].y = fT_1*fT_1*fT_1*ptDriver[0].y + 3*fT_1*fT_1*fT*ptDriver[1].y + 3*fT_1*fT*fT*ptDriver[2].y + fT*fT*fT*ptDriver[3].y;
		*/
		/*ptDriver[0].x = ptDriverC[0].x + (ptDriverC[1].x-ptDriverC[0].x)*(float)time/100;
		ptDriver[0].y = ptDriverC[0].y + (ptDriverC[1].y-ptDriverC[0].y)*(float)time/100;

		ptDriver[1].x = ptDriverC[1].x + (ptDriverC[2].x-ptDriverC[1].x)*(float)time/100;
		ptDriver[1].y = ptDriverC[1].y + (ptDriverC[2].y-ptDriverC[1].y)*(float)time/100;

		ptDriver[2].x = ptDriverC[2].x + (ptDriverC[3].x-ptDriverC[2].x)*(float)time/100;
		ptDriver[2].y = ptDriverC[2].y + (ptDriverC[3].y-ptDriverC[2].y)*(float)time/100;
		
		q[0].x = ptDriver[0].x + (ptDriver[1].x - ptDriver[0].x)*(float)time/100;	
		q[0].y = ptDriver[0].y + (ptDriver[1].y - ptDriver[0].y)*(float)time/100;	
		
		q[1].x = ptDriver[1].x + (ptDriver[2].x - ptDriver[1].x)*(float)time/100;	
		q[1].y = ptDriver[1].y + (ptDriver[2].y - ptDriver[1].y)*(float)time/100;	

		pnt[time].x = q[0].x + ( q[1].x - q[0].x )*(float)time/100;
		pnt[time].y = q[0].y + ( q[1].y - q[0].y )*(float)time/100;
		*/
		/// B''(t) = 6(1-t) P0 + 6(3t-2) P1 + 6(1–3t) P2 + 6t P3
		//pnt[time].x = 6*fT_1*ptDriver[0].x + 6*(3*fT-2)*ptDriver[1].x + 6*(1-3*fT)*ptDriver[2].x + 6*fT*ptDriver[3].x;
		//pnt[time].y = 6*fT_1*ptDriver[0].y + 6*(3*fT-2)*ptDriver[1].y + 6*(1-3*fT)*ptDriver[2].y + 6*fT*ptDriver[3].y;


/*void clsInkify::LocalMaxima(int iBrushSize, GlobalHelper &g_GH)
{
	float fMaxGrad,fTemp;
	int xPos,yPos,bFound;

	for ( int x = 0; x < iNC; x += iBrushSize ) 
	{
		for ( int y = 0; y < iNR; y+= iBrushSize ) 
		{
			fMaxGrad = 0.0f;
			bFound = 0;
			xPos = yPos = -1;
			for ( int eY = y - iBrushSize/2; eY < y + iBrushSize/2; eY++ ) 
			{
				for ( int eX = x - iBrushSize/2; eX < x + iBrushSize/2; eX++  ) 
				{
					if ( eX < 0 ) { eX = 0; }
					if ( eX >= iNC ) { eX = iNC - 1; }
						
					if ( eY < 0 ) { eY = 0; }
					if ( eY >= iNR ) { eY = iNR - 1; }

					if( *(UCHAR*)(pImgDest->imageData + eY*pImgGrey->widthStep + eX) ==0)
					{
						
						bFound = 1;
						fTemp = getGradientMagnitude(eY,eX);
						if( fMaxGrad <fTemp || (xPos ==-1 && yPos==-1))
						{
							fMaxGrad = fTemp;
							yPos = eY;
							xPos = eX;
						}
					}
				}
			}
			if(bFound)
			for ( int eY = y - iBrushSize/2; eY < y + iBrushSize/2; eY++ ) 
			{
				for ( int eX = x - iBrushSize/2; eX < x + iBrushSize/2; eX++  ) 
				{
					if ( eX < 0 ) { eX = 0; }
					if ( eX >= iNC ) { eX = iNC - 1; }
						
					if ( eY < 0 ) { eY = 0; }
					if ( eY >= iNR ) { eY = iNR - 1; }
					if(eY !=yPos && eX !=xPos)
						*(UCHAR*)(pImgDest->imageData + eY*pImgGrey->widthStep + eX) = 255;
				}
			}
		}
	}

}*/
