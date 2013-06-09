#include "stdafx.h"
#include "oldphoto.h"
#include "GlobalHelper.h"
 
extern char g_pcszPath[];

extern "C" __declspec(dllexport) void OldPhoto(char *szPath, int bIsBnW, int iBlur, int iContrast, int bIsDamaged, int*iErrCode)
{
	DWORD dwStart = GetTickCount();
	SYSTEMTIME stStart;
	GetLocalTime(&stStart);

	GlobalHelper g_GH;
	g_GH.Init(szPath,iErrCode);
	if(*iErrCode >0)
		return;

	//g_GH.DebugOut("OldPhoto: Begin",TRUE);
	
	IplImage *pImgSource;
	int iRet = g_GH.load_RGB(szPath,&pImgSource,false);
	if(!iRet)
	{
		//g_GH.DebugOut("OldPhoto: LoadImage failed",FALSE);
		*iErrCode = 1;
		return;
	}

	clsOldPhoto oldPhoto;
	oldPhoto.OldPhotoMain(&pImgSource, bIsBnW, iBlur, iContrast, bIsDamaged, iErrCode,g_GH);

	iRet = g_GH.save_RGB(szPath,pImgSource);
	if(!iRet)
	{
		//g_GH.DebugOut("OldPhoto: save_RGB failed",FALSE);
		*iErrCode = 1;
		return;
	}

	g_GH.cvReleaseImageProxy(&pImgSource);
	//g_GH.DebugOut("OldPhoto: End",TRUE);

	DWORD dwEnd = GetTickCount();
	char pszTimec[1024];pszTimec[0] = 0;
	sprintf(pszTimec,"OldPhoto,%d-%02d-%02d %02d:%02d:%02d,%ld\0",stStart.wYear,stStart.wMonth,stStart.wDay,stStart.wHour,
		stStart.wMinute, stStart.wSecond, dwEnd - dwStart);
//	sprintf(pszTimec,"OldPhoto,%d-%02d-%02d %02d:%02d:%02d,%ld\0",stStart.wYear,stStart.wMonth,stStart.wDay,dwEnd - dwStart);

	g_GH.DebugOut(pszTimec);

}
clsOldPhoto::clsOldPhoto()
{
	
}
clsOldPhoto::~clsOldPhoto()
{
}
void clsOldPhoto::OldPhotoMain(IplImage **pImgSrc, int bIsBnW, int iBlur, int iContrast, int bIsDamaged, int*iErrCode,GlobalHelper &g_GH)
{
	pImgSource = *pImgSrc;
	if(!pImgSource)
	{
		//g_GH.DebugOut("OldPhotoMain: Source Image NULL",FALSE);
		*iErrCode = 1;
		return;
	}

	iNC = pImgSource->width;
	iNR = pImgSource->height;

	if(bIsBnW)
		BlackNWhite();
	
	if(iContrast>0)
		Contrast(iContrast);

	if(iBlur>0)
	{		
		IplImage *pImgDest = g_GH.cvCreateImageProxy(cvSize(iNC,iNR),IPL_DEPTH_8U,3);
		g_GH.cvSmoothProxy(pImgSource,pImgDest,CV_GAUSSIAN,iBlur,iBlur,0,0);
		g_GH.cvCopyProxy(pImgDest,pImgSource,NULL);
		g_GH.cvReleaseImageProxy(&pImgDest);

	}

	pImgSrcHSV = g_GH.convertImageRGBtoHSV(pImgSource);
	if(!pImgSrcHSV)
	{
		//g_GH.DebugOut("OldPhoto: ConvertImage failed",FALSE);
		*iErrCode = 1;
		return;
	}


	ApplyCanvas(false,g_GH);
	ApplySepia();


	g_GH.cvReleaseImageProxy(&pImgSource);
	pImgSource = g_GH.convertImageHSVtoRGB(pImgSrcHSV);
	if(!pImgSource)
	{
		//g_GH.DebugOut("OldPhoto: convertImage failed",FALSE);
		*iErrCode = 1;
		return;
	}
	g_GH.cvReleaseImageProxy(&pImgSrcHSV);

	if(bIsDamaged)
		ApplyBorder(g_GH);

	*pImgSrc = pImgSource;

}
void clsOldPhoto::BlackNWhite()
{
	int iWStep, iJStep;
	int tmp;

	for (int i=0;i<iNR;i++)
	{
		iWStep = i*pImgSource->widthStep +(int)pImgSource->imageData;
		for(int j=0;j<iNC;j++)
		{
			iJStep = iWStep + j*3;
			*(uchar*)(iJStep + 0) =
			*(uchar*)(iJStep + 1) =
			*(uchar*)(iJStep + 2) = (*(uchar*)(iJStep + 0)+
			*(uchar*)(iJStep + 1) + *(uchar*)(iJStep + 2) )/3.0;
		}
	}

}
void clsOldPhoto::ApplyBorder(GlobalHelper &g_GH)
{
	char buff[1024];
	buff[0] = NULL;
	strcpy(buff,g_pcszPath);

	strcat(buff,"distortedborder");

	IplImage *imgrgb,*imgtmp;
	int iRet = g_GH.load_RGB(buff,&imgrgb,FALSE);
	if(!iRet)
	{
		//g_GH.DebugOut("clsOldPhoto::ApplyBorder - Failed to load canvas",FALSE);
		//g_GH.DebugOut(buff,FALSE);
		return;
	}
	imgtmp = g_GH.cvCreateImageProxy(cvSize(iNC,iNR),pImgSource->depth,3);

	g_GH.cvResizeProxy(imgrgb,imgtmp,1);

	int iWStep, iJStep, iWStepTemp;
	for (int i=0;i<iNR;i++)
	{
		iWStep = i*pImgSource->widthStep + (int)pImgSource->imageData;
		iWStepTemp = i*pImgSource->widthStep + (int)imgtmp->imageData;
		for(int j=0;j<iNC;j++)
		{
			iJStep = j*3;
			if(*(uchar*)(iWStepTemp + iJStep + 0)<128)
				*(uchar*)( iWStep + iJStep + 0) = *(uchar*)(iWStep + iJStep + 1)
				= *(uchar*)( iWStep + iJStep + 2) = 255;
		}
	}

	g_GH.cvReleaseImageProxy(&imgtmp);
	g_GH.cvReleaseImageProxy(&imgrgb);

}
void clsOldPhoto::Contrast(int iContrast)
{
	int iWStep, iJStep;
	int tmp,tmp1;
	float fPerMinus = (100-iContrast)/100.0;
	float fPerPlus = iContrast/100.0;
	for (int i=0;i<iNR;i++)
	{
		iWStep = i*pImgSource->widthStep + (int)pImgSource->imageData;
		for(int j=0;j<iNC;j++)
		{
			iJStep = iWStep+j*3;
			tmp = (*(uchar*)(iJStep + 0) + *(uchar*)(iJStep + 1)+
			*(uchar*)( iJStep + 2))/3.0;
			
			if(tmp<=128)
			{
				*(uchar*)(iJStep + 0) *=fPerMinus;
				*(uchar*)(iJStep + 1) *=fPerMinus;
				*(uchar*)(iJStep + 2) *=fPerMinus;
			}
			else
			{
				*(uchar*)(iJStep + 0) +=(255-*(uchar*)(iJStep + 0))*fPerPlus;
				*(uchar*)(iJStep + 1) +=(255-*(uchar*)(iJStep + 1))*fPerPlus;
				*(uchar*)(iJStep + 2) +=(255-*(uchar*)(iJStep + 2))*fPerPlus;
			}
		}
	}
}
void clsOldPhoto::ApplySepia()
{
	int iWStep, iJStep;
	int tmp;

	for (int i=0;i<iNR;i++)
	{
		iWStep = i*pImgSource->widthStep + (int)pImgSrcHSV->imageData;
		for(int j=0;j<iNC;j++)
		{
			*(uchar*)( iWStep + j*3 + 0) = 25;
		}
	}
}

