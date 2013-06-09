#pragma once

extern "C" __declspec(dllexport) void GreyScale(char *szPath, int bPreviewMode, int*iErrCode);

class clsGreyScale
{
private:
	IplImage *pImgSource, *pImgDest;
	int iNR, iNC;

public:
	clsGreyScale();
	~clsGreyScale();
	void GreyScale(GlobalHelper &g_GH);
	void GreyScaleMain(IplImage **pImgSrc,int*iErrCode,GlobalHelper &g_GH);
};

