#pragma once

extern "C" __declspec(dllexport) void HolgaArt(char *szPath, int bPreviewMode, int*iErrCode);

class clsHolgaArt
{
private:
	IplImage *pImgSource,*pImgCanvas;
	int iNR, iNC;
	void RGB2CMYK(BYTE r, BYTE g, BYTE b, BYTE& c, BYTE& m, BYTE& y, BYTE& k);
	void CMYK2RGB(BYTE c, BYTE m, BYTE y, BYTE k,BYTE &r, BYTE &g, BYTE &b);
	double R, G, B;
	double C, M, Y, K;

public:
	clsHolgaArt();
	~clsHolgaArt();
	void RedChannel(GlobalHelper &g_GH);
	void CyanLayer(GlobalHelper &g_GH);
	void HolgaArtMain(IplImage **pImgSrc,int*iErrCode,GlobalHelper &g_GH);
};

