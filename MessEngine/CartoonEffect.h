#pragma once

extern "C" __declspec(dllexport) void CartoonEffect(char *szPath,int iFilterSize, int bPreviewMode, int*iErrCode);

class clsCartoonEffect
{
private:
	IplImage *pImgSource, *pImgDest;
	int iNR, iNC;
	int m_iFilterSize;
	float m_fSigmaDist,  m_fSigmaColor;

public:
	clsCartoonEffect();
	~clsCartoonEffect();
	void MeanClusterColor(GlobalHelper &g_GH);
	void CartoonEffectMain(IplImage **pImgSrc,int iFilterSize, int*iErrCode,GlobalHelper &g_GH);
};

