#include "GlobalHelper.h"
extern "C" __declspec(dllexport) void Barcode(char *szPath, int bPreviewMode, int*iErrCode);

class clsBarcode
{
private:
	int iNC, iNR;
	IplImage *pImgSource;
	IplImage *pImgDest;

	void ApplyCanvas(GlobalHelper &g_GH);

public:
	clsBarcode();
	~clsBarcode();

	//static const char pcszPath[];
	void BarcodeMain(IplImage **pImgSrc, int bPreviewMode, int*iErrCode,GlobalHelper &g_GH);

};
