#include "stdafx.h"
#include "honeymoon.h"
#include "GlobalHelper.h"
 
extern char g_pcszPath[];

extern "C" __declspec(dllexport) void HoneyMoon(char *szPath, int bPreviewMode, int*iErrCode)
{
	GlobalHelper g_GH;
	g_GH.Init(szPath,iErrCode);
	if(*iErrCode >0)
		return;

	//g_GH.DebugOut("HoneyMoon: Begin",TRUE);
	
	IplImage *pImgSource;
	int iRet = g_GH.load_RGB(szPath,&pImgSource,bPreviewMode);
	if(!iRet)
	{
		//g_GH.DebugOut("HoneyMoon: LoadImage failed",FALSE);
		*iErrCode = 1;
		return;
	}

	clsHoneyMoon HoneyMoon;
	HoneyMoon.HoneyMoonMain(&pImgSource,bPreviewMode,iErrCode,g_GH);

	iRet = g_GH.save_RGB(szPath,pImgSource);
	if(!iRet)
	{
		//g_GH.DebugOut("HoneyMoon: save_RGB failed",FALSE);
		*iErrCode = 1;
		return;
	}

	g_GH.cvReleaseImageProxy(&pImgSource);
	//g_GH.DebugOut("HoneyMoon: End",TRUE);

}
clsHoneyMoon::clsHoneyMoon()
{
	
}
clsHoneyMoon::~clsHoneyMoon()
{
}
void clsHoneyMoon::HoneyMoonMain(IplImage **pImgSrc, int bPreviewMode, int*iErrCode,GlobalHelper &g_GH)
{
	pImgSource = *pImgSrc;
	if(!pImgSource)
	{
		//g_GH.DebugOut("HoneyMoonMain: Source Image NULL",FALSE);
		*iErrCode = 1;
		return;
	}

	iNC = pImgSource->width;
	iNR = pImgSource->height;
	
	/*pImgSrcHSV = g_GH.convertImageRGBtoHSV(pImgSource);
	if(!pImgSrcHSV)
	{
		g_GH.DebugOut("HoneyMoon: ConvertImage failed",FALSE);
		*iErrCode = 1;
		return;
	}*/
	//top left	
	ApplyDarkCorners(0,0,304,0,0,304,g_GH);

	ApplyCanvas(g_GH);

	//g_GH.cvReleaseImageProxy(&pImgSource);
	//pImgSource = g_GH.convertImageHSVtoRGB(pImgSrcHSV);
	/*if(!pImgSource)
	{
		g_GH.DebugOut("HoneyMoon: convertImage failed",FALSE);
		*iErrCode = 1;
		return;
	}
	g_GH.cvReleaseImageProxy(&pImgSrcHSV);*/
	//*pImgSrc = pImgSource;

}
/*
void clsHoneyMoon::ApplySepia()
{
	int iWStep, iJStep;
	int tmp;

	for (int i=0;i<iNR;i++)
	{
		iWStep = i*pImgSource->widthStep;
		for(int j=0;j<iNC;j++)
		{
			*(uchar*)(pImgSrcHSV->imageData + iWStep + j*3 + 0) = 25;
		}
	}
}
*/
/*
Point A - Right Angle
Point C - Along Horizontal Axis
Point B - ALong Vertical Axis
Point D - the value if i,j (some where in triangle)
Point E - Point on Hypotanues
*/
void clsHoneyMoon::ApplyDarkCorners(float a1,float a2,float c1,float c2,float b1,float b2,GlobalHelper &g_GH)
{
	char buff[1024];
	buff[0] = NULL;
	strcpy(buff,g_pcszPath);

	strcat(buff,"moon");

	IplImage *imgrgb;
	int iRet = g_GH.load_RGB(buff,&imgrgb,FALSE);
	if(!iRet)
	{
		//g_GH.DebugOut("clsHoneyMoon::ApplyCanvas - Failed to load canvas",FALSE);
		//g_GH.DebugOut(buff,FALSE);
		return;
	}

	//Left Top
	float fStep =  0.6;
	float val;
	int iStep,jStep;
	
	if(a2<b2) iStep=1;
	else iStep = -1;

	if(a1<c1) jStep=1;
	else jStep = -1;
	int iWStep, iJStep;
	int tmp;

	//float a1=0,a2=0; // right angle coordinate
	//float c1=256,c2=0; // along horizontal axiz
	//float b1=0,b2=512; //along verticle axis
	float e1,e2; // point on hypotenuse
	float totlength, curlen;
	for (int i=a2;i!=b2;i+=iStep)
	{
		iWStep = i*pImgSource->widthStep;
		for(int j=a1;j!=c1;j+=jStep)
		{
			e1 = abs(( (a1*i -a2*j)*(b1-c1) - (j-a1)*(c1*b2-c2*b1) )/((i-a2)*(b1-c1)-(j-a1)*(b2-c2) ));
			e2 = abs(((i-a2)*(c1*b2-c2*b1) - (a1*i-a2*j)*(b2-c2))/((i-a2)*(b1-c1)-(j-a1)*(b2-c2)));
				
			curlen = sqrt( (j-a1)*(j-a1) + (i-a2)*(i-a2)  );
			totlength = sqrt( (e1-a1)*(e1-a1) + (e2-a2)*(e2-a2)  );
			int tot=164;
			int divisor=141;
			if(curlen>tot)
			{
				int fuck=curlen-tot;
				if(fuck>141)
					fuck=141;
				*(uchar*)(pImgSource->imageData + i*pImgSource->widthStep + j*3 + 2) = 
					*(uchar*)(pImgSource->imageData + i*pImgSource->widthStep + j*3 + 2)*fuck/divisor +
					*(uchar*)(imgrgb->imageData + i*imgrgb->widthStep + j*3 + 2)*(1-fuck/divisor);
				*(uchar*)(pImgSource->imageData + i*pImgSource->widthStep + j*3 + 1) = 
					*(uchar*)(pImgSource->imageData + i*pImgSource->widthStep + j*3 + 1)*fuck/divisor +
					*(uchar*)(imgrgb->imageData + i*imgrgb->widthStep + j*3 + 1)*(1-fuck/divisor);
				*(uchar*)(pImgSource->imageData + i*pImgSource->widthStep + j*3 + 0) = 
					*(uchar*)(pImgSource->imageData + i*pImgSource->widthStep + j*3 + 0)*fuck/divisor +
					*(uchar*)(imgrgb->imageData + i*imgrgb->widthStep + j*3 + 0)*(1-fuck/divisor);
			}
			else
			{
				*(uchar*)(pImgSource->imageData + i*pImgSource->widthStep + j*3 + 2) = *(uchar*)(imgrgb->imageData + i*imgrgb->widthStep + j*3 + 2);
				*(uchar*)(pImgSource->imageData + i*pImgSource->widthStep + j*3 + 1) = *(uchar*)(imgrgb->imageData + i*imgrgb->widthStep + j*3 + 1);
				*(uchar*)(pImgSource->imageData + i*pImgSource->widthStep + j*3 + 0) = *(uchar*)(imgrgb->imageData + i*imgrgb->widthStep + j*3 + 0);
			}
			
			/*fStep = 255/totlength;
			val = fStep*curlen+64;

			iJStep = j*3;
			tmp = *(uchar*)(pImgSource->imageData + iWStep + iJStep+2);


			if(tmp>val && val<256)
				*(uchar*)(pImgSource->imageData + iWStep + iJStep + 2) = val;*/
		}
	}
	g_GH.cvReleaseImageProxy(&imgrgb);


}

