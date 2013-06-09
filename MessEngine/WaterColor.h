#include "GlobalHelper.h"
extern "C" __declspec(dllexport) void WaterColor(char *szPath, int bPreviewMode, int*iErrCode);

class clsWaterColor
{
private:
	int iNC, iNR;
	IplImage *pImgSource,*pImgHSV,*pImgDest,*pImgGrey;
	float **ppfConv;

	void ApplyCanvas(GlobalHelper &g_GH);
	int Sobel(GlobalHelper &g_GH);
	void WaterStrokes(GlobalHelper &g_GH);

public:
	clsWaterColor();
	~clsWaterColor();

	//static const char pcszPath[];
	void WaterColorMain(IplImage **pImgSrc, int bPreviewMode, int*iErrCode,GlobalHelper &g_GH);

};
