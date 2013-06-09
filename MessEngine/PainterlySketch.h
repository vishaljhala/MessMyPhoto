#include "GlobalHelper.h"
class clsPainterlySketch
{
private:
	IplImage *pImgSource, *pImgDest, *pImgTemp,*pImgGrey;
	int iNR, iNC;
	//int m_iFilterSize;
	//float m_fSigmaDist,  m_fSigmaColor;
	void Diff(GlobalHelper &g_GH);

public:
	clsPainterlySketch();
	~clsPainterlySketch();
	void SketchMain(IplImage **pImgSrc, 
	  int*iErrCode,GlobalHelper &g_GH);
};