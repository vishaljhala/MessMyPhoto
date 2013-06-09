#include "GlobalHelper.h"

class clsOldPhoto
{
private:
	int iNC, iNR;
	IplImage *pImgSource;
	IplImage * pImgSrcHSV;

	void ApplyCanvas(int bUseTexture,GlobalHelper &g_GH);
	void ApplySepia();
	void ApplyBorder(GlobalHelper &g_GH);
	void Contrast(int iContrast);
	void BlackNWhite();
	void ApplyDarkCorners(float a1,float a2,float c1,float c2,float b1,float b2);
	void ApplyStains(int iX,int iY,GlobalHelper &g_GH);

public:
	clsOldPhoto();
	~clsOldPhoto();

	//static const char pcszPath[];
	void OldPhotoMain(IplImage **pImgSrc, int bIsBnW, int iBlur, int iContrast, int bIsDamaged, int*iErrCode,GlobalHelper &g_GH);

};
