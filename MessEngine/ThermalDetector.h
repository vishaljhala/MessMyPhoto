#pragma once

extern "C" __declspec(dllexport) void ThermalDetector(char *szPath, int bPreviewMode, int*iErrCode);

class clsThermalDetector
{
private:
	IplImage *pImgSource, *pImgDest;
	int iNR, iNC;
	int iHistogram[256];
	int iCutOff[4];
public:
	clsThermalDetector();
	~clsThermalDetector();
	void ThermalDetectorMain(IplImage **pImgSrc,int*iErrCode,GlobalHelper &g_GH);
	void HeatMap(GlobalHelper &g_GH);
	void Histogram(GlobalHelper &g_GH);
	void Invert(GlobalHelper &g_GH);

};

