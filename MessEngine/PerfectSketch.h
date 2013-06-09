#include "GlobalHelper.h"

class clsPerfectSketch
{
private:
	int Sobel(GlobalHelper &g_GH);
	void DrawSketch(GlobalHelper &g_GH);
	//void Histogram(GlobalHelper &g_GH);
	//void Normalize(GlobalHelper &g_GH);

	int iNC, iNR;
	IplImage *pImgSource;
	IplImage * pImgSrcHSV;
	int **ppfConv,**ppfConvGrad;
	float m_fDetails;
	int iMinGrad, iMaxGrad;
	int iHistPix[256][3],iHistGrad[256];
	int iCutOffPix, iCutOffGrad;
public:
	clsPerfectSketch();
	~clsPerfectSketch();

	int SketchFilter(IplImage **pImgSrc, int iDetails, int*iErrCode,GlobalHelper &g_GH);

};
