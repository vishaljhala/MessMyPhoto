#include "GlobalHelper.h"
class clsMotionBlur
{
private:
	IplImage *pImgSource, *pImgDest;
	int iNR, iNC;
	int m_iFilterSize, m_iAngle;
	float m_fSigmaDist,  m_fSigmaColor;

	void CreateGaussFilter(int iFilterSize,float fSigma, float*pfFilter);
	float Gauss(int x, float fSigma);
	void Convolution(float *pfFilter, int **ppfGradXY1,GlobalHelper &g_GH);

public:
	clsMotionBlur();
	~clsMotionBlur();
	void MotionBlurMain(IplImage **pImgSrc,int iFilterSize, int iAngle, 
	  int*iErrCode,GlobalHelper &g_GH);
};