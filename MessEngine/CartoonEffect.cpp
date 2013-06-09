#include "StdAfx.h"
#include "CartoonEffect.h"
#include "GlobalHelper.h"


extern "C" __declspec(dllexport) void CartoonEffect(char *szPath,int iFilterSize, int bPreviewMode, int*iErrCode)
{
	GlobalHelper g_GH;
	g_GH.Init(szPath,iErrCode);
	if(*iErrCode >0)
		return;


	IplImage *pImgSource;
	int iRet = g_GH.load_RGB(szPath,&pImgSource,bPreviewMode);
	if(!iRet)
	{
		*iErrCode = 1;
		return;
	}
	clsCartoonEffect Cartoon;
	Cartoon.CartoonEffectMain(&pImgSource,iFilterSize, 
		 iErrCode,g_GH);

	iRet = g_GH.save_RGB(szPath,pImgSource);

	if(!iRet)
	{
		*iErrCode = 1;
		return;
	}

	g_GH.cvReleaseImageProxy(&pImgSource);


	*iErrCode = 0;
}
clsCartoonEffect::clsCartoonEffect()
{
}
clsCartoonEffect::~clsCartoonEffect()
{
}
void clsCartoonEffect::CartoonEffectMain(IplImage **pImgSrc,int iFilterSize,
	  int*iErrCode,GlobalHelper &g_GH)
{
	pImgSource = *pImgSrc;
	m_iFilterSize =iFilterSize;

	if(!pImgSource)
	{
		*iErrCode = 1;
		return;
	}

	iNC = pImgSource->width;
	iNR = pImgSource->height;
	
	int iBig = iNC;
	if(iNR>iNC) iBig = iNR;
	float fPercent = 500.0/iBig;

	int iNewNR = iNR*fPercent;
	int iNewNC = iNC*fPercent;

	pImgDest = g_GH.cvCreateImageProxy(cvSize(iNewNC,iNewNR),pImgSource->depth,3);
	IplImage * pImgTemp = g_GH.cvCreateImageProxy(cvSize(iNewNC,iNewNR),pImgSource->depth,3);
	g_GH.cvResizeProxy(pImgSource,pImgDest,1);

	g_GH.cvSmoothProxy(pImgDest,pImgTemp,CV_BILATERAL,19,0,29.5,300);
	g_GH.cvSmoothProxy(pImgTemp,pImgDest,CV_BILATERAL,19,0,29.5,300);
	g_GH.cvSmoothProxy(pImgDest,pImgTemp,CV_BILATERAL,19,0,29.5,300);

	g_GH.cvResizeProxy(pImgTemp,pImgSource,1);

	g_GH.cvReleaseImageProxy(&pImgDest);
	g_GH.cvReleaseImageProxy(&pImgTemp);

}
