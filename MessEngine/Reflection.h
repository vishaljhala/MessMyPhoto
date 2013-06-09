#include "GlobalHelper.h"
extern "C" __declspec(dllexport) void Reflection(char *szPath,  int iDistance, int iShadowLength, int iShadowIntensity, int*iErrCode);

class clsReflection
{
private:
	int iNC, iNR;
	IplImage *pImgSource;
	IplImage *pImgDest;
	int m_iDistance, m_iShadowLength, m_iShadowIntensity;
	void ApplyCanvas(GlobalHelper &g_GH);

public:
	clsReflection();
	~clsReflection();

	//static const char pcszPath[];
	void ReflectionMain(IplImage **pImgSrc, int iDistance, int iShadowLength, int iShadowIntensity,GlobalHelper &g_GH);

};
