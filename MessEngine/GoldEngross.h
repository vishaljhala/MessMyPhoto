#include "GlobalHelper.h"
extern "C" __declspec(dllexport) void GoldEngross(char *szPath, int bIsBorder, int*iErrCode);

class clsGoldEngross
{
private:
	int iNC, iNR;
	IplImage *pImgSource,*pImgCanvas;
	IplImage *pImgDest,*pImgGrey;
	float ** ppfSobelX; //float **ppfSobelY;

	void ApplyCanvas(GlobalHelper &g_GH);

public:
	clsGoldEngross();
	~clsGoldEngross();

	//static const char pcszPath[];
	void GoldEngrossMain(IplImage **pImgSrc, int iDepth, int*iErrCode,GlobalHelper &g_GH);
	int Sobel(float **ppfSobelX,GlobalHelper &g_GH);
	void Engross(int iDetails, GlobalHelper &g_GH);

};
