#include "stdafx.h"
#include "MotherNature.h"
#include "GlobalHelper.h"
 
extern char g_pcszPath[];

extern "C" __declspec(dllexport) void MotherNature(char *szPath, int iParam0,int iParam1, int iParam2, int iParam3, int*iErrCode)
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

	clsMotherNature oldPhoto;
	oldPhoto.MotherNatureMain(&pImgSource, iParam0, iParam1, iParam2, iParam3, iErrCode,g_GH);

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
	sprintf(pszTimec,"MotherNature,%d-%02d-%02d %02d:%02d:%02d,%ld\0",stStart.wYear,stStart.wMonth,stStart.wDay,stStart.wHour,
		stStart.wMinute, stStart.wSecond, dwEnd - dwStart);
	//	sprintf(pszTimec,"OldPhoto,%d-%02d-%02d %02d:%02d:%02d,%ld\0",stStart.wYear,stStart.wMonth,stStart.wDay,dwEnd - dwStart);

	g_GH.DebugOut(pszTimec);

}
clsMotherNature::clsMotherNature()
{
	
}
clsMotherNature::~clsMotherNature()
{
}
void clsMotherNature::MotherNatureMain(IplImage **pImgSrc, int iParam0, int iParam1, int iParam2, int iParam3, int*iErrCode,GlobalHelper &g_GH)
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
	m_iParam1 = iParam1;
	m_iParam2 = iParam2;
	m_iParam3 = iParam3;

	//0-Droplets
	switch(iParam0)
	{
	case 0://Droplets
		Droplets(g_GH);
		break;
	case 1://Sun burst
		SunBurst(g_GH);
		break;
	case 2://Dew
		Dew(g_GH);
		break;
	case 3://Rainbow
		Rainbow(g_GH);
		break;
	case 4://Night
		Night(g_GH);
		break;
	case 5://Rain
		RainWater(g_GH);
		break;
	}
	*pImgSrc = pImgSource;

}
void clsMotherNature::RainWater(GlobalHelper &g_GH)
{
	char buff[1024];
	buff[0] = NULL;
	strcpy(buff,g_pcszPath);
	strcat(buff,"rain");

	IplImage *imgTemplate;
	int iRet = g_GH.load_RGB(buff,&imgTemplate,FALSE);
	if(!iRet)
		return;

	IplImage *pImgDest = g_GH.cvCreateImageProxy(cvSize(iNC,iNR),pImgSource->depth,pImgSource->nChannels);
	g_GH.cvResizeProxy(imgTemplate,pImgDest,1);

	g_GH.cvReleaseImageProxy(&imgTemplate);	

	g_GH.BlendScreen(pImgSource,pImgDest,0);

	g_GH.cvReleaseImageProxy(&pImgDest);
}
void clsMotherNature::Night(GlobalHelper &g_GH)
{
	char buff[1024];
	buff[0] = NULL;
	strcpy(buff,g_pcszPath);

	strcat(buff,"moon");

	float fBlend = m_iParam1/100.0;
	float fBlendOrig = 1-fBlend;

	IplImage *imgMoon;
	int iRet = g_GH.load_RGB(buff,&imgMoon,FALSE);
	if(!iRet)
		return;

	float iPer = iNC*0.2/imgMoon->width;
	int iMoonW = iNC*0.2;
	int iMoonH = imgMoon->height*iPer;

	IplImage *pImgDest = g_GH.cvCreateImageProxy(cvSize(iMoonW,iMoonH),pImgSource->depth,pImgSource->nChannels);
	g_GH.cvResizeProxy(imgMoon,pImgDest,1);

	g_GH.cvReleaseImageProxy(&imgMoon);	

	int iWStep, iJStep;
	for(int i=0;i<iNR;i++)
	{
		iWStep = i*pImgSource->widthStep + (int)pImgSource->imageData;
		for(int j=0;j<iNC;j++)
		{
			iJStep=j*3;
			*(uchar*)(iWStep +iJStep )   *=fBlendOrig;
			*(uchar*)(iWStep +iJStep +1) *=fBlendOrig;
			*(uchar*)(iWStep +iJStep +2) *=fBlendOrig;
		}
	}
	g_GH.BlendScreen(pImgSource,pImgDest,0);


	g_GH.cvReleaseImageProxy(&pImgDest);
}
void clsMotherNature::Rainbow(GlobalHelper &g_GH)
{
	IplImage *pImgHSV,*pImgRainbow;
	pImgHSV = g_GH.convertImageRGBtoHSV(pImgSource);
	pImgRainbow = g_GH.cvCreateImageProxy(cvSize(pImgSource->width,pImgSource->height),pImgSource->depth,3);
	int iStartAngle, iEndAngle;

	int iWStep, iJStep;
	for(int i=0;i<iNR;i++)
	{
		iWStep = i*pImgRainbow->widthStep + (int)pImgRainbow->imageData;
		for(int j=0;j<iNC;j++)
		{
			iJStep=j*3;
			*(uchar*)(iWStep +iJStep ) = *(uchar*)(iWStep +iJStep +1) = *(uchar*)(iWStep +iJStep +2) = 255;
		}
	}

	int iWStepRain;//, iJStepRain;
	int iCenterX,iCenterY;
	int iArcW, iArcH;


	switch(m_iParam1)
	{
	case 0:
		if(m_iParam2 == 1)
		{ 
			iCenterX=0;iCenterY=iNR/2.0;
			iStartAngle = 270; iEndAngle = 360;
			iArcW = iNC/2.0; iArcH = iNR/2.0;
		}
		if(m_iParam2 == 2)
		{ 
			iCenterX=0;iCenterY=iNR;
			iStartAngle = 270; iEndAngle = 335;
			iArcW = iNC; iArcH = iNR;
		}
		break;
	case 1:
		if(m_iParam2 == 0)
		{ 
			iCenterX=iNC/2.0;iCenterY=iNR/2.0;
			iStartAngle = 270; iEndAngle = 180;
			iArcW = iNC/2.0; iArcH = iNR/2.0;
		}
		if(m_iParam2 == 2)
		{ 
			iCenterX=iNC/2.0;iCenterY=iNR/2.0;
			iStartAngle = 270; iEndAngle = 360;
			iArcW = iNC/2.0; iArcH = iNR/2.0;
		}
		break;
	case 2:
		if(m_iParam2 == 0)
		{ 
			iCenterX=iNC;iCenterY=iNR;
			iStartAngle = 270; iEndAngle = 205;
			iArcW = iNC; iArcH = iNR;
		}
		if(m_iParam2 == 1)
		{ 
			iCenterX=iNC;iCenterY=iNR/2.0;
			iStartAngle = 270; iEndAngle = 180;
			iArcW = iNC/2.0; iArcH = iNR/2.0;
		}
		break;
	default:
		iCenterX=0;iCenterY=iNR/2.0;
		iStartAngle = 270; iEndAngle = 360;
		iArcW = iNC/2.0; iArcH = iNR/2.0;
		break;
	}

	for(int i=163;i>=0;i=i-8)
	{
		g_GH.cvEllipseProxy(pImgRainbow,cvPoint(iCenterX,iCenterY),cvSize(iArcW-(i/8.0),iArcH-(i/8.0)),0,iStartAngle,iEndAngle,CV_RGB(255,255,i),2,8,0);
				
	}
	/*int iHalf=iNR/4.0;
	float blend;
	for(int i=iHalf;i<iNR;i++)
	{
		iWStepRain = i*pImgRainbow->widthStep + (int)pImgRainbow->imageData;
		for(int j=0;j<iNC;j++)
		{
			iJStep=j*3;
			if(*(uchar*)(iWStepRain +iJStep )<255)
			{
				blend = (float)(i-iHalf)/iHalf;
				//*(uchar*)(iWStepRain +iJStep +0 ) = 255*blend+*(uchar*)(iWStepRain +iJStep +0 )*(1-blend);
				//*(uchar*)(iWStepRain +iJStep +1 ) =  0;//*(blend)+ *(uchar*)(iWStepRain +iJStep +1 )*(1-blend);
				*(uchar*)(iWStepRain +iJStep +2 ) = 0;//*(blend) + *(uchar*)(iWStepRain +iJStep +2 )*(1-blend);
			}
		}
	}*/
	for(int i=0;i<iNR;i++)
	{
		iWStep = i*pImgHSV->widthStep + (int)pImgHSV->imageData;
		iWStepRain = i*pImgRainbow->widthStep + (int)pImgRainbow->imageData;
		for(int j=0;j<iNC;j++)
		{
			iJStep=j*3;
			if(*(uchar*)(iWStepRain +iJStep )<255)
			{
				*(uchar*)(iWStep +iJStep +0 ) = *(uchar*)(iWStepRain +iJStep +0 );
				*(uchar*)(iWStep +iJStep +1 ) = *(uchar*)(iWStep +iJStep +1 )*0.8 + *(uchar*)(iWStepRain +iJStep +1 )*0.2;
				*(uchar*)(iWStep +iJStep +2 ) = *(uchar*)(iWStep +iJStep +2 )*0.8 + *(uchar*)(iWStepRain +iJStep +2 )*0.2;
			}
		}
	}

	g_GH.cvReleaseImageProxy(&pImgSource);
	pImgSource = g_GH.convertImageHSVtoRGB(pImgHSV);
	g_GH.cvReleaseImageProxy(&pImgHSV);
	g_GH.cvReleaseImageProxy(&pImgRainbow);
}
void clsMotherNature::Dew(GlobalHelper &g_GH)
{
	char buff[1024];
	buff[0] = NULL;
	strcpy(buff,g_pcszPath);
	strcat(buff,"fog");

	float fBlend = 60/100.0;
	float fBlendOrig = 1-fBlend;

	IplImage *imgrgb;
	int iRet = g_GH.load_RGB(buff,&imgrgb,FALSE);
	if(!iRet)
		return;

	IplImage *pImgDest = g_GH.cvCreateImageProxy(cvSize(iNC,iNR),pImgSource->depth,pImgSource->nChannels);
	g_GH.cvResizeProxy(imgrgb,pImgDest,1);

	g_GH.cvReleaseImageProxy(&imgrgb);	

	DewBlur(g_GH);

	int iWStep, iJStep, iWStepDest;
	int tmp;
	
	for (int i=0;i<iNR;i++)
	{
		iWStep = i*pImgSource->widthStep+(int) pImgSource->imageData;
		iWStepDest = i*pImgDest->widthStep+(int) pImgDest->imageData;
		for(int j=0;j<iNC;j++)
		{
			iJStep = j*3;
			*(uchar*)( iWStep + iJStep + 0) = 
			*(uchar*)(iWStep + iJStep + 0)*fBlendOrig+
			*(uchar*)(iWStepDest + iJStep + 0)*fBlend;

			*(uchar*)(iWStep + iJStep + 1) = 
			*(uchar*)(iWStep + iJStep + 1)*fBlendOrig+
			*(uchar*)(iWStepDest + iJStep + 1)*fBlend;

			*(uchar*)(iWStep + iJStep + 2) = 
			*(uchar*)(iWStep + iJStep + 2)*fBlendOrig+
			*(uchar*)(iWStepDest + iJStep + 2)*fBlend;
		}
	}
	g_GH.cvReleaseImageProxy(&pImgDest);

	strcpy(buff,g_pcszPath);
	strcat(buff,"waterdrop");

	IplImage *imgMoon;
	 iRet = g_GH.load_RGB(buff,&imgMoon,FALSE);
	if(!iRet)
		return;

	pImgDest = g_GH.cvCreateImageProxy(cvSize(iNC,iNR),pImgSource->depth,pImgSource->nChannels);
	g_GH.cvResizeProxy(imgMoon,pImgDest,1);

	g_GH.cvReleaseImageProxy(&imgMoon);	

	g_GH.BlendScreen(pImgSource,pImgDest,0);

	g_GH.cvReleaseImageProxy(&pImgDest);

	
}
void clsMotherNature::DewBlur(GlobalHelper &g_GH)
{
	
	int iItr = iNC*iNR*m_iParam1/100.0;
	int x,y;
	int iAvgR,iAvgG,iAvgB;
	int iStep1, iStep2;
	for (int i=0;i<iItr;i++)
	{
		x = m_iParam3 + (double)(iNC-1 - m_iParam3*2) * rand() / RAND_MAX;
		y = m_iParam3 + (double)(iNR-1 - m_iParam3*2) * rand() / RAND_MAX;
		
		iAvgR = iAvgG = iAvgB = 0;
		for(int j=-m_iParam3;j<=m_iParam3;j++)
		{
			iStep1 = (int)pImgSource->imageData + y*pImgSource->widthStep + (x+j)*3;
			iStep2 = (int)pImgSource->imageData + (y+j)*pImgSource->widthStep + x*3;
			iAvgR+=*(uchar*)(iStep1 +0 );
			iAvgR+=*(uchar*)(iStep2 +0 );
			iAvgG+=*(uchar*)(iStep1 +1 );
			iAvgG+=*(uchar*)(iStep2 +1 );
			iAvgB+=*(uchar*)(iStep1 +2 );
			iAvgB+=*(uchar*)(iStep2 +2 );
		}
		iAvgR /=((m_iParam3*2+1)*2.0);
		iAvgG /=((m_iParam3*2+1)*2.0);
		iAvgB /=((m_iParam3*2+1)*2.0);
		
		for(int j=-m_iParam3;j<=m_iParam3;j++)
		{
			iStep1 = (int)pImgSource->imageData + y*pImgSource->widthStep + (x+j)*3;
			iStep2 = (int)pImgSource->imageData + (y+j)*pImgSource->widthStep + x*3;
			*(uchar*)(iStep1 +0 ) = iAvgR;
			*(uchar*)(iStep2 +0 ) = iAvgR;
			*(uchar*)(iStep1 +1 ) = iAvgG;
			*(uchar*)(iStep2 +1 ) = iAvgG;
			*(uchar*)(iStep1 +2 ) = iAvgB;
			*(uchar*)(iStep2 +2 ) = iAvgB;
		}


			
	}
	

}
void clsMotherNature::SunBurst(GlobalHelper &g_GH)
{
	char buff[1024];
	buff[0] = NULL;
	strcpy(buff,g_pcszPath);

	strcat(buff,"sunburst");


	IplImage *imgMoon;
	int iRet = g_GH.load_RGB(buff,&imgMoon,FALSE);
	if(!iRet)
		return;

	IplImage *pImgDest = g_GH.cvCreateImageProxy(cvSize(iNC,iNR),pImgSource->depth,pImgSource->nChannels);
	g_GH.cvResizeProxy(imgMoon,pImgDest,1);

	g_GH.cvReleaseImageProxy(&imgMoon);	

	g_GH.BlendScreen(pImgSource,pImgDest,m_iParam1);

	g_GH.cvReleaseImageProxy(&pImgDest);
}
void clsMotherNature::Droplets(GlobalHelper &g_GH)
{
	char buff[1024];
	buff[0] = NULL;
	strcpy(buff,g_pcszPath);

	strcat(buff,"waterdrop");


	IplImage *imgMoon;
	int iRet = g_GH.load_RGB(buff,&imgMoon,FALSE);
	if(!iRet)
		return;

	IplImage *pImgDest = g_GH.cvCreateImageProxy(cvSize(iNC,iNR),pImgSource->depth,pImgSource->nChannels);
	g_GH.cvResizeProxy(imgMoon,pImgDest,1);

	g_GH.cvReleaseImageProxy(&imgMoon);	

	g_GH.BlendScreen(pImgSource,pImgDest,0);

	g_GH.cvReleaseImageProxy(&pImgDest);

}

