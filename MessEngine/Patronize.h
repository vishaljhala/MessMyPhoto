#include "GlobalHelper.h"
extern "C" __declspec(dllexport) void Patronize(char *szPath, char *szCountry, int iAlphaBlend,  int iStyle , int*iErrCode);

class clsPatronize
{
private:
	int iNC, iNR;
	IplImage *pImgSource;
	IplImage *pImgDest;

	void ApplyCanvas(char *szCountry, int iAlphaBlend,  int iStyle,GlobalHelper &g_GH);

public:
	clsPatronize();
	~clsPatronize();

	//static const char pcszPath[];
	void PatronizeMain(IplImage **pImgSrc, char *szCountry, int iAlphaBlend,  int iStyle , int*iErrCode,GlobalHelper &g_GH);

};
