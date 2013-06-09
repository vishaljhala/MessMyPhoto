#include "GlobalHelper.h"
extern "C" __declspec(dllexport) void CoherentSketch(char *szPath,int iFilterSize, int*iErrCode);
class clsCoherentSketch
{

public:
	clsCoherentSketch();
	~clsCoherentSketch();

	void CreateSobelGradient(IMAGE channel,float **ppfGradX, float **ppfGradY);
	void CreateTangentVector(IMAGE channel,float **ppfGradX, float **ppfGradY,float **ppfGradNorm,float **ppfOrient,int iFilterSize);
	void RefineTangentEstimate(IMAGE channel,float **ppfGradX,float **ppfGradY,float **ppfGradNorm,float **ppfOrient, int iFilterSize);
	void PlotImage(IMAGE iR,float **ppfGradNorm,char* szPath);
};
