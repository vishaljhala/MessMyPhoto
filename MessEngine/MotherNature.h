#include "GlobalHelper.h"

extern "C" __declspec(dllexport) void MotherNature(char *szPath, int iParam0,int iParam1, int iParam2, int iParam3, int*iErrCode);


class clsMotherNature
{
private:
	int iNC, iNR;
	IplImage *pImgSource;
	
	int m_iParam1,m_iParam2, m_iParam3;

	void Droplets(GlobalHelper &g_GH);
	void SunBurst(GlobalHelper &g_GH);
	void Dew(GlobalHelper &g_GH);
	void Rainbow(GlobalHelper &g_GH);
	void DewBlur(GlobalHelper &g_GH);
	void Night(GlobalHelper &g_GH);
	//void ApplyDarkCorners(float a1,float a2,float c1,float c2,float b1,float b2,GlobalHelper &g_GH,IplImage*imgrgb);
	void RainWater(GlobalHelper &g_GH);

public:
	clsMotherNature();
	~clsMotherNature();

	//static const char pcszPath[];
	void MotherNatureMain(IplImage **pImgSrc, int iParam0, int iParam1, int iParam2, int iParam3, int*iErrCode,GlobalHelper &g_GH);

};
