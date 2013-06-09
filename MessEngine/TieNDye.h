#pragma once

extern "C" __declspec(dllexport) void TieNDye(char *szPath, int iSoftColor, int iHardColor, int iDetails, int*iErrCode);

class clsTieNDye
{
private:
	IplImage *pImgSource,*pImgCanvas;
	int iNR, iNC;
	//int iHistGreyLevel[256],iHistGradient[256];
	//int iCutOff,iCutOffSobel,iMaxSobel;
	//int **ppfSobelX;
	int m_iSoftColor, m_iHardColor;
	// Sobel(GlobalHelper &g_GH);
	void Threshold(GlobalHelper &g_GH);
	void Lines(GlobalHelper &g_GH);
	//void Histogram(GlobalHelper &g_GH);


public:
	clsTieNDye();
	~clsTieNDye();
	void TieNDyeMain(IplImage **pImgSrc,int iSoftColor, int iHardColor,int*iErrCode,GlobalHelper &g_GH);

};

