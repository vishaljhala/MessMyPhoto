#include "GlobalHelper.h"
extern "C" __declspec(dllexport) void HoneyMoon(char *szPath, int bPreviewMode, int*iErrCode);

class clsHoneyMoon
{
private:
	int iNC, iNR;
	IplImage *pImgSource;
	//IplImage * pImgSrcHSV;

	void ApplyCanvas(GlobalHelper &g_GH);
	//void ApplySepia();
	void ApplyDarkCorners(GlobalHelper &g_GH);
	void ApplyDarkCorners(float a1,float a2,float c1,float c2,float b1,float b2,GlobalHelper &g_GH);

	//void ApplyStains(int iX,int iY,GlobalHelper &g_GH);

public:
	clsHoneyMoon();
	~clsHoneyMoon();

	//static const char pcszPath[];
	void HoneyMoonMain(IplImage **pImgSrc, int bPreviewMode, int*iErrCode,GlobalHelper &g_GH);

};
