#pragma once

extern "C" __declspec(dllexport) void NightVision(char *szPath, int bIsBorder, int*iErrCode);

class clsNightVision
{
private:
	IplImage *pImgSource, *pImgDest,*pImgDest2;
	int iNR, iNC;

public:
	clsNightVision();
	~clsNightVision();
	void NightVisionMain(IplImage **pImgSrc,int bIsBorder,int*iErrCode,GlobalHelper &g_GH);
	void Greenize(GlobalHelper &g_GH);
	void SniperView(GlobalHelper &g_GH);
	void ApplyBorder(GlobalHelper &g_GH);
};

