#include "GlobalHelper.h"
extern "C" __declspec(dllexport) void OilPaint(char *szPath, int bPreviewMode, int*iErrCode);

class clsOilPaint
{
private:
	int iNC, iNR;
	IplImage *pImgSource,*pImgDest;
	int **ppIConv;

	void OilPaintEffect(GlobalHelper &g_GH);

public:
	clsOilPaint();
	~clsOilPaint();

	//static const char pcszPath[];
	void OilPaintMain(IplImage **pImgSrc, int bPreviewMode, int*iErrCode,GlobalHelper &g_GH);

};
