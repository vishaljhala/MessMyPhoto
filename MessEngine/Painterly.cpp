#include "stdafx.h"
#include "Painterly.h"
#include "GlobalHelper.h"

#define PI 3.1415926535

extern "C" __declspec(dllexport) void Painterly(char *szPath, float fSigma_blur_factor,int fThreshold, int iMaxStrokeLn, 
	int iMinStrokeLn, int iMaxBrushSz,int bPreviewMode, int*iErrCode)
{
	GlobalHelper g_GH;
	g_GH.Init(szPath,iErrCode);
	if(*iErrCode >0)
		return;

	//g_GH.DebugOut("Painterly: Begin",TRUE);

	PainterlyParam param;
	/*param.fSigma_blur_factor = 0.5;
	param.fThreshold = 100; 			
	param.iMaxStrokeLength = 16;
	param.iMinStrokeLength = 4;
	param.iMaxBrushSize = 8;*/

	param.fSigma_blur_factor = fSigma_blur_factor;
	param.fThreshold = fThreshold; 			
	param.iMaxStrokeLength = iMaxStrokeLn;
	param.iMinStrokeLength = iMinStrokeLn;
	param.iMaxBrushSize = iMaxBrushSz;


	IplImage *pImgSource;
	int iRet = g_GH.load_RGB(szPath,&pImgSource,bPreviewMode);
	if(!iRet)
	{
		//g_GH.DebugOut("Painterly: LoadImage failed",FALSE);
		*iErrCode = 1;
		return;
	}
	clsPainterly paint;
	paint.PainterlyMain(param,&pImgSource,g_GH);

	iRet = g_GH.save_RGB(szPath,pImgSource);

	if(!iRet)
	{
		//g_GH.DebugOut("Painterly: save_RGB failed",FALSE);
		*iErrCode = 1;
		return;
	}

	g_GH.cvReleaseImageProxy(&pImgSource);

	//g_GH.DebugOut("Painterly: End",TRUE);

	*iErrCode = 0;
}
clsPainterly::clsPainterly()
{
	
}
clsPainterly::~clsPainterly()
{
}
int clsPainterly::PainterlyMain(PainterlyParam param,IplImage **pImgSrc,GlobalHelper &g_GH)
{
	pImgSource = *pImgSrc;
	m_param.fSigma_blur_factor = param.fSigma_blur_factor;
	m_param.fThreshold = param.fThreshold; 			
	m_param.iMaxStrokeLength = param.iMaxStrokeLength;
	m_param.iMinStrokeLength = param.iMinStrokeLength;
	m_param.iMaxBrushSize = param.iMaxBrushSize;

	IMAGE iGrey = NULL;
	int ** ppiBlur;
	int ** ppiStrkLst;

	if(!pImgSource)
	{
		//g_GH.DebugOut("PainterlyMain: Source Image NULL",FALSE);
		return 0;
	}

	iNC = pImgSource->width;
	iNR = pImgSource->height;

	pImgTemp = g_GH.cvCreateImageProxy(cvSize(iNC,iNR),IPL_DEPTH_8U,3);
	pImgDest = g_GH.cvCreateImageProxy(cvSize(iNC,iNR),IPL_DEPTH_8U,3);
	iGrey = g_GH.newimage(iNR,iNC);

	g_GH.cvSetProxy(pImgDest,CV_RGB(255,255,255),0);
	g_GH.cvSetProxy(pImgTemp,CV_RGB(0,0,0),0);
	
	float *pfFilter = NULL;
	pfFilter = (float *) calloc ( 19, sizeof (float)  );

	ppiBlur = g_GH.IntHeap2D(iNR,iNC);
	ppiStrkLst = g_GH.IntHeap2D(iNR*iNC,2);
	ppfSobelX = g_GH.FloatHeap2D(iNR,iNC);
	ppfSobelY = g_GH.FloatHeap2D(iNR,iNC);
	if(!ppiBlur || !ppiStrkLst || !ppfSobelX || !ppfSobelY )
	{
		//g_GH.DebugOut("PainterlyMain: IntHeap2D failed",FALSE);
		return 0 ;
	}
	for ( int i = param.iMaxBrushSize; i > 1; i/=2 )
	{
		int iStp,jStp;
		for(int ii=0;ii<iNR; ii++)
		{
			iStp = ii*pImgSource->widthStep;
			for(int j=0;j<iNC;j++)
			{
				jStp = j*3;
				iGrey->data[ii][j] = ( *(uchar*)(pImgSource->imageData + iStp + jStp + 0) 
					+ *(uchar*)(pImgSource->imageData + iStp + jStp + 1) 
					+ *(uchar*)(pImgSource->imageData + iStp + jStp + 2) )/3;
			}
		}
		int intFilter = (2*i*param.fSigma_blur_factor)+1;
		if(intFilter>19) intFilter = 19;
		float fsigma = intFilter/3.0;
		CreateGaussFilter(intFilter,fsigma,pfFilter);
		Convolution(pfFilter,ppiBlur,iGrey,intFilter,fsigma,iNR,iNC);	
		for(int ii=0;ii<iNR; ii++)
			for(int j=0;j<iNC;j++)
			{
				if( ppiBlur[ii][j]<0)
					iGrey->data[ii][j] = 0;
				else if(ppiBlur[ii][j]>255)
					iGrey->data[ii][j] = 255;
				else
					iGrey->data[ii][j] = ppiBlur[ii][j];
			}
				
		
		//sprintf(szn,"c:\\grey%d.jpg",i);
		//g_GH.save_image(iGrey,szn);
		Sobel(iGrey,ppfSobelX,ppfSobelY,g_GH);
		
		int iTotStrokes;		
		IdentifyStrokePoints( (float)i,param,g_GH,ppiStrkLst,&iTotStrokes );
		
		for(int iCtr=0;iCtr<iTotStrokes;iCtr++)
		{
			MakeStroke( i, ppiStrkLst[iCtr][0],ppiStrkLst[iCtr][1],param,g_GH );
		}

	}
	/*RenderEdges(g_GH);
	int iTotStrok;		
	IdentifyStrokePoints( 2,param,g_GH,ppiStrkLst,&iTotStrok );
		
	for(int iCtr=0;iCtr<iTotStrok;iCtr++)
	{
		MakeStroke( 2, ppiStrkLst[iCtr][0],ppiStrkLst[iCtr][1],param,g_GH );
	}
	//sprintf(szn,"c:\\layeredge.jpg");
	//g_GH.cvSaveImageProxy(szn, destImg);*/

	free(pfFilter);
	free(ppiBlur[0]); free(ppiBlur);
	free(ppiStrkLst[0]); free(ppiStrkLst);
	free(ppfSobelX[0]); free(ppfSobelX);
	free(ppfSobelY[0]); free(ppfSobelY);
	delete iGrey;
	
	*pImgSrc = pImgDest;
	g_GH.cvReleaseImageProxy(&pImgSource);
	g_GH.cvReleaseImageProxy(&pImgTemp);

}
int clsPainterly::IdentifyStrokePoints( float iBrushSize,PainterlyParam param,GlobalHelper &g_GH,int **ppiStrkLst,int* iCtr ) 
{
	int iGrid =  iBrushSize;
	int iGridSqr = iGrid * iGrid;
	*iCtr = 0;

	//ZeroMemory(ppiStrkLst, sizeof(int)*iTot*2);

	for ( int x = 0; x < iNC; x += iGrid ) {
		for ( int y = 0; y < iNR; y+= iGrid ) {
			float areaError = 0.0f;
			float worstError = 0;
			int wEX = x, wEY = y;
				
			for ( int eY = y - iGrid/2; eY < y + iGrid/2; eY++ ) {
				for ( int eX = x - iGrid/2; eX < x + iGrid/2; eX++  ) {
					int jitteredX = eX + (int)((0.25) * iGrid );
					int jitteredY = eY + (int)((0.25) * iGrid );
		
					if ( jitteredX < 0 ) { jitteredX = 0; }
					if ( jitteredX >= iNC ) { jitteredX = iNC - 1; }
						
					if ( jitteredY < 0 ) { jitteredY = 0; }
					if ( jitteredY >= iNR ) { jitteredY = iNR - 1; }
						
					float difference = GetDifference( jitteredY, jitteredX );
						
					if ( difference > worstError ) {
						worstError = difference;
						wEX = jitteredX;
						wEY = jitteredY;
					}
						
					areaError += difference;
				}
			}
				
			areaError /= (float)iGridSqr;
				
			if ( areaError > param.fThreshold ) {
				ppiStrkLst[*iCtr][0] = wEY;
				ppiStrkLst[*iCtr][1] = wEX;
				*iCtr = *iCtr+1;
			}
		}
	}
	return 0;	
}