void clsOldPhoto::ApplyCanvas(int bUseTexture,GlobalHelper &g_GH)
{
	
	char buff[1024];
	buff[0] = NULL;
	strcpy(buff,g_pcszPath);

	if(bUseTexture)
		strcat(buff,"OldPhotoCanvasText");
	else
		strcat(buff,"OldPhotoCanvasPlain");

	IplImage *imgrgb;
	int iRet = g_GH.load_RGB(buff,&imgrgb,FALSE);
	if(!iRet)
	{
		//g_GH.DebugOut("clsOldPhoto::ApplyCanvas - Failed to load canvas",FALSE);
		//g_GH.DebugOut(buff,FALSE);
		return;
	}

	IplImage *imghsv = g_GH.convertImageRGBtoHSV(imgrgb);
	if(!imghsv)
	{
		//g_GH.DebugOut("clsOldPhoto::ApplyCanvas - convertImage failed",FALSE);
		return;
	}

	int iWStep, iJStep;
	int tmp;
	for (int i=0;i<imghsv->height;i++)
	{
		iWStep = i*imghsv->widthStep + (int)imghsv->imageData ;
		for(int j=0;j<imghsv->width;j++)
		{
			iJStep = j*3;
			tmp = *(uchar*)(iWStep + iJStep + 1);
			*(uchar*)(iWStep + iJStep + 1) = ((float)tmp*0.4);

			tmp = *(uchar*)(iWStep + iJStep + 2);
			*(uchar*)(iWStep + iJStep + 2) = ((float)tmp*0.4);
		}
	}
	
	int iCanvasNR=0,iCanvasNC=0;		
	int iWStepPatch, iJStepPatch,tmp1;
	for (int i=0;i<iNR;i++)
	{
		iWStep = i*pImgSrcHSV->widthStep + (int)pImgSrcHSV->imageData;
		iWStepPatch = iCanvasNR*imghsv->widthStep + (int)imghsv->imageData ;

		for(int j=0;j<iNC;j++)
		{
			iJStep = j*3;
			iJStepPatch = iCanvasNC*3;

			tmp = *(uchar*)(iWStep + iJStep + 1);
			*(uchar*)(iWStep + iJStep + 1) = ((float)tmp*0.6) +
				*(uchar*)(iWStepPatch + iJStepPatch + 1);

			tmp = *(uchar*)(iWStep + iJStep + 2);
			*(uchar*)(iWStep + iJStep + 2) = ((float)tmp*0.6) +
				*(uchar*)(iWStepPatch + iJStepPatch + 2);

			iCanvasNC++;
			if(iCanvasNC>=imghsv->width)
				iCanvasNC = 0;
		}
		iCanvasNC=0;
		iCanvasNR++;
		if(iCanvasNR>=imghsv->height)
			iCanvasNR = 0;
	}
	g_GH.cvReleaseImageProxy(&imghsv);
	g_GH.cvReleaseImageProxy(&imgrgb);

}
void clsOldPhoto::ApplyStains(int iX,int iY,GlobalHelper &g_GH)
{
	int iWStep, iJStep;
	int tmp,tmp1;

	char buff[1024];
	buff[0] = NULL;
	strcpy(buff,g_pcszPath);
	strcat(buff,"OldPhotoStain2");

	IplImage *imgrgb;
	int iRet = g_GH.load_RGB(buff,&imgrgb,FALSE);
	if(!iRet)
	{
		//g_GH.DebugOut("clsOldPhoto::ApplyCanvas - Failed to load canvas",FALSE);
		//g_GH.DebugOut(buff,FALSE);
		return;
	}

	IplImage *imghsv = g_GH.convertImageRGBtoHSV(imgrgb);
	if(!imghsv)
	{
		//g_GH.DebugOut("clsOldPhoto::ApplyCanvas - convertImage failed",FALSE);
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

}
/*
Point A - Right Angle
Point C - Along Horizontal Axis
Point B - ALong Vertical Axis
Point D - the value if i,j (some where in triangle)
Point E - Point on Hypotanues
*/
void clsOldPhoto::ApplyDarkCorners(float a1,float a2,float c1,float c2,float b1,float b2)
{
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
			fStep = 255/totlength;
			val = fStep*curlen+64;

			iJStep = j*3;
			tmp = *(uchar*)(pImgSrcHSV->imageData + iWStep + iJStep+2);


			if(tmp>val && val<256)
				*(uchar*)(pImgSrcHSV->imageData + iWStep + iJStep + 2) = val;
		}
	}

}