/*
water drops of all sizes..
	char buff[1024];
	buff[0] = NULL;

	IplImage *imgrgb;

	//Copy the Big droplet
	strcpy(buff,g_pcszPath);
	strcat(buff,"waterdrop1");
	int iRet = g_GH.load_RGB(buff,&imgrgb,FALSE);
	if(!iRet)
		return;

	for(int iDrop=0;iDrop<m_iParam1;iDrop++)
	{
		int randNumber;
		//srand(time(NULL));
		int x = (double)rand() / (RAND_MAX + 1) * ((iNC-83));
		int y = (double)rand() / (RAND_MAX + 1) * ((iNR-58));
            
		
		int iWStep, iJStep, iWStepTemp,iJStepTemp;
		for (int i=0;i<imgrgb->height;i++)
		{
			iWStep = (i+y)*pImgSource->widthStep + (int)pImgSource->imageData;
			iWStepTemp = i*imgrgb->widthStep + (int)imgrgb->imageData;
			for(int j=0;j<imgrgb->width;j++)
			{
				iJStep = (j+x)*3;
				iJStepTemp = j*3;
				if(*(uchar*)(iWStepTemp + iJStepTemp + 2)>128 && *(uchar*)(iWStepTemp + iJStepTemp + 1)<128 &&  *(uchar*)(iWStepTemp + iJStepTemp + 1)<128)
				{
				}
				else
				{
					*(uchar*)( iWStep + iJStep + 0) = *(uchar*)( iWStep + iJStep + 0)*0.4 +  *(uchar*)(iWStepTemp + iJStepTemp + 0)*0.6;
					*(uchar*)( iWStep + iJStep + 1) = *(uchar*)( iWStep + iJStep + 1)*0.4 +  *(uchar*)(iWStepTemp + iJStepTemp + 1)*0.6;
					*(uchar*)( iWStep + iJStep + 2) = *(uchar*)( iWStep + iJStep + 2)*0.4 +  *(uchar*)(iWStepTemp + iJStepTemp + 2)*0.6;
				}
			}
		}
	}
	g_GH.cvReleaseImageProxy(&imgrgb);

	//Copy the Medium droplet
	strcpy(buff,g_pcszPath);
	strcat(buff,"waterdrop2");
	iRet = g_GH.load_RGB(buff,&imgrgb,FALSE);
	if(!iRet)
		return;

	for(int iDrop=0;iDrop<m_iParam2;iDrop++)
	{
		int randNumber;
		//srand(time(NULL));
		int x = (double)rand() / (RAND_MAX + 1) * ((iNC-83));
		int y = (double)rand() / (RAND_MAX + 1) * ((iNR-58));
            
		
		int iWStep, iJStep, iWStepTemp,iJStepTemp;
		for (int i=0;i<imgrgb->height;i++)
		{
			iWStep = (i+y)*pImgSource->widthStep + (int)pImgSource->imageData;
			iWStepTemp = i*imgrgb->widthStep + (int)imgrgb->imageData;
			for(int j=0;j<imgrgb->width;j++)
			{
				iJStep = (j+x)*3;
				iJStepTemp = j*3;
				if(*(uchar*)(iWStepTemp + iJStepTemp + 2)>128 && *(uchar*)(iWStepTemp + iJStepTemp + 1)<128 &&  *(uchar*)(iWStepTemp + iJStepTemp + 1)<128)
				{
				}
				else
				{
					*(uchar*)( iWStep + iJStep + 0) = *(uchar*)( iWStep + iJStep + 0)*0.4 +  *(uchar*)(iWStepTemp + iJStepTemp + 0)*0.6;
					*(uchar*)( iWStep + iJStep + 1) = *(uchar*)( iWStep + iJStep + 1)*0.4 +  *(uchar*)(iWStepTemp + iJStepTemp + 1)*0.6;
					*(uchar*)( iWStep + iJStep + 2) = *(uchar*)( iWStep + iJStep + 2)*0.4 +  *(uchar*)(iWStepTemp + iJStepTemp + 2)*0.6;
				}
			}
		}
	}
	g_GH.cvReleaseImageProxy(&imgrgb);
	
	//Copy the small droplet
	strcpy(buff,g_pcszPath);
	strcat(buff,"waterdrop3");
	iRet = g_GH.load_RGB(buff,&imgrgb,FALSE);
	if(!iRet)
		return;

	for(int iDrop=0;iDrop<m_iParam3;iDrop++)
	{
		int randNumber;
		//srand(time(NULL));
		int x = (double)rand() / (RAND_MAX + 1) * ((iNC-83));
		int y = (double)rand() / (RAND_MAX + 1) * ((iNR-58));
            
		
		int iWStep, iJStep, iWStepTemp,iJStepTemp;
		for (int i=0;i<imgrgb->height;i++)
		{
			iWStep = (i+y)*pImgSource->widthStep + (int)pImgSource->imageData;
			iWStepTemp = i*imgrgb->widthStep + (int)imgrgb->imageData;
			for(int j=0;j<imgrgb->width;j++)
			{
				iJStep = (j+x)*3;
				iJStepTemp = j*3;
				if(*(uchar*)(iWStepTemp + iJStepTemp + 2)>128 && *(uchar*)(iWStepTemp + iJStepTemp + 1)<128 &&  *(uchar*)(iWStepTemp + iJStepTemp + 1)<128)
				{
				}
				else
				{
					*(uchar*)( iWStep + iJStep + 0) = *(uchar*)( iWStep + iJStep + 0)*0.4 +  *(uchar*)(iWStepTemp + iJStepTemp + 0)*0.6;
					*(uchar*)( iWStep + iJStep + 1) = *(uchar*)( iWStep + iJStep + 1)*0.4 +  *(uchar*)(iWStepTemp + iJStepTemp + 1)*0.6;
					*(uchar*)( iWStep + iJStep + 2) = *(uchar*)( iWStep + iJStep + 2)*0.4 +  *(uchar*)(iWStepTemp + iJStepTemp + 2)*0.6;
				}
			}
		}
	}
	g_GH.cvReleaseImageProxy(&imgrgb);

*/