void clsPainterly::MakeStroke(float iBrushSize, int rowInitial, int colInitial,PainterlyParam param,GlobalHelper &g_GH)
{
	//int **ppfPath = g_GH.IntHeap2D(param.iMaxStrokeLength,2);
	int rowCurr = rowInitial;
	int colCurr = colInitial;
	int iCtr = 0;
	float fLastDeltaX = 0,fLastDeltaY = 0;
	float fGradDirX,fGradDirY;
	float fDeltaX,fDeltaY;
	CvPoint pPath[64];

	pPath[iCtr].x = colCurr;
	pPath[iCtr].y = rowCurr;
	float fDiff;	
	for ( iCtr = 1; iCtr < param.iMaxStrokeLength; iCtr++ ) 
	{
		int dR = *(uchar*)(pImgSource->imageData + rowCurr*pImgSource->widthStep + colCurr*3 + 2) -
				 *(uchar*)(pImgSource->imageData + pPath[0].y*pImgSource->widthStep + pPath[0].x*3 + 2);
			
		int dG = *(uchar*)(pImgSource->imageData + rowCurr*pImgSource->widthStep + colCurr*3 + 1) -
				 *(uchar*)(pImgSource->imageData + pPath[0].y*pImgSource->widthStep + pPath[0].x*3 + 1);
			
		int dB = *(uchar*)(pImgSource->imageData + rowCurr*pImgSource->widthStep + colCurr*3 + 0) -
				 *(uchar*)(pImgSource->imageData + pPath[0].y*pImgSource->widthStep + pPath[0].x*3 + 0);
			
		fDiff = sqrt( ( double )( dR * dR + dB * dB + dG * dG ) );
		
		if ( iCtr > param.iMinStrokeLength && 
				GetDifference( rowCurr,colCurr ) < fDiff ) {
			break;
		}
			
		if ( getGradientMagnitude(rowCurr,colCurr) == 0.0f ) 
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
	if ( iCtr > 1 ) 
	{
		for(int i=0;i<iCtr-1;i++)
		{
			int ddR = *(uchar*)(pImgSource->imageData + pPath[0].y*pImgSource->widthStep + pPath[0].x*3 + 2);
			
			int ddG = *(uchar*)(pImgSource->imageData + pPath[0].y*pImgSource->widthStep + pPath[0].x*3 + 1);
			
			int ddB = *(uchar*)(pImgSource->imageData + pPath[0].y*pImgSource->widthStep + pPath[0].x*3 + 0);
			g_GH.cvLineProxy(pImgDest,pPath[i],pPath[i+1],CV_RGB(ddR,
			ddG,ddB),iBrushSize,8,0);
			
			g_GH.cvLineProxy(pImgTemp,pPath[i],pPath[i+1],CV_RGB(255,255,255),iBrushSize,8,0);
		}
	} 
		
	//RenderStroke( iBrushSize, pPath,iCtr);
	//free(ppfPath[0]);free(ppfPath);
}
/*
void clsPainterly::RenderEdges( GlobalHelper &g_GH ) 
{

	for(int ii=0;ii<iNR; ii++)
		for(int j=0;j<iNC;j++)
			srcR->data[ii][j] = srcG->data[ii][j] = srcB->data[ii][j] = 0;

	g_GH.cvSetProxy(tmpImg,CV_RGB(0,0,0),0);

	float fMaxGrad = 0;
	float fGrad;

	for(int ii=0;ii<iNR; ii++)
		for(int j=0;j<iNC;j++)
		{
			fGrad = getGradientMagnitude(ii,j);
			if(fMaxGrad< fGrad)
				fMaxGrad = fGrad;
		}
	fMaxGrad*=2;
	for(int ii=0;ii<iNR; ii++)
		for(int j=0;j<iNC;j++)
		{
			fGrad = getGradientMagnitude(ii,j);
			
			if(fGrad < fMaxGrad )
				ppfSobelX[ii][j] = ppfSobelY[ii][j] = 0;
		}
}
*/
float clsPainterly::GetDifferenceClr( int row, int col ) 
{
	int dR = *(uchar*)(pImgSource->imageData + row*pImgSource->widthStep + col*3 + 2)
			- *(uchar*)(pImgDest->imageData + row*pImgDest->widthStep + col*3 + 2);	
	int dG = *(uchar*)(pImgSource->imageData + row*pImgSource->widthStep + col*3 + 1)
			- *(uchar*)(pImgDest->imageData + row*pImgDest->widthStep + col*3 + 1);	
	int dB = *(uchar*)(pImgSource->imageData + row*pImgSource->widthStep + col*3 + 0)
			- *(uchar*)(pImgDest->imageData + row*pImgDest->widthStep + col*3 + 0);	
	
	return (float)sqrt( ( double )( dR * dR + dB * dB + dG * dG ) );
}
int clsPainterly::Sobel(IMAGE im,float **ppfSobelX,float **ppfSobelY,GlobalHelper &g_GH)
{
	int x,y;

	for(int i=1;i<im->info->nr-1;i++)
	{
		for(int j=1;j<im->info->nc-1;j++)
		{
			x = -1*im->data[i-1][j-1] + -2*im->data[i-1][j] + -1*im->data[i-1][j+1] +
				im->data[i+1][j-1] + 2*im->data[i+1][j] + 1*im->data[i+1][j+1];

			y = -1*im->data[i-1][j-1] + -2*im->data[i][j-1] + -1*im->data[i+1][j-1] +
				im->data[i-1][j+1] + 2*im->data[i][j+1] + 1*im->data[i+1][j+1];

			ppfSobelY[i][j] = y;
			ppfSobelX[i][j] = x;
			//ppfConv[i][j] = sqrt((float)x*x+y*y);
		}
	}
	return 0;
}	
float clsPainterly::GetDifference( int row, int col ) 
{
	float difference;
				
	if ( *(uchar*)(pImgTemp->imageData + row*pImgTemp->widthStep + col*3 + 2)==0 && 
		*(uchar*)(pImgTemp->imageData + row*pImgTemp->widthStep + col*3 + 1)==0 && 
		*(uchar*)(pImgTemp->imageData + row*pImgTemp->widthStep + col*3 + 0)==0 )
	{
		difference = (float)32000;
	} 
	else 
	{
		difference = GetDifferenceClr( row, col );
	}
		
	return difference;
}
float clsPainterly::getGradientMagnitude( int row, int col ) 
{
	return sqrt(ppfSobelX[row][col]*ppfSobelX[row][col] + 
			ppfSobelY[row][col]*ppfSobelY[row][col]);
}
	
void clsPainterly::getGradientDirection(  int row, int col, float *fGradDirX, float *fGradDirY ) 
{
	*fGradDirX = ppfSobelX[row][col] / getGradientMagnitude( row,col );
	*fGradDirY = ppfSobelY[row][col] / getGradientMagnitude( row,col );
}

void clsPainterly::CreateGaussFilter(int iFilterSize,float fSigma, float*pfFilter)
{

	int iCenter = (int)((float)iFilterSize/2);
		

	for(int i=0;i<=iCenter;i++)
	{
			pfFilter[iCenter+i] = Gauss(i, fSigma);
			pfFilter[iCenter-i] = Gauss(i, fSigma);
	}
}
float clsPainterly::Gauss(int x, float fSigma)
{
	
	float fNorm = exp((double) ((-x*x)/(2*fSigma*fSigma)));
	return (1/(sqrt(2*PI)*fSigma)) * fNorm;
}
void clsPainterly::Convolution(float*pfFilter, int **ppfGradXY1,IMAGE channel,int iFilterSize, float fSigma,int iNR,int iNC)
{
	float fTempX,fTempY;
	int iNCFilterMidPnt = iFilterSize/2;

	
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
					fTempY += pfFilter[iNCFilterMidPnt+k]*channel->data[i+k][j];
					//if(k==0) continue;
					fTempX += pfFilter[iNCFilterMidPnt+k]*channel->data[i][j+k];

			}
			ppfGradXY1[i][j] = sqrt(fTempX*fTempX+fTempY*fTempY);

		}
	}


}



