#include "stdafx.h"
#include "Jigsaw.h"
#include "GlobalHelper.h"
 
extern char g_pcszPath[];

extern "C" __declspec(dllexport) void Jigsaw(char *szPath, int bPreviewMode, int*iErrCode)
{
	GlobalHelper g_GH;
	g_GH.Init(szPath,iErrCode);
	if(*iErrCode >0)
		return;

	//g_GH.DebugOut("Jigsaw: Begin",TRUE);
	
	IplImage *pImgSource;
	int iRet = g_GH.load_RGB(szPath,&pImgSource,bPreviewMode);
	if(!iRet)
	{
		//g_GH.DebugOut("Jigsaw: LoadImage failed",FALSE);
		*iErrCode = 1;
		return;
	}

	clsJigsaw Jigsaw;
	Jigsaw.JigsawMain(&pImgSource,bPreviewMode,iErrCode,g_GH);

	iRet = g_GH.save_RGB(szPath,pImgSource);
	if(!iRet)
	{
		//g_GH.DebugOut("Jigsaw: save_RGB failed",FALSE);
		*iErrCode = 1;
		return;
	}

	g_GH.cvReleaseImageProxy(&pImgSource);
	//g_GH.DebugOut("Jigsaw: End",TRUE);

}
clsJigsaw::clsJigsaw()
{
	
}
clsJigsaw::~clsJigsaw()
{
}
void clsJigsaw::JigsawMain(IplImage **pImgSrc, int bPreviewMode, int*iErrCode,GlobalHelper &g_GH)
{
	pImgSource = *pImgSrc;
	if(!pImgSource)
	{
		//g_GH.DebugOut("JigsawMain: Source Image NULL",FALSE);
		*iErrCode = 1;
		return;
	}

	iNC = pImgSource->width;
	iNR = pImgSource->height;
	
	ApplyCanvas(g_GH);


}

void clsJigsaw::ApplyCanvas(GlobalHelper &g_GH)
{
	
	char buff[1024];
	buff[0] = NULL;
	strcpy(buff,g_pcszPath);

	strcat(buff,"jigsaw");

	IplImage *imgrgb;
	int iRet = g_GH.load_RGB(buff,&imgrgb,FALSE);
	if(!iRet)
	{
		//g_GH.DebugOut("clsJigsaw::ApplyCanvas - Failed to load canvas",FALSE);
		//g_GH.DebugOut(buff,FALSE);
		return;
	}

	pImgDest = g_GH.cvCreateImageProxy(cvSize(iNC,iNR),pImgSource->depth,pImgSource->nChannels);
	g_GH.cvResizeProxy(imgrgb,pImgDest,1);

	int iWStep, iJStep;
	int tmp,tmp1;
	for (int i=0;i<iNR;i++)
	{
		iWStep = i*pImgSource->widthStep;
		for(int j=0;j<iNC;j++)
		{
			iJStep = j*3;
			tmp1=255 - *(uchar*)(pImgDest->imageData + iWStep + iJStep + 2);
			//if(tmp1<224)
			{
				tmp =  *(uchar*)(pImgSource->imageData + iWStep + iJStep + 0);
				tmp = tmp-tmp1;
				if(tmp<0) tmp=0;
				*(uchar*)(pImgSource->imageData + iWStep + iJStep + 0) = tmp;

				tmp =  *(uchar*)(pImgSource->imageData + iWStep + iJStep + 1);
				tmp = tmp-tmp1;
				if(tmp<0) tmp=0;
				*(uchar*)(pImgSource->imageData + iWStep + iJStep + 1) = tmp;

				tmp =  *(uchar*)(pImgSource->imageData + iWStep + iJStep + 2);
				tmp = tmp-tmp1;
				if(tmp<0) tmp=0;
				*(uchar*)(pImgSource->imageData + iWStep + iJStep + 2) = tmp;

			}
		}
	}
	


	g_GH.cvReleaseImageProxy(&imgrgb);
	g_GH.cvReleaseImageProxy(&pImgDest);

}
