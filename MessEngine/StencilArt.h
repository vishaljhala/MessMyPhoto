#include "GlobalHelper.h"
extern "C" __declspec(dllexport) void StencilArt(char *szPath, int bPreviewMode, int*iErrCode);

class clsStencilArt
{
private:
	int iNC, iNR;
	IplImage *pImgSource;
	IplImage *pImgDest;
	float **ppfConv;

	void ApplyCanvas(GlobalHelper &g_GH);
	int Sobel(GlobalHelper &g_GH);

public:
	clsStencilArt();
	~clsStencilArt();

	//static const char pcszPath[];
	void StencilArtMain(IplImage **pImgSrc, int bPreviewMode, int*iErrCode,GlobalHelper &g_GH);

};