/*
	char buff[1024];
	buff[0] = NULL;
	strcpy(buff,g_pcszPath);

	strcat(buff,"sunburst");

	IplImage *imgrgb,*imgtmp;
	int iRet = g_GH.load_RGB(buff,&imgrgb,FALSE);
	if(!iRet)
	{
		//g_GH.DebugOut("clsOldPhoto::ApplyBorder - Failed to load canvas",FALSE);
		//g_GH.DebugOut(buff,FALSE);
		return;
	}
	imgtmp = g_GH.cvCreateImageProxy(cvSize(iNC/2.0,iNR/2.0),pImgSource->depth,3);

	g_GH.cvResizeProxy(imgrgb,imgtmp,1);
	g_GH.cvReleaseImageProxy(&imgrgb);
	//imgrgb = g_GH.cvCreateImageProxy(cvSize(imgtmp->width,imgtmp->height),pImgSource->depth,3);
	imgrgb = g_GH.convertImageRGBtoHSV(imgtmp);
	g_GH.cvReleaseImageProxy(&imgtmp);

	IplImage *pImgHSV;
	pImgHSV = g_GH.convertImageRGBtoHSV(pImgSource);
	
	int iStart = iNC/4.0;
	int iWStep, iJStep, iWStepTemp,iJStepTemp;
	for (int i=0;i<imgrgb->height;i++)
	{
		iWStep = i*pImgHSV->widthStep + (int)pImgHSV->imageData;
		iWStepTemp = i*imgrgb->widthStep + (int)imgrgb->imageData;
		for(int j=0;j<imgrgb->width;j++)
		{
			iJStep = (j+iStart)*3;
			iJStepTemp = j*3;
			if(*(uchar*)(iWStep + iJStep + 2)<*(uchar*)(iWStepTemp + iJStepTemp + 2))
			{
				*(uchar*)(iWStep + iJStep + 1)=*(uchar*)(iWStepTemp + iJStepTemp + 1);
				*(uchar*)(iWStep + iJStep + 2)=*(uchar*)(iWStepTemp + iJStepTemp + 2);
			}
		}
	}

	g_GH.cvReleaseImageProxy(&pImgSource);
	g_GH.cvReleaseImageProxy(&imgrgb);
	pImgSource = g_GH.convertImageHSVtoRGB(pImgHSV);
	g_GH.cvReleaseImageProxy(&pImgHSV);

}

*/