void clsHoneyMoon::ApplyDarkCorners(GlobalHelper &g_GH)
{
	char buff[1024];
	buff[0] = NULL;
	strcpy(buff,g_pcszPath);

	strcat(buff,"moon");

	IplImage *imgrgb;
	int iRet = g_GH.load_RGB(buff,&imgrgb,FALSE);
	if(!iRet)
	{
		//g_GH.DebugOut("clsHoneyMoon::ApplyCanvas - Failed to load canvas",FALSE);
		//g_GH.DebugOut(buff,FALSE);
		return;
	}
	int iWStep,iJStep,tmp1;
	int i2 = imgrgb->height/2.0;
	int j2 = imgrgb->width/2.0;
	int iWStepPatch, iJStepPatch;
	float fFactor = 0;
	for (int i=0;i<imgrgb->height;i++)
	{
		iWStep = i*pImgSource->widthStep;
		iWStepPatch = i*imgrgb->widthStep;

		for(int j=0;j<imgrgb->width;j++)
		{
			if(i>i2)
			{
				fFactor = i-i2;
			}
			//if(j>j2)
				//fFactor=j-j2;
			if(fFactor>100)
				fFactor=100;

			iJStep = j*3;
			iJStepPatch = j*3;
			tmp1 = *(uchar*)(pImgSource->imageData + iWStep + iJStep + 0);
			*(uchar*)(pImgSource->imageData + iWStep + iJStep + 0) = ((float)tmp1*(100-fFactor/100)) +
				*(uchar*)(imgrgb->imageData + iWStepPatch + iJStepPatch + 0)*(fFactor)/100;

			tmp1 = *(uchar*)(pImgSource->imageData + iWStep + iJStep + 1);
			*(uchar*)(pImgSource->imageData + iWStep + iJStep + 1) = ((float)tmp1*(100-fFactor)/100) +
				*(uchar*)(imgrgb->imageData + iWStepPatch + iJStepPatch + 1)*(fFactor)/100;

			tmp1 = *(uchar*)(pImgSource->imageData + iWStep + iJStep + 2);
			*(uchar*)(pImgSource->imageData + iWStep + iJStep + 2) = ((float)tmp1*(100-fFactor)/100) + 
				*(uchar*)(imgrgb->imageData + iWStepPatch + iJStepPatch + 2)*(fFactor)/100;

		}
	}
	g_GH.cvReleaseImageProxy(&imgrgb);

}

