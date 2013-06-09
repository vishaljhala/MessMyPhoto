#pragma once

extern "C" __declspec(dllexport) void CynoType(char *szPath, int bPreviewMode, int*iErrCode);

class clsCynoType
{
private:
	IplImage *pImgSource, *pImgDest;
	int iNR, iNC;
	void RGB2CMYK(BYTE r, BYTE g, BYTE b, BYTE& c, BYTE& m, BYTE& y, BYTE& k);
	void CMYK2RGB(BYTE c, BYTE m, BYTE y, BYTE k,BYTE &r, BYTE &g, BYTE &b);
	double R, G, B;
	double C, M, Y, K;

public:
	clsCynoType();
	~clsCynoType();
	void CynoType(GlobalHelper &g_GH);
	void CynoTypeMain(IplImage **pImgSrc,int*iErrCode,GlobalHelper &g_GH);
};

