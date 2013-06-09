#include "stdafx.h"
#include "CoherentSketch.h"
#include "DoGFilter.h"
#include "GlobalHelper.h"
#define PI 3.1415926535
#define PIby2 1.57079632675
extern GlobalHelper g_GH;

extern "C" __declspec(dllexport) void CoherentSketch(char *szPath,int iFilterSize, int*iErrCode)
{
	//Below code is for actual pencil sketch.
	IMAGE imgGrey = NULL,imgTemp = NULL;
	int iNC,iNR;
	float **ppfGradX,**ppfGradY,**ppfGradNorm,**ppfOrient;
	
	imgGrey = g_GH.load_image(szPath,false);
	
	if (!imgGrey)
	{
		*iErrCode = -1; //Failed ot load image
		return;
	}

	iNC = imgGrey->info->nc;
	iNR = imgGrey->info->nr;
	
	imgTemp = g_GH.newimage(iNR,iNC);

	ppfGradX = g_GH.FloatHeap2D(iNR,iNC);
	ppfGradY = g_GH.FloatHeap2D(iNR,iNC);
	ppfGradNorm = g_GH.FloatHeap2D(iNR,iNC);
	ppfOrient = g_GH.FloatHeap2D(iNR,iNC);

	clsCoherentSketch cs;

	cs.CreateSobelGradient(imgGrey,ppfGradX,ppfGradY);
	char str[20];

	cs.CreateTangentVector(imgGrey,ppfGradX,ppfGradY,ppfGradNorm,ppfOrient,iFilterSize);

	for(int loop=0;loop<6;loop++)
	{
		sprintf(str,"c:\\t1%d.jpg",loop);
		cs.RefineTangentEstimate(imgGrey,ppfGradX,ppfGradY,ppfGradNorm,ppfOrient,iFilterSize);
		cs.PlotImage(imgGrey,ppfGradNorm,str);
		break;
	}

	free(ppfGradY[0]);free(ppfGradY);
	free(ppfGradX[0]);free(ppfGradX);
	free(ppfGradNorm[0]);free(ppfGradNorm);
	free(ppfOrient[0]);free(ppfOrient);

	delete imgGrey;
	delete imgTemp;
	*iErrCode = 0;
}
clsCoherentSketch::clsCoherentSketch()
{
	
}
clsCoherentSketch::~clsCoherentSketch()
{
}
void clsCoherentSketch::CreateSobelGradient(IMAGE channel,float **ppfGradX, float **ppfGradY)
{
	int a,b,c;
	int maxx=-1000,minx=1000, maxy=-1000,miny=1000;
	for(int i=1;i<channel->info->nr-1;i++)
	{
		for(int j=1;j<channel->info->nc-1;j++)
		{
			a = channel->data[i+1][j-1]-channel->data[i-1][j-1];
			b=	2*channel->data[i+1][j]-2*channel->data[i-1][j];
			c= channel->data[i+1][j+1]-channel->data[i-1][j+1];
				 
			ppfGradX[i][j] = a+b+c;//sqrt(float(a*a+b*b+c*c));


			a=channel->data[i-1][j+1]-channel->data[i-1][j-1];
			b=2*channel->data[i][j+1]-2*channel->data[i][j-1]; 
			c=channel->data[i+1][j+1]-channel->data[i+1][j-1];

			ppfGradY[i][j] = a+b+c;//sqrt(float(a*a+b*b+c*c));

			if(ppfGradX[i][j]<minx)
				minx = ppfGradX[i][j];
			if(ppfGradX[i][j]>maxx)
				maxx = ppfGradX[i][j];

			if(ppfGradY[i][j]<miny)
				miny = ppfGradY[i][j];
			if(ppfGradY[i][j]>maxy)
				maxy = ppfGradY[i][j];

		}
	}
}
void clsCoherentSketch::CreateTangentVector(IMAGE channel,float **ppfGradX, float **ppfGradY,float **ppfGradNorm,float **ppfOrient,int iFilterSize)
{

	float temp;
	float minx=1000,maxx=-1000,miny=1000,maxy=-1000;

	for(int i=0;i<channel->info->nr;i++)
	{
		for(int j=0;j<channel->info->nc;j++)
		{
			ppfGradNorm[i][j] = sqrt(ppfGradX[i][j]*ppfGradX[i][j] + ppfGradY[i][j]*ppfGradY[i][j]);
			ppfOrient[i][j] = atan2(ppfGradY[i][j],ppfGradX[i][j]);

			
			//Min-Max Tangent
			if(ppfGradNorm[i][j]<minx)
				minx = ppfGradNorm[i][j];
			if(ppfGradNorm[i][j]>maxx)
				maxx = ppfGradNorm[i][j];
			if(ppfOrient[i][j]<miny)
				miny = ppfOrient[i][j];
			if(ppfOrient[i][j]>maxy)
				maxy = ppfOrient[i][j];
		}
	}
	for(int i=0;i<channel->info->nr;i++)
	{
		for(int j=0;j<channel->info->nc;j++)
		{
			if(ppfGradNorm[i][j]>255)
				ppfGradNorm[i][j] = 255;
		}
	}

}

