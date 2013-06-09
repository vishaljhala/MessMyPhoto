#pragma once

extern "C" __declspec(dllexport) void RotateInvert(char *szPath,int iAction, int*iErrCode);

class clsRotateInvert
{
private:
	IplImage *pImgSource, *pImgDest;
	int iNR, iNC;
	int m_iAction;

public:
	clsRotateInvert();
	~clsRotateInvert();
	void Rotate(IplImage **pImgSrc,int iAngle,GlobalHelper &g_GH);
	void Invert(IplImage **pImgSrc,GlobalHelper &g_GH);
};

