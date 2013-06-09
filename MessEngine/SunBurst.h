#include "GlobalHelper.h"
extern "C" __declspec(dllexport) void SunBurst(char *szPath, int bPreviewMode, int*iErrCode);

class clsSunBurst
{
private:
	int iNC, iNR;
	IplImage *pImgSource;
	IplImage *pImgDest;

	void ApplyCanvas(GlobalHelper &g_GH);

public:
	clsSunBurst();
	~clsSunBurst();

	//static const char pcszPath[];
	void SunBurstMain(IplImage **pImgSrc, int bPreviewMode, int*iErrCode,GlobalHelper &g_GH);

};