void clsCoherentSketch::RefineTangentEstimate(IMAGE channel,float **ppfGradX,float **ppfGradY,float **ppfGradNorm,float **ppfOrient, int iFilterSize)
{
	iFilterSize =5;
	int iCenter = iFilterSize/2;
	int m,n;
	float fAngleDiff;
		int fPercent;
		int fDelta;
	float minx = 1000, maxx = -1000,miny = 1000, maxy = -1000;

	for (int i=0;i<channel->info->nr;i++)
	{
		for(int j=0;j<channel->info->nc;j++)
		{
			//Original code
			fDelta = 255;fPercent = 0;
			for(int k=-iCenter;k<=iCenter;k++)
			{
				for(int l=-iCenter;l<=iCenter;l++)
				{
					m = i+k;
					n = j+l;
					if(m<0 || m>=channel->info->nr) continue;
					if(n<0 || n>=channel->info->nc) continue;
					
					fAngleDiff = abs(ppfOrient[m][n] - ppfOrient[i][j]);
					if(fAngleDiff<PIby2)
					{
						//fPercent = 100-(fAngleDiff*100/PIby2);
						fPercent = 1;
						if(ppfGradNorm[i][j]-ppfGradNorm[m][n]<0)
							fDelta = 0;
					}
				}
			}
			if(fPercent ==0)
				ppfGradY[i][j] = ppfGradNorm[i][j];
			else
				ppfGradY[i][j] = fDelta;

			if(ppfGradY[i][j]<minx)
				minx = ppfGradY[i][j];
			if(ppfGradY[i][j]>maxx)
				maxx = ppfGradY[i][j];

		}
	}
	for(int i=0;i<channel->info->nr;i++)
	{
		for(int j=0;j<channel->info->nc;j++)
		{
			ppfGradNorm[i][j] = ppfGradY[i][j];
			if( ppfGradNorm[i][j]>=255)
				ppfGradNorm[i][j] = 255;
			//else
				//ppfGradNorm[i][j] = 0;
			//if(ppfGradNorm[i][j]<0)
				//ppfGradNorm[i][j] = 0;
		}
	}

}
void clsCoherentSketch::PlotImage(IMAGE iR,float **ppfGradNorm,char* szPath)
{
	for(int i=0;i<iR->info->nr;i++)
	{
		for(int j=0;j<iR->info->nc;j++)
		{
			
			//if(ppfGradNorm[i][j]>=255)
				iR->data[i][j] = ppfGradNorm[i][j];
			//else
				//iR->data[i][j] = 0;
		}
	}
	g_GH.save_image(iR,szPath);
}