#include "GlobalHelper.h"
extern "C" __declspec(dllexport) void CrossProcess(char *szPath, int bPreviewMode, int*iErrCode);

class clsCrossProcess
{
private:
	int iNC, iNR;
	IplImage *pImgSource;
	IplImage *pImgDest;

	int iHistSat[256][3];
	int iHistVal[256][3];

	void ApplyCanvas(GlobalHelper &g_GH);
	void Histogram(GlobalHelper &g_GH);

public:
	clsCrossProcess();
	~clsCrossProcess();

	//static const char pcszPath[];
	void CrossProcessMain(IplImage **pImgSrc, int bPreviewMode, int*iErrCode,GlobalHelper &g_GH);

};
