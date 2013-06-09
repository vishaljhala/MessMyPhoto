#include "GlobalHelper.h"
class clsDoGFilter
{
private:
	IplImage *pImgSource, *pImgDest;
	int iNR, iNC;
	int m_iFilterSize;
	float m_fSigmaDist,  m_fSigmaColor;

	void CreateGaussFilter(int iFilterSize,float fSigma, float*pfFilter);
	float Gauss(int x, float fSigma);
	void Convolution(float*pfFilter, float **ppfGradXY1,int iFilterSize, float fSigma,GlobalHelper &g_GH);
	void Diff(float **ppfGradXY1,float **ppfGradXY2 ,GlobalHelper &g_GH);
	void Threshold(float **ppfGradXY1,GlobalHelper &g_GH);

public:
	clsDoGFilter();
	~clsDoGFilter();
	void SketchMain(IplImage **pImgSrc,int iFilterSize, float fSigmaDist, float fSigmaColor, 
	  int*iErrCode,GlobalHelper &g_GH);
};