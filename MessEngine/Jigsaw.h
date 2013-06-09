#include "GlobalHelper.h"
extern "C" __declspec(dllexport) void Jigsaw(char *szPath, int bPreviewMode, int*iErrCode);

class clsJigsaw
{
private:
	int iNC, iNR;
	IplImage *pImgSource;
	IplImage *pImgDest;

	void ApplyCanvas(GlobalHelper &g_GH);

public:
	clsJigsaw();
	~clsJigsaw();

	//static const char pcszPath[];
	void JigsawMain(IplImage **pImgSrc, int bPreviewMode, int*iErrCode,GlobalHelper &g_GH);

};
