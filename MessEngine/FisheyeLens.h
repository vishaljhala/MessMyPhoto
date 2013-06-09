#include "GlobalHelper.h"
extern "C" __declspec(dllexport) void FisheyeLens(char *szPath, int iStyle, int*iErrCode);

class clsFisheyeLens
{
private:
	int iNC, iNR;
	IplImage *pImgSource;
	IplImage *pImgDest;

	void ApplyCanvas(int istyle,GlobalHelper &g_GH);

public:
	clsFisheyeLens();
	~clsFisheyeLens();

	//static const char pcszPath[];
	void FisheyeLensMain(IplImage **pImgSrc, int iStyle, int*iErrCode,GlobalHelper &g_GH);

};