void clsHoneyMoon::ApplyCanvas(GlobalHelper &g_GH)
{
	
	char buff[1024];
	buff[0] = NULL;
	strcpy(buff,g_pcszPath);

	strcat(buff,"HoneyCell");

	IplImage *imgrgb;
	int iRet = g_GH.load_RGB(buff,&imgrgb,FALSE);
	if(!iRet)
	{
		//g_GH.DebugOut("clsHoneyMoon::ApplyCanvas - Failed to load canvas",FALSE);
		//g_GH.DebugOut(buff,FALSE);
		return;
	}

	strcat(buff,"1");

	IplImage *imgrgb1;
	iRet = g_GH.load_RGB(buff,&imgrgb1,FALSE);
	if(!iRet)
	{
		//g_GH.DebugOut("clsHoneyMoon::ApplyCanvas - Failed to load canvas",FALSE);
		//g_GH.DebugOut(buff,FALSE);
		return;
	}

	/*IplImage *imghsv = g_GH.convertImageRGBtoHSV(imgrgb);
	if(!imghsv)
	{
		g_GH.DebugOut("clsHoneyMoon::ApplyCanvas - convertImage failed",FALSE);
		return;
	}
	*/
	IplImage *imgTemp;

	/*int iWStep, iJStep;
	int tmp;
	for (int i=0;i<imgrgb->height;i++)
	{
		iWStep = i*imgrgb->widthStep;
		for(int j=0;j<imgrgb->width;j++)
		{
			iJStep = j*3;
			tmp = *(uchar*)(imgrgb->imageData + iWStep + iJStep + 1);
			*(uchar*)(imgrgb->imageData + iWStep + iJStep + 1) = ((float)tmp*0.4);

			tmp = *(uchar*)(imgrgb->imageData + iWStep + iJStep + 2);
			*(uchar*)(imgrgb->imageData + iWStep + iJStep + 2) = ((float)tmp*0.3);
		}
	}
	*/
	int iWStep, iJStep, iItr;
	int iCanvasNR=0,iCanvasNC=0;		
	int iWStepPatch, iJStepPatch,tmp1;
	imgTemp = imgrgb;
	iItr=0;
	for (int i=0;i<iNR;i++)
	{
		iWStep = i*pImgSource->widthStep;
		iWStepPatch = iCanvasNR*imgTemp->widthStep;
		
		for(int j=0;j<iNC;j++)
		{
			iJStep = j*3;
			iJStepPatch = iCanvasNC*3;

			tmp1 = *(uchar*)(pImgSource->imageData + iWStep + iJStep + 0);
			*(uchar*)(pImgSource->imageData + iWStep + iJStep + 0) = ((float)tmp1*0.8) +
				*(uchar*)(imgTemp->imageData + iWStepPatch + iJStepPatch + 0)*0.2;

			tmp1 = *(uchar*)(pImgSource->imageData + iWStep + iJStep + 1);
			*(uchar*)(pImgSource->imageData + iWStep + iJStep + 1) = ((float)tmp1*0.8) +
				*(uchar*)(imgTemp->imageData + iWStepPatch + iJStepPatch + 1)*0.2;

			tmp1 = *(uchar*)(pImgSource->imageData + iWStep + iJStep + 2);
			*(uchar*)(pImgSource->imageData + iWStep + iJStep + 2) = ((float)tmp1*0.8) +
				*(uchar*)(imgTemp->imageData + iWStepPatch + iJStepPatch + 2)*0.2;

			iCanvasNC++;
			if(iCanvasNC>=imgTemp->width)
				iCanvasNC = 0;
		}
		iCanvasNC=0;
		iCanvasNR++;
		if(iCanvasNR>=imgTemp->height)
		{	
			iCanvasNR = 0;
			iItr++;
			if(iItr%2==1)
				imgTemp = imgrgb1;
			else
				imgTemp = imgrgb;
		}
	}
	g_GH.cvReleaseImageProxy(&imgrgb);
	g_GH.cvReleaseImageProxy(&imgrgb1);

}
/*
void clsHoneyMoon::ApplyStains(int iX,int iY,GlobalHelper &g_GH)
{
	int iWStep, iJStep;
	int tmp,tmp1;

	char buff[1024];
	buff[0] = NULL;
	strcpy(buff,pcszPath);
	strcat(buff,"HoneyMoonStain2");

	IplImage *imgrgb;
	int iRet = g_GH.load_RGB(buff,&imgrgb,FALSE);
	if(!iRet)
	{
		g_GH.DebugOut("clsHoneyMoon::ApplyCanvas - Failed to load canvas",FALSE);
		g_GH.DebugOut(buff,FALSE);
		return;
	}

	IplImage *imghsv = g_GH.convertImageRGBtoHSV(imgrgb);
	if(!imghsv)
	{
		g_GH.DebugOut("clsHoneyMoon::ApplyCanvas - convertImage failed",FALSE);
		return;
	}

	int iWStep1, iJStep1;

	for (int i=0;i<imghsv->height;i++)
	{
		iWStep = i*imghsv->widthStep;
		iWStep1 = (i+iY)*pImgSrcHSV->widthStep;

		for(int j=0;j<imghsv->width;j++)
		{
			iJStep = j*3;
			iJStep1 = (j+iX)*3;

			tmp = *(uchar*)(imghsv->imageData + iWStep + iJStep + 1);
			tmp1 = *(uchar*)(imghsv->imageData + iWStep + iJStep + 2);
			
			if(tmp>64)
			{
				*(uchar*)(pImgSrcHSV->imageData + iWStep1 + iJStep1 + 1) = (float)*(uchar*)(pImgSrcHSV->imageData + iWStep1 + iJStep1 + 1)*0.6
					+ tmp*0.4;
				*(uchar*)(pImgSrcHSV->imageData + iWStep1 + iJStep1 + 2) = (float)*(uchar*)(pImgSrcHSV->imageData + iWStep1 + iJStep1 + 2)*0.6
					+ tmp1*0.4;
			}
		}
	}

	g_GH.cvReleaseImageProxy(&imghsv);
	g_GH.cvReleaseImageProxy(&imgrgb);

}*/
/*
Point A - Right Angle
Point C - Along Horizontal Axis
Point B - ALong Vertical Axis
Point D - the value if i,j (some where in triangle)
Point E - Point on Hypotanues
*/
