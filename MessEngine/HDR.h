#include "GlobalHelper.h"
extern "C" __declspec(dllexport) void HDR(char *szPath, int bPreviewMode, int*iErrCode);

class clsHDR
{
private:
	int iNC, iNR;
	IplImage *pImgSource;
	IplImage *pImgDest;

	void ApplyCanvas(GlobalHelper &g_GH);

public:
	clsHDR();
	~clsHDR();

	//static const char pcszPath[];
	void HDRMain(IplImage **pImgSrc, int bPreviewMode, int*iErrCode,GlobalHelper &g_GH);

};
