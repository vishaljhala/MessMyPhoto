#include "GlobalHelper.h"

class clsCyborgSketch
{
private:
	void ApplyCanvas(GlobalHelper &g_GH, int iGridWidth, int iOrientation);
	int Sobel(GlobalHelper &g_GH);
	void ApplyCyborg(GlobalHelper &g_GH);

	int iNC, iNR;
	IplImage *pImgSource;
	IplImage * pImgSrcHSV;
	int **ppfConv;
	int m_iScanType;

public:
	clsCyborgSketch();
	~clsCyborgSketch();

	int CyborgFilter(IplImage **pImgSrc, int iScanType, int iGridWidth, int iOrientation,int*iErrCode,GlobalHelper &g_GH);

};
