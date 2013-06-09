#pragma once

extern "C" __declspec(dllexport) void AbstractLineDrawing(char *szPath, int bPreviewMode, int*iErrCode);

class clsAbstractLineDrawing
{
private:
	IplImage *pImgSource, *pImgDest;
	int iNR, iNC;

public:
	clsAbstractLineDrawing();
	~clsAbstractLineDrawing();
	void AbstractLineDrawingMain(IplImage **pImgSrc,int*iErrCode,GlobalHelper &g_GH);
};